.486p
model flat
ideal

vga=0
vga=1

if vga
; 640*480
  Ix=640/1
  Iy=480/1
else
; 320*240
  Ix=640/2
  Iy=480/2
endif


extrn DosClose:near
extrn DosCloseEventSem:near
extrn DosCreateEventSem:near
extrn DosCreateThread:near
extrn DosExit:near
extrn DosExitList:near
extrn DosOpen:near
extrn DosPostEventSem:near
extrn DosResetEventSem:near
extrn DosSetPriority:near
extrn DosSleep:near
extrn DosSuspendThread:near
extrn DosWaitEventSem:near
extrn DosWrite:near

extrn UsbCancelTransfer:near
extrn UsbClose:near
extrn UsbCtrlMessage:near
extrn UsbIsoClose:near
extrn UsbIsoOpen:near
extrn UsbOpen:near
extrn UsbQueryDeviceReport:near
extrn UsbQueryNumberDevices:near
extrn UsbStartIsoTransfer:near

extrn accept:near
extrn bind:near
extrn listen:near
extrn psock_errno:near
extrn recv:near
extrn send:near
extrn setsockopt:near
extrn socket:near
extrn sock_errno:near
;extrn sock_init:near
extrn soclose:near

stack 8192

dataseg ; must be first
dSize=61440 ; max data size
iSize=65536 ; data+parm size
iUsed=4 ; number of buffers
tSize=iSize*iUsed ; total
IsoData db tSize dup(0)

dataseg
IsoEvent dd 0 ; semphore handle

udataseg
ObtainImage dd ?
ObtainPosts dd ?
SupplyImage dd ?
SupplyPosts dd ?

dataseg
szFail0 db 'device driver open error.',13,10
szFail1 db 'set configuration failed.',13,10
szFail2 db 'obtain thread error.',13,10
szFail3 db 'frame size error.',13,10

dataseg
szGood0 db 'started.',13,10
szGood1 db 'stopped.',13,10
szGood2 db 'waiting.',13,10

dataseg
szInfo1 db '[open isochronous]',13,10
szInfo2 db '[queue isochronous]',13,10
szInfo3 db '[close isochronous]',13,10
szInfo4 db '[????????]',13,10
szInfo5 db '????',13,10

udataseg
fhDevice dd ?

udataseg
tidObtain dd ?
tidSupply dd ?

udataseg
BytesDone dd ?

codeseg
proc MainRoutine c near
arg @@Mod,@@Nul,@@Env,@@Arg
; determine begin of arguments
  cld ; operate foreward scan
  mov ecx,512 ; max scan length
  mov edi,[@@Arg] ; start address
  repne scasb ; find terminator
; process passed arguments
  call ProcessArguments
; show appropriate started message
  call DosWrite c,1,offset(szGood0),10,offset(BytesDone)
; access attached webcam
  call ObtainUvcDevice
  jnz ShowStoppedMessage
; process compound descriptor
  call ProcessDescriptors
  jnz ShowStoppedMessage
; open usb camera device driver
  call UsbOpen c,offset(fhDevice),[idVendor],[idProduct],[bcdDevice],0
  test eax,eax ; check for errors
  jnz DosOpenFailure ; bailout
; register termination processing
  call DosExitList c,1,offset(ProcessAborted)
; create isochronous event semaphore
  call DosCreateEventSem c,0,offset(IsoEvent),1,0
; create obtain image event semaphore
  call DosCreateEventSem c,0,offset(ObtainImage),0,0
; create supply image event semaphore
  call DosCreateEventSem c,0,offset(SupplyImage),0,0
; start obtain control thread
  call DosCreateThread c,offset(tidObtain),offset(ObtainThread),0,2,8192
; start supply control thread
  call DosCreateThread c,offset(tidSupply),offset(SupplyThread),0,2,8192
; hang in here forever
  call DosSleep c,-1
;------------------------------------------------------------------------------
  jmp ProcessAborted ; cannot happen!
;------------------------------------------------------------------------------
label DosOpenFailure near
; report dos open failure
  call DosWrite c,1,offset(szFail0),27,offset(BytesDone)
label ShowStoppedMessage near
; show application stopped message
  call DosWrite c,1,offset(szGood1),10,offset(BytesDone)
; exit the process
  call DosExit c,1,0
endp MainRoutine

codeseg
proc ProcessAborted c near
; suspend supply control thread
  call DosSuspendThread c,[tidSupply]
; suspend obtain control thread
  call DosSuspendThread c,[tidObtain]
; close supply image event semaphore
  call DosCloseEventSem c,[SupplyImage]
; close obtain image event semaphore
  call DosCloseEventSem c,[ObtainImage]
; close isochronous event semaphore
  call DosCloseEventSem c,[IsoEvent]
; cancel isochronous transfers
  call UsbCancelTransfer c,[fhDevice],[AddrEndpoint],[AltInterface],[IsoEvent]
; close isochronous transfer
  call DosWrite c,1,offset(szInfo3),szInfo4-szInfo3,offset(BytesDone)
  call UsbIsoClose c,[fhDevice],[AddrEndpoint],[AltInterface]
; reset alternative interface request
  call UsbCtrlMessage c,[fhDevice],001h,0Bh,0,[NumInterface],0,0,0
; close usb camera device driver
  call UsbClose c,[fhDevice]
; close server connection
  call soclose c,[ServerSocket]
; show application stopped message
  call DosWrite c,1,offset(szGood1),10,offset(BytesDone)
; exit termination process
  call DosExitList c,3,0)
endp ProcessAborted

dataseg
ObtainCount dd 3
SupplyCount dd 1

udataseg
; image buffers
InBuffer0 db Ix*Iy*2 dup(?)
InBuffer1 db Ix*Iy*2 dup(?)
InBuffer2 db Ix*Iy*2 dup(?)

dataseg
; ring of image buffer pointers
InBufPtr0 dd offset(InBufPtr1),offset(InBuffer0),0
InBufPtr1 dd offset(InBufPtr2),offset(InBuffer1),0
InBufPtr2 dd offset(InBufPtr0),offset(InBuffer2),0

dataseg
; current image buffer pointers
ObtainPtr dd offset(InBufPtr0)
SupplyPtr dd offset(InBufPtr2)

dataseg
Negotiate db 26 dup(0)
Suggested db 26 dup(0)

dataseg
IsoFree dd iUsed
IsoThis dd offset(IsoData)

udataseg
IsoPost dd ?

dataseg
pTimeStamp dd 0

codeseg
proc ObtainThread c near
arg @@parameter:dword
; set configuration request
  call UsbCtrlMessage c,[fhDevice],000h,09h,0001h,0000h,0,0,0
  test eax,eax ; check for errors
  jnz BadObtainThread ; failure
; supply negotiate setting
  call UsbCtrlMessage c,[fhDevice],021h,01h,0100h,0001h,26,offset(Negotiate),0
  test eax,eax ; check for errors
  jnz BadObtainThread ; failure
; obtain suggested setting
  call UsbCtrlMessage c,[fhDevice],0A1h,81h,0100h,0001h,26,offset(Suggested),0
  test eax,eax ; check for errors
  jnz BadObtainThread ; failure
; commit suggested setting
  call UsbCtrlMessage c,[fhDevice],021h,01h,0200h,0001h,26,offset(Suggested),0
  test eax,eax ; check for errors
  jnz BadObtainThread ; failure
; issue set alternative interface
  call UsbCtrlMessage c,[fhDevice],001h,0Bh,[AltInterface],[NumInterface],0,0,0
  test eax,eax ; check for errors
  jnz BadObtainThread ; failure
; open isochronous transfer
  call DosWrite c,1,offset(szInfo1),szInfo2-szInfo1,offset(BytesDone)
  call UsbIsoOpen c,[fhDevice],[AddrEndpoint],[AltInterface],iUsed,[IsoFrameSize]
  test eax,eax ; check for errors
  jnz BadObtainThread ; failure
; obtain first buffer pointer
  push ebp ; save register
  mov eax,[ObtainPtr] ; current
  mov edi,[eax+4] ; this buffer
  lea ebp,[edi+Ix*Iy*2] ; end
; set time critical priority
  call DosSetPriority c,2,3,31,0
label SupplyIsochronousBuffer near
; address current isochronous buffer
  mov esi,[IsoThis] ; this buffer
; check buffer completion status
  movzx eax,[word(esi+dSize)]
  test eax,eax ; check for errors
  jnz NotIsoTransfer ; failure
; use actual completion sizes
  sub ebx,ebx ; frame index
label NextIsochronousFrame near
  mov eax,[IsoThis] ; this buffer
  movzx edx,[word(eax+ebx*2+dSize+4)]
; verify header data available
  cmp edx,12 ; header length
  jb BumpToNextFrame ; skip
; validate payload header
  mov eax,[esi] ; HLE/BFH[0]
  cmp al,12 ; header length
  jne BumpToNextFrame ; skip
  and ah,7Ch ; fixed flags
  cmp ah,0Ch ; check flags
  jne BumpToNextFrame ; skip
; synchronize video frame
  mov eax,[esi+2] ; pts
  cmp eax,[pTimeStamp]
  je EndRestartFrame
  mov [pTimeStamp],eax
; reset this buffer pointer
  lea edi,[ebp-Ix*Iy*2]
label EndRestartFrame near
; check for end of write buffer
  lea eax,[edi+edx-12] ; predict
  cmp eax,ebp ; buffer boundary
  jna MoveNewImageData ; fits
; reset this buffer pointer
  lea edi,[ebp-Ix*Iy*2]
label MoveNewImageData near
; move isochronous frame
  mov ecx,edx ; frame length
  lea ecx,[ecx-12] ; header
  lea esi,[esi+12] ; header
  rep movsb ; frame data
  sub esi,edx ; start
; check for end of frame
  cmp [StreamFormat],'MJPG'
  jne EndObtainMJPG ; no
; check for end of image
  cmp [word(edi-2)],0D9FFh
  je ImageComplete ; mjpg
label EndObtainMJPG near
; check for end of frame
  cmp [StreamFormat],'YUYV'
  jne EndObtainYUYV ; no
; check for end of write buffer
  cmp edi,ebp ; buffer boundary
  je ImageComplete ; yuyv
label EndObtainYUYV near
label BumpToNextFrame near
  inc ebx ; current frame index
  cmp ebx,[IsoFrameUsed] ; last
  jnb EndIsochronousMove ; queue
  add esi,[IsoFrameSize] ; next
  jmp NextIsochronousFrame
label ImageComplete near
; update buffer last address used
  mov eax,[ObtainPtr] ; current
  mov [eax+8],edi ; last address
; update currently available image buffers
  call DosResetEventSem c,[ObtainImage],offset(ObtainPosts)
  cmp eax,012Ch ; already reset
  je InspectMinimumCount
  test eax,eax ; check for errors
  jnz BadIsoTransfer ; failure
  mov eax,[ObtainPosts] ; additional
  add [ObtainCount],eax ; current
label InspectMinimumCount near
  cmp [ObtainCount],1 ; minimum
  ja DeliverThisBuffer ; available
; overrun so reuse this buffer
; reset this buffer pointer
  lea edi,[ebp-Ix*Iy*2]
  jmp BumpToNextFrame
label DeliverThisBuffer near
; deliver image buffer to supply thread
  call DosPostEventSem c,[SupplyImage]
  cmp eax,012Bh ; already posted
  je InspectNextObtain ; continue
  test eax,eax ; check for errors
  jnz BadIsoTransfer ; failure
label InspectNextObtain near
  dec [ObtainCount] ; current
; obtain next buffer pointer
  mov eax,[ObtainPtr] ; current
  mov eax,[eax] ; chain pointer
  mov [ObtainPtr],eax ; current
  mov edi,[eax+4] ; this buffer
  lea ebp,[edi+Ix*Iy*2] ; end
  jmp BumpToNextFrame
label NotIsoTransfer near
; reset this buffer pointer
  lea edi,[ebp-Ix*Iy*2]
label EndIsochronousMove near
; reset iso frame length array
  mov eax,[IsoFrameSize] ; length
  mov ecx,[IsoFrameUsed] ; count
  mov esi,[IsoThis] ; this buffer
label ResetIsoFrameLengthArray near
; reset individual iso frame lengths
  mov [word(esi+ecx*2+dSize+4-2)],ax
  loop ResetIsoFrameLengthArray
; queue next isochronous transfer
; call DosWrite c,1,offset(szInfo2),szInfo3-szInfo2,offset(BytesDone)
  lea eax,[esi+dSize] ; parm buffer pointer
  mov [word(eax+2)],dSize ; data buffer size
  call UsbStartIsoTransfer c,[fhDevice],[AddrEndpoint],[AltInterface],[IsoEvent],eax,esi,[IsoFrameSize],[IsoFrameUsed]
  test eax,eax ; check for errors
  jnz BadIsoTransfer ; failure
; update next isochronous buffer pointer
  add [IsoThis],iSize ; address next buffer
  cmp [IsoThis],offset(IsoData+tSize)
  jne DecrementFreeBuffers ; proper next
  mov [IsoThis],offset(IsoData) ; first
label DecrementFreeBuffers near
  dec [IsoFree] ; buffers available
  jnz SupplyIsochronousBuffer
; await isochronous buffers filled
  call DosWaitEventSem c,[IsoEvent],3000
  test eax,eax ; check for errors
  jnz BadIsoTransfer ; failure
; update currently filled isochronous buffers
  call DosResetEventSem c,[IsoEvent],offset(IsoPost)
  test eax,eax ; check for errors
  jnz BadIsoTransfer ; failure
  mov eax,[IsoPost] ; additional buffers filled
  add [IsoFree],eax ; current buffers filled
  jmp SupplyIsochronousBuffer
label BadIsoTransfer near
  pop ebp ; restore register
label BadObtainThread near
  call ShowReturnCode ; info
; show appropriate error message
  call DosWrite c,1,offset(szFail2),22,offset(BytesDone)
; exit the obtain thread
  call DosExit c,0,0
endp ObtainThread

dataseg
hex2ascii db '0123456789ABCDEF'

codeseg
proc ShowReturnCode near
  pusha ; save registers
; convert return code
  mov ecx,8 ; code length
label ConvertDigit near
  mov edx,eax ; error code
  and edx,0000000Fh ; digit
  mov dl,[hex2ascii+edx]
  mov [szInfo4+ecx],dl
  shr eax,4 ; next one
  loop ConvertDigit
; show appropriate info message
  call DosWrite c,1,offset(szInfo4),12,offset(BytesDone)
  popa ; restore registers
  ret ; return
endp ShowReturnCode

dataseg
backlog=5
port=14225
SocketAddress db 2,0,high(port),low(port),0,0,0,0,8 dup(0)
SocketOption dd 4 ; SO_REUSEADDR

udataseg
ClientSocket dd ?
ServerSocket dd ?

include 'weblook.inc'

dataseg
sMsgNr1 db 'Socket Interface not available!',13,10
sMsgNr4 db 'On-line. Use Ctrl+Break to end.',13,10

dataseg
szMsgNr0 db 'socket',0
szMsgNr1 db 'setsockopt',0
szMsgNr2 db 'bind',0
szMsgNr3 db 'listen',0
szMsgNr4 db 'accept',0
szMsgNr5 db 'recv',0
szMsgNr6 db 'send',0
szMsgNr7 db 'soclose',0
label szMsgNr8 byte

codeseg
proc SupplyThread c near
arg @@parameter:dword
; open webeye server socket
; call DosWrite c,1,offset(szMsgNr0),szMsgNr1-szMsgNr0,offset(BytesDone)
  call socket c,2,1,6 ; create
  cmp eax,-1 ; check for errors
  mov ebx,offset(szMsgNr0)
  je EndSupplyThread ; failure
  mov [ServerSocket],eax
; avoid address in use error
; call DosWrite c,1,offset(szMsgNr1),szMsgNr2-szMsgNr1,offset(BytesDone)
  call setsockopt c,[ServerSocket],65535,[SocketOption],offset(SocketOption),4
  cmp eax,-1 ; check for errors
  mov ebx,offset(szMsgNr1)
  je EndSupplyThread ; failure
; reserve webeye server port
; call DosWrite c,1,offset(szMsgNr2),szMsgNr3-szMsgNr2,offset(BytesDone)
  call bind c,[ServerSocket],offset(SocketAddress),16
  cmp eax,-1 ; check for errors
  mov ebx,offset(szMsgNr2)
  je EndSupplyThread ; failure
; allow client connections
; call DosWrite c,1,offset(szMsgNr3),szMsgNr4-szMsgNr3,offset(BytesDone)
  call listen c,[ServerSocket],backlog
  cmp eax,-1 ; check for errors
  mov ebx,offset(szMsgNr3)
  je EndSupplyThread ; failure
; issue server is on-line message
  call DosWrite c,1,offset(sMsgNr4),33,offset(BytesDone)
  push ebp ; save register
label InspectNextSupply near
  dec [SupplyCount] ; available
  jnz ProcessNextSupply ; yes
; show application waiting message
; call DosWrite c,1,offset(szGood2),10,offset(BytesDone)
; await supply image semaphore
  call DosWaitEventSem c,[SupplyImage],-1
  test eax,eax ; check for errors
  jnz BadSupplyThread ; failure
; update currently available image buffers
  call DosResetEventSem c,[SupplyImage],offset(SupplyPosts)
  test eax,eax ; check for errors
  jnz BadSupplyThread ; failure
  mov eax,[SupplyPosts] ; additional buffers available
  add [SupplyCount],eax ; buffers available
label ProcessNextSupply near
; obtain next buffer pointer
  mov eax,[SupplyPtr] ; current
  mov eax,[eax] ; chain pointer
  mov [SupplyPtr],eax ; current
  mov edi,[eax+8] ; last address
  mov esi,[eax+4] ; this buffer
; check for end of frame
  cmp [StreamFormat],'MJPG'
  jne EndSupplyMJPG ; no
; verify mjpg image present
  cmp [dword(esi)],0E0FFD8FFh
  jne SyncReset ; skip
  cmp [word(edi-2)],0D9FFh
  jne SyncReset ; skip
label EndSupplyMJPG near
; check for end of frame
  cmp [StreamFormat],'YUYV'
  jne EndSupplyYUYV ; no
; verify yuyv image complete
  lea eax,[esi+Ix*Iy*2]
  cmp eax,edi ; complete
  jne SyncReset ; skip
; make rgb from yuyv
  call yuyv2rgb ; rgb
; make mjpg from rgb
  call rgb2jpg ; mjpg
label EndSupplyYUYV near
; accept client connection
; call DosWrite c,1,offset(szMsgNr4),szMsgNr5-szMsgNr4,offset(BytesDone)
  call accept c,[ServerSocket],0,0
  cmp eax,-1 ; check for errors
  mov ebx,offset(szMsgNr4)
  je BadSupplyThread ; failure
  mov [ClientSocket],eax
; send response to client
  sub edi,esi ; image size
; call DosWrite c,1,offset(szMsgNr6),szMsgNr7-szMsgNr6,offset(BytesDone)
  call send c,[ClientSocket],esi,edi,0
  cmp eax,-1 ; check for errors
  mov ebx,offset(szMsgNr6)
  je CloseClient ; failure
label CloseClient near
; close client connection
; call DosWrite c,1,offset(szMsgNr7),szMsgNr8-szMsgNr7,offset(BytesDone)
  call soclose c,[ClientSocket]
  cmp eax,-1 ; check for errors
  mov ebx,offset(szMsgNr7)
  je BadSupplyThread ; failure
  mov [ClientSocket],0
label SyncReset near
; deliver buffer to obtain thread
  call DosPostEventSem c,[ObtainImage]
  cmp eax,012Bh ; already posted
  je InspectNextSupply ; continue
  test eax,eax ; check for errors
  jz InspectNextSupply ; continue
label BadSupplyThread near
  pop ebp ; restore register
; show appropriate error message
  call ShowReturnCode ; info
label EndSupplyThread near
; report the server error
  call psock_errno c,ebx
  call sock_errno
; exit the thread
  call DosExit c,0,0
endp SupplyThread

dataseg
ColorIndex dd 3 ; default
; ITU-R BT.601 - SDTV standard
; precalculated yuyv2rgb coefficients
; y=000..255,u=000..255,v=000..255
ayc0 dd 16777216 ; 1.00000000*2E24
buc0 dd 29611786 ; 1.765*2E24
guc0 dd 05754585 ; 0.343*2E24
gvc0 dd 11928600 ; 0.711*2E24
rvc0 dd 23488102 ; 1.400*2E24
yzc0 dd 00000000 ; yMin=000
; ITU-R BT.601 - SDTV standard
; precalculated yuyv2rgb coefficients
; y=016..240,u=016..240,v=016..240
ayc1 dd 19173961 ; 1.14285714*2E24
buc1 dd 33839645 ; 2.017*2E24
guc1 dd 06576669 ; 0.392*2E24
gvc1 dd 13639877 ; 0.813*2E24
rvc1 dd 26776437 ; 1.596*2E24
yzc1 dd 00000016 ; yMin=016
; ITU-R BT.601 - SDTV standard
; precalculated yuyv2rgb coefficients
; y=016..235,u=016..240,v=016..240
ayc2 dd 19535115 ; 1.16438356*2E24
buc2 dd 33839645 ; 2.017*2E24
guc2 dd 06576669 ; 0.392*2E24
gvc2 dd 13639877 ; 0.813*2E24
rvc2 dd 26776437 ; 1.596*2E24
yzc2 dd 00000016 ; yMin=016
; ITU-R BT.709 - HDTV standard
; precalculated yuyv2rgb coefficients
; y=016..235,u=016..240,v=016..240
ayc dd 19535115 ; 1.16438356*2E24
buc dd 35433480 ; 2.112*2E24
guc dd 03573547 ; 0.213*2E24
gvc dd 08942256 ; 0.533*2E24
rvc dd 30081548 ; 1.793*2E24
yzc dd 00000016 ; yMin=016

udataseg
rgbBuffer db Ix*Iy*3 dup(?)

codeseg
proc yuyv2rgb near
; convert yuyv to rgb
  mov edi,offset(rgbBuffer+Ix*Iy*3)
label ConvertThisLine near
  mov ecx,Ix/2 ; dwords
; update line pointer
  lea edi,[edi-Ix*3]
; convert yuyv to rgb
label ConvertPixels near
; calculate 1st Y value
  movzx edx,[byte(esi)+0]
  mov eax,[ayc] ; ayf*2E24
  sub edx,[yzc] ; (Y-Z)*2E0
  shl edx,16 ; (Y-Z)*2E16
  mul edx ; ayf*(Y-Z)*2E8
  mov ebp,edx ; retain
; calculate 2nd Y value
  movzx edx,[byte(esi)+2]
  mov eax,[ayc] ; ayf*2E24
  sub edx,[yzc] ; (Y-Z)*2E0
  shl edx,16 ; (Y-Z)*2E16
  mul edx ; ayf*(Y-Z)*2E8
  mov ebx,edx ; retain
; calculate 1st U value
  movzx edx,[byte(esi)+1]
  mov eax,[buc] ; buf*2E24
  sub edx,80h ; (U-80h)*2E0
  shl edx,16 ; (U-80h)*2E16
  imul edx ; buf*(U-80h)*2E8
; calculate 1st B value
  mov eax,ebp ; 1st Y*2E8
  add eax,edx ; 1st B*2E8
  sar eax,8 ; keep 8 bits
  adc eax,0 ; round result
  test ah,ah ; validate
  jz Store1stB ; correct
  mov al,000h ; minimum
  js Store1stB ; correct
  mov al,0FFh ; maximum
label Store1stB near
  mov [byte(edi+0)],al
; calculate 2nd B value
  mov eax,ebx ; 2nd Y*2E8
  add eax,edx ; 2nd B*2E8
  sar eax,8 ; keep 8 bits
  adc eax,0 ; round result
  test ah,ah ; validate
  jz Store2ndB ; correct
  mov al,000h ; minimum
  js Store2ndB ; correct
  mov al,0FFh ; maximum
label Store2ndB near
  mov [byte(edi+3)],al
; calculate 2nd V value
  movzx edx,[byte(esi)+3]
  mov eax,[rvc] ; rvf*2E24
  sub edx,80h ; (V-80h)*2E0
  shl edx,16 ; (V-80h)*2E16
  imul edx ; rvf*(V-80h)*2E8
; calculate 1st R value
  mov eax,ebp ; 1st Y*2E8
  add eax,edx ; 1st R*2E8
  sar eax,8 ; keep 8 bits
  adc eax,0 ; round result
  test ah,ah ; validate
  jz Store1stR ; correct
  mov al,000h ; minimum
  js Store1stR ; correct
  mov al,0FFh ; maximum
label Store1stR near
  mov [byte(edi+2)],al
; calculate 2nd R value
  mov eax,ebx ; 2nd Y*2E8
  add eax,edx ; 2nd R*2E8
  sar eax,8 ; keep 8 bits
  adc eax,0 ; round result
  test ah,ah ; validate
  jz Store2ndR ; correct
  mov al,000h ; minimum
  js Store2ndR ; correct
  mov al,0FFh ; maximum
label Store2ndR near
  mov [byte(edi+5)],al
; calculate 2nd U value
  movzx edx,[byte(esi)+1]
  mov eax,[guc] ; guf*2E24
  sub edx,80h ; (U-80h)*2E0
  shl edx,16 ; (U-80h)*2E16
  imul edx ; guf*(U-80h)*2E8
; calculate 1st G value
  sub ebp,edx ; 1st G*2E8
; calculate 2nd G value
  sub ebx,edx ; 2nd G*2E8
; calculate 1st V value
  movzx edx,[byte(esi)+3]
  mov eax,[gvc] ; gvf*2E24
  sub edx,80h ; (V-80h)*2E0
  shl edx,16 ; (V-80h)*2E16
  imul edx ; gvf*(V-80h)*2E8
; calculate 1st G value
  mov eax,ebp ; 1st G*2E8
  sub eax,edx ; 1st G*2E8
  sar eax,8 ; keep 8 bits
  adc eax,0 ; round result
  test ah,ah ; validate
  jz Store1stG ; correct
  mov al,000h ; minimum
  js Store1stG ; correct
  mov al,0FFh ; maximum
label Store1stG near
  mov [byte(edi+1)],al
; calculate 2nd G value
  mov eax,ebx ; 2nd G*2E8
  sub eax,edx ; 2nd G*2E8
  sar eax,8 ; keep 8 bits
  adc eax,0 ; round result
  test ah,ah ; validate
  jz Store2ndG ; correct
  mov al,000h ; minimum
  js Store2ndG ; correct
  mov al,0FFh ; maximum
label Store2ndG near
  mov [byte(edi+4)],al
; update pixel pointers
  lea esi,[esi+4] ; yuuv
  lea edi,[edi+6] ; rgb
; loop till line complete
  dec ecx ; next dword
  jnz ConvertPixels
; loop till image complete
  lea edi,[edi-Ix*3] ; line
  cmp edi,offset(rgbBuffer)
  jnb ConvertThisLine
  ret ; return
endp yuyv2rgb

codeseg
proc dec2bin near
; decimal to binary
  sub eax,eax ; input
  sub edx,edx ; output
label ConvertInput near
  inc edi ; next position
  mov al,[edi] ; digit
; convert decimal digit
  cmp al,'0' ; minimum
  jb Enddec2bin ; done
  cmp al,'9' ; maximum
  ja Enddec2bin ; done
  sub al,'0' ; digit
  lea edx,[edx*4+edx]
  lea edx,[edx*2+eax]
  jmp ConvertInput
label Enddec2bin Near
  ret ; return
endp dec2bin

dataseg
MaxAlternate dd -1;

dataseg
VideoMJPG db '1'
VideoYUYV db '1'

codeseg
proc ProcessArguments near
; scan for forward slash
  mov al,[edi] ; character
  inc edi ; next position
  cmp al,00h ; terminator
  je EndScanString ; done
  cmp al,'/' ; parameter
  jne ProcessArguments
; color index argument
  cmp [byte(edi)],'c'
  jne NotColorIndex
  call dec2bin ; convert
; check color index value
  cmp edx,3 ; max value
  jnb ProcessArguments
; store color index value
  mov [ColorIndex],edx
  jmp ProcessArguments
label NotColorIndex near
  cmp [byte(edi)],'i'
  jne NotAltSetting
  call dec2bin ; convert
  mov [MaxAlternate],edx
  jmp ProcessArguments
label NotAltSetting near
; video MJPG argument
  cmp [byte(edi)],'m'
  jne NotVideoMJPG
  mov [VideoMJPG],'1'
  mov [VideoYUYV],'0'
  jmp ProcessArguments
label NotVideoMJPG near
; video YUYV argument
  cmp [byte(edi)],'u'
  jne NotVideoYUYV
  mov [VideoYUYV],'1'
  mov [VideoMJPG],'0'
  jmp ProcessArguments
label NotVideoYUYV near
; tcp/ip port argument
  cmp [byte(edi)],'p'
  jne ProcessArguments
  call dec2bin ; convert
; check tcp/ip port number
  cmp edx,0FFFFh ; max
  ja ProcessArguments
  cmp edx,00400h ; min
  jb ProcessArguments
; store tcp/ip port number
  mov [SocketAddress+3],dl
  mov [SocketAddress+2],dh
  jmp ProcessArguments
label EndScanString near
; set color coefficients
  mov edx,[ColorIndex]
  cmp edx,3 ; default
  jnb EndSetValues
  mov edi,offset(ayc)
  mov esi,offset(ayc0)
  lea esi,[esi+edx*8]
  lea esi,[esi+edx*8]
  lea esi,[esi+edx*8]
  mov ecx,6 ; dwords
  rep movsd ; values
label EndSetValues near
; skip sanity checks
  ret ; return
endp ProcessArguments

udataseg
BytesRead dd ?
DevNumber dd ?
DevReport db ReportSize dup(?)
ReportSize = 4096

udataseg
bcdDevice dd ?
idProduct dd ?
idVendor dd ?

codeseg
proc ObtainUvcDevice near
  call DosSleep c,100 ; mSeconds
; obtain number of attached usb devices
  call UsbQueryNumberDevices c,offset(DevNumber)
  test eax,eax ; check for errors
  jnz EndUvcDevice ; stop
label InspectThisDevice near
  cmp [DevNumber],eax ; present
  jna ObtainUvcDevice ; retry
; look for attached uvc device
  mov [BytesRead],ReportSize
  call UsbQueryDeviceReport c,[DevNumber],offset(BytesRead),offset(DevReport)
  test eax,eax ; check for errors
  jnz EndUvcDevice ; stop
  dec [DevNumber] ; previous
  cmp [DevReport+04h],0EFh
  jne InspectThisDevice
  cmp [DevReport+05h],002h
  jne InspectThisDevice
  cmp [DevReport+06h],001h
  jne InspectThisDevice
; use attached uvc device
  mov ax,[word(DevReport)+08h]
  mov [dword(idVendor)],eax
  mov ax,[word(DevReport)+0Ah]
  mov [dword(idProduct)],eax
  mov ax,[word(DevReport)+0Ch]
  mov [dword(bcdDevice)],eax
label EndUvcDevice near
  ret ; return
endp ObtainUvcDevice

udataseg
AltInterface dd ?
NumInterface dd ?

udataseg
AddrEndpoint dd ?
IsoFrameSize dd ?
IsoFrameUsed dd ?

dataseg
StreamFormat dd 'Huh?'

codeseg
proc ProcessDescriptors near
; address compound descriptor
  mov esi,offset(DevReport)
  mov ebx,[BytesRead] ; size
  lea ebx,[esi+ebx] ; boundary
  mov eax,[esi] ; size/type
Label NextDescriptor near
; point to next descriptor
  and eax,0FFh ; length
  add esi,eax ; next
  cmp esi,ebx ; boundary
  jnb EndDescriptor ; stop
  mov eax,[esi] ; size/type
label FindDescriptor near
; find interface descriptor
  cmp ah,04h ; interface
  jne NextDescriptor
  cmp [byte(esi)+5],0Eh
  jne NextDescriptor
  cmp [byte(esi)+6],02h
  jne NextDescriptor
; verify having bandwidth
  cmp [byte(esi)+3],00h
  jnz ObtainEndpoint
; video streaming interface
Label NextVideoStream near
; point to next descriptor
  and eax,0FFh ; length
  add esi,eax ; next
  cmp esi,ebx ; boundary
  jnb EndDescriptor ; stop
  mov eax,[esi] ; size/type
  cmp ah,24h ; streaming
  jne FindDescriptor
; decode video streaming
  shld edx,eax,16 ; subtype
; verify decode yuyv video
  cmp [VideoYUYV],'1' ; use
  jne EndDecodeYUYV ; skip
  cmp dl,04h ; format yuyv
  je FormatInxYUYV
  cmp dl,05h ; frame yuyv
  je FrameInxYUYV
label EndDecodeYUYV near
; verify decode mjpg video
  cmp [VideoMJPG],'1' ; use
  jne EndDecodeMJPG ; skip
  cmp dl,06h ; format mjpg
  je FormatInxMJPG
  cmp dl,07h ; frame mjpg
  je FrameInxMJPG
label EndDecodeMJPG near
  jmp NextVideoStream
label FormatInxMJPG near
;------------------------------------------------------------------------------
  call ShowReturnCode ; info
;------------------------------------------------------------------------------
; setup bFormatIndex
  mov [Negotiate+2],dh
  jmp NextVideoStream
label FormatInxYUYV near
;------------------------------------------------------------------------------
  call ShowReturnCode ; info
;------------------------------------------------------------------------------
; setup bFormatIndex
  mov [Negotiate+2],dh
  jmp NextVideoStream
label FrameInxMJPG near
; frame index Ix*Iy mjpg
  cmp [word(esi)+5],Ix
  jne NextVideoStream
  cmp [word(esi)+7],Iy
  jne NextVideoStream
;------------------------------------------------------------------------------
  call ShowReturnCode ; info
;------------------------------------------------------------------------------
; setup bFrameIndex
  mov [Negotiate+3],dh
; setup dwDefaultFrameInterval
  mov edx,[esi+21] ; default
  mov [dword(Negotiate)+4],edx
; switch video stream format
  mov [StreamFormat],'MJPG'
  jmp NextVideoStream
label FrameInxYUYV near
; frame index Ix*Iy yuyv
  cmp [word(esi)+5],Ix
  jne NextVideoStream
  cmp [word(esi)+7],Iy
  jne NextVideoStream
;------------------------------------------------------------------------------
  call ShowReturnCode ; info
;------------------------------------------------------------------------------
; setup bFrameIndex
  mov [Negotiate+3],dh
; setup dwDefaultFrameInterval
  mov edx,[esi+21] ; default
  mov [dword(Negotiate)+4],edx
; switch video stream format
  mov [StreamFormat],'YUYV'
  jmp NextVideoStream
label ObtainEndpoint near
; verify alternate setting
  mov edx,[AltInterface]
  cmp edx,[MaxAlternate]
  jnb NextDescriptor
; setup bInterfaceNumber
  movzx edx,[byte(esi)+2]
  mov [NumInterface],edx
; setup bAlternateSetting
  movzx edx,[byte(esi)+3]
  mov [AltInterface],edx
label NextEndpoint near
; point to next descriptor
  and eax,0FFh ; length
  add esi,eax ; next
  cmp esi,ebx ; boundary
  jnb EndDescriptor ; stop
  mov eax,[esi] ; size/type
  cmp ah,05h ; endpoint
  jne FindDescriptor
; setup bEndpointAddress
  movzx edx,[byte(esi)+2]
  mov [AddrEndpoint],edx
; setup wMaxPacketSize
  movzx edx,[word(esi)+4]
  mov [IsoFrameSize],edx
  jmp NextDescriptor
label EndDescriptor near
;------------------------------------------------------------------------------
; show video stream format
  mov eax,[StreamFormat]
  bswap eax ; proper format
  mov [dword(szInfo5)],eax
  call DosWrite c,1,offset(szInfo5),6,offset(BytesDone)
;------------------------------------------------------------------------------
; update iso frame length
  mov eax,[IsoFrameSize]
  shld edx,eax,21 ; multi
  and ah,07h ; isolate
  and dl,03h ; isolate
  mov [IsoFrameSize],eax
  cmp dl,01h ; double
  jb EndMultiple ; single
  je AddMultiple ; double
  add eax,eax ; triple
label AddMultiple near
  add [IsoFrameSize],eax
label EndMultiple near
; verify iso frame length
  mov eax,[IsoFrameSize]
  call ShowReturnCode
  test eax,eax ; present
  jnz EndVerifySize ; yes
; report invalid frame size
  call DosWrite c,1,offset(szFail3),19,offset(BytesDone)
  mov al,1 ; failure
  test eax,eax
  ret ; return
label EndVerifySize near
; calculate iso frame count
  sub edx,edx ; ensure zeroes
  mov eax,dSize ; data size
  div [IsoFrameSize] ; count
  and al,0F8h ; microframes
  mov [IsoFrameUsed],eax
;------------------------------------------------------------------------------
  call ShowReturnCode ; info
;------------------------------------------------------------------------------
; apply sanity checks
  cmp esi,ebx ; boundary
  ret ; return
endp ProcessDescriptors

end MainRoutine
