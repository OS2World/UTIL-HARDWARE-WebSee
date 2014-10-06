.486p
model flat
ideal

vga=0
vga=1

if vga
; 640*480
  Ix=640/1
  Iy=480/1
  Ox=Ix/2
  Oy=Iy/2
else
; 320*240
  Ix=640/2
  Iy=480/2
  Ox=Ix/1
  Oy=Iy/1
endif

extrn DosBeep:near
extrn DosClose:near
extrn DevCloseDC:near
extrn DosCreateThread:near
extrn DosExit:near
extrn DosFreeMem:near
extrn DosOpen:near
extrn DevOpenDC:near
extrn DosSetPriority:near
extrn DosSleep:near
extrn DosSuspendThread:near
extrn DosWrite:near

extrn GpiBitBlt:near
extrn GpiCreateBitmap:near
extrn GpiCreatePS:near
extrn GpiDeleteBitmap:near
extrn GpiDestroyPS:near
extrn GpiSetBitmap:near
extrn GpiSetBitmapBits:near

extrn RexxStart:near

extrn WinBeginPaint:near
extrn WinCreateMsgQueue:near
extrn WinCreateStdWindow:near
extrn WinCreateWindow:near
extrn WinDefWindowProc:near
extrn WinDestroyMsgQueue:near
extrn WinDestroyWindow:near
extrn WinDispatchMsg:near
extrn WinEndPaint:near
extrn WinGetMsg:near
extrn WinInitialize:near
extrn WinInvalidateRect:near
extrn WinLoadString:near
extrn WinMessageBox:near
extrn WinPostMsg:near
extrn WinPostQueueMsg:near
extrn WinQuerySysValue:near
extrn WinRegisterClass:near
extrn WinSetActiveWindow:near
extrn WinSetWindowPos:near
extrn WinSubclassWindow:near
extrn WinTerminate:near

extrn connect:near
extrn recv:near
extrn send:near
extrn soclose:near
extrn socket:near

stack 8192

include 'webview.inc'

dataseg
flCreateFlags dd 00000D37h
szClientClass db 'WEBVIEW - Client Window',0

udataseg
szMessageText db 255 dup(?)
szWindowTitle db 255 dup(?)

udataseg
hab dd ?
hmq dd ?
hwndClient dd ?
hwndFrame  dd ?
qmsg dd 8 dup(?)

udataseg
xWindowSize dd ?
yWindowSize dd ?

udataseg
ActionTaken dd ?

udataseg
tidObtain dd ?

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
; obtain anchor block handle
  call WinInitialize c,0
  test eax,eax ; success
  jz EndProcess ; failure
  mov [hab],eax ; save
; obtain program title
  call WinLoadString c,[hab],0,00h,255,offset(szWindowTitle)
  test eax,eax ; success
  jz EndMainMsgQueue ; no
; obtain message queue handle
  call WinCreateMsgQueue c,[hab],0
  test eax,eax ; success
  jz EndMainMsgQueue ; no
  mov [hmq],eax ; save
; start obtain control thread
  call DosCreateThread c,offset(tidObtain),offset(ObtainThread),0,2,8192
; register client window class
  call WinRegisterClass c,[hab],offset(szClientClass),offset(ClientWinProc),0,0
  test eax,eax ; success
  jz EndWindow ; failure
; create frame and client windows
  call WinCreateStdWindow c,1,0,offset(flCreateFlags),offset(szClientClass),offset(szWindowTitle),0,0,1,offset(hwndClient)
  test eax,eax ; success
  jz EndWindow ; failure
  mov [hwndFrame],eax ; save
; replace frame window procedure
  call WinSubclassWindow c,[hwndFrame],offset(FrameWinProc)
  test eax,eax ; success
  jz EndScreen ; failure
  mov [WinOldWindowProc],eax
; resize and show usb camera screen
  call WinQuerySysValue c,1,28 ; SV_CXDLGFRAME
  lea eax,[eax*2+Ox+0] ; calculate width
  mov [xWindowSize],eax ; small width
  call WinQuerySysValue c,1,29 ; SV_CYDLGFRAME
  lea eax,[eax*2+Oy-1] ; calculate height
  mov [yWindowSize],eax ; small height
  call WinQuerySysValue c,1,30 ; SV_CYTITLEBAR
  add [yWindowSize],eax ; small height
  call WinQuerySysValue c,1,35 ; SV_CYMENU
  add [yWindowSize],eax ; small height
  call WinSetWindowPos c,[hwndFrame],0,0,0,[xWindowSize],[yWindowSize],1089h
  test eax,eax ; success
  jz EndScreen ; failure
label ProcessMessage near
; obtain message from the message queue
  call WinGetMsg c,[hab],offset(qmsg),0,0,0
  test eax,eax ; continue message
  jz EndProcessMessage ; quit message
; dispatch message to client window procedure
  call WinDispatchMsg c,[hab],offset(qmsg)
  jmp ProcessMessage
label EndProcessMessage near
label EndScreen near
  call WinDestroyWindow c,[hwndFrame]
label EndWindow near
; suspend obtain control thread
  call DosSuspendThread c,[tidObtain]
; release message queue handle
  call WinDestroyMsgQueue c,[hmq]
label EndMainMsgQueue near
  call WinTerminate c,[hab]
label EndProcess near
; exit the process
  call DosExit c,1,0
endp MainRoutine

dataseg
bhMemory dw 12,0,Ix,Iy,1,24
paMemory dd 0,0,Ox,Oy,0,0,Ix,Iy
srMemory dd 0,0

dataseg
szDeviceToken db '*',0

udataseg
hbmMemory dd ?
hdcMemory dd ?
hdcScreen dd ?
hpsMemory dd ?
hpsScreen dd ?

dataseg
SnapShot db 0 ; done

codeseg
proc ClientWinProc c near
arg @@hwnd,@@msg,@@mp1,@@mp2
; refresh video display screen
  cmp [@@msg],23h ; WM_PAINT
  jne NotRefreshScreen
  call WinBeginPaint c,[@@hwnd],0,0
  mov [hpsScreen],eax ; save
  call GpiSetBitmapBits c,[hpsMemory],0,Iy,offset(rgbBuffer),offset(bhMemory)
  call GpiBitBlt c,[hpsScreen],[hpsMemory],4,offset(paMemory),0CCh,2
  call WinEndPaint c,[hpsScreen]
  mov eax,1 ; done
  ret ; return
label NotRefreshScreen near
; handle command input
  cmp [@@msg],20h ; WM_COMMAND
  jne NotCommand
; take snapshot
  mov eax,[@@mp1]
  cmp al,1 ; SnapShot
  jb NotSnapshot
  cmp al,2 ; TestShot
  ja NotSnapshot
  mov [SnapShot],al
label NotSnapshot near
  sub eax,eax ; reserved
  ret ; return
label NotCommand near
; exit usb camera request
  cmp [@@msg],2Ah ; WM_QUIT
  jne NotExitCamera
  mov eax,1 ; done
  ret ; return
label NotExitCamera near
; create video display screen
  cmp [@@msg],1 ; WM_CREATE
  jne NotCreateScreen
  call DevOpenDC c,[hab],8,offset(szDeviceToken),0,0,0
  mov [hdcMemory],eax ; save
  call GpiCreatePS c,[hab],[hdcMemory],offset(srMemory),5008h
  mov [hpsMemory],eax ; save
  call GpiCreateBitmap c,[hpsMemory],offset(bhMemory),0,0,0
  mov [hbmMemory],eax ; save
  call GpiSetBitmap c,[hpsMemory],[hbmMemory]
  sub eax,eax ; continue
  ret ; return
label NotCreateScreen near
; destroy video display sceen
  cmp [@@msg],2 ; WM_DESTROY
  jne NotDestroyScreen
  call GpiDeleteBitmap c,[hbmMemory]
  call GpiDestroyPS c,[hdcMemory]
  call DevCloseDC c,[hpsMemory]
  mov eax,1 ; done
  ret ; return
label NotDestroyScreen near
; pass message to default window processing
  call WinDefWindowProc c,[@@hwnd],[@@msg],[@@mp1],[@@mp2]
  ret ; return
endp ClientWinProc

udataseg
WinOldWindowProc dd ?

codeseg
proc FrameWinProc c near
arg @@hwnd,@@msg,@@mp1,@@mp2
uses ebx,ecx,edx,edi,esi
; handle window minimize/maximize
  cmp [@@msg],46h ; WM_MINMAXFRAME
  jne PassToOldFrameProcedure
  mov eax,[@@mp1] ; swp pointer
; set maximum window size
  test [dword(eax)+0*4],800h ; SWP_MAXIMIZE
  jz NotMaximizeWindow
; preset small camera screen size
  mov ecx,[yWindowSize] ; small height
  mov edx,[xWindowSize] ; small width
; check large size fits on screen
  lea ebx,[ecx+Oy] ; large height
  cmp ebx,[eax+1*4] ; old cymax
  ja UpdateHeightPosition ; no
  lea ebx,[edx+Ox] ; large width
  cmp ebx,[eax+2*4] ; old cxmax
  ja UpdateHeightPosition ; no
; setup large camera screen size
  lea ecx,[ecx+Oy] ; large height
  lea edx,[edx+Ox] ; large width
; update window height/width
  mov [paMemory+3*4],Oy*2
  mov [paMemory+2*4],Ox*2
label UpdateHeightPosition near
; update maximum height/position
  mov ebx,[eax+1*4] ; old cymax
  mov [eax+1*4],ecx ; new cymax
  sub ebx,ecx ; max position
  shr ebx,1 ; center position
  add [eax+3*4],ebx ; ypos
; update maximum width/position
  mov ebx,[eax+2*4] ; old cxmax
  mov [eax+2*4],edx ; new cxmax
  sub ebx,edx ; max position
  shr ebx,1 ; center position
  add [eax+4*4],ebx ; xpos
  jmp PassToOldFrameProcedure
label NotMaximizeWindow near
; set restored window size
  test [dword(eax)+0*4],1000h ; SWP_RESTORE
  jz NotRestoreWindow
; update window height/width
  mov [paMemory+3*4],Oy*1
  mov [paMemory+2*4],Ox*1
label NotRestoreWindow near
label PassToOldFrameProcedure near
; pass message to default window processing
  call [WinOldWindowProc] c,[@@hwnd],[@@msg],[@@mp1],[@@mp2]
  ret ; return
endp FrameWinProc

udataseg
ClientSocket dd ?

dataseg
port=14225
SocketAddress db 2,0,high(port),low(port),127,0,0,1,8 dup(0) ; localhost

udataseg
; image buffer
InBuffer db Ix*Iy dup(?)

dataseg
GoodInput db 0

codeseg
proc ObtainThread c near
arg @@parameter:dword
local @@hab:dword
local @@hmq:dword
; obtain anchor block handle
  call WinInitialize c,0
  test eax,eax ; success
  jz EndObtainThread
  mov [@@hab],eax ; save
; obtain message queue handle
  call WinCreateMsgQueue c,[@@hab],0
  test eax,eax ; success
  jz EndObtainMsgQueue
  mov [@@hmq],eax ; save
  push ebp ; save register
label TryIsochronousread near
; limit maximum polling rate
  call DosSleep c,30 ; mSeconds
label RawIsochronousRead near
; take usb camera snapshot
  cmp [SnapShot],1 ; take
  jne EndTakeSnapShot ; no
  mov [SnapShot],0 ; done
; write usb camera bmap image
  call rgb2bmp ; webview.bmp
; write usb camera jpeg image
  call rgb2jpg ; webview.jpg
; invoke rexx post processing
  call RexxStart c,1,offset(ArgList),offset(RexxFile),0,0,0,0,offset(ReturnCode),offset(Result)
  test eax,eax ; return code
  jnz EndFreeString ; failure
; handle exit/return value
  mov edi,[Result+4] ; buffer
  test edi,edi ; allocated
  jz EndFreeString ; no
; show exit/return string
  cmp eax,[Result] ; empty
  je EndShowString ; yes
  mov [Result],eax ; empty
  call WinMessageBox c,1,1,edi,offset(szWindowTitle),0,4030h
label EndShowString near
; free allocated storage
  call DosFreeMem c,edi
  test eax,eax ; any errors
  jnz EndFreeString ; failure
  mov [Result+4],eax ; free
label EndFreeString near
label EndTakeSnapShot near
; open webview client socket
  call socket c,2,1,6 ; create
  cmp eax,-1 ; check for errors
  jne SocketCreated ; success
; show appropriate error message
  call WinLoadString c,[hab],0,01h,255,offset(szMessageText)
  call WinMessageBox c,1,1,offset(szMessageText),offset(szWindowTitle),0,4046h
  jmp EndSocketAttempt
label SocketCreated near
  mov [ClientSocket],eax
; connect to server socket
  call connect c,[ClientSocket],offset(SocketAddress),16
  cmp eax,-1 ; check for errors
  je CloseClient ; failure
; point to input buffer
  mov edi,offset(InBuffer)
label ObtainResponse near
; protect against buffer overflow
  cmp edi,offset(Inbuffer+Ix*Iy-1024)
  ja CloseClient ; ignore input
; obtain http response from server
  call recv c,[ClientSocket],edi,1024,0
  cmp eax,-1 ; check for errors
  je CloseClient ; failure
  add edi,eax ; bump pointer
  test eax,eax ; more
  jnz ObtainResponse
; mark input received
  mov [GoodInput],1
label CloseClient near
; close webview client socket
  call soclose c,[ClientSocket]
  cmp eax,-1 ; check for errors
  jne EndSocketAttempt ; success
; show appropriate error message
  call WinLoadString c,[hab],0,02h,255,offset(szMessageText)
  call WinMessageBox c,1,1,offset(szMessageText),offset(szWindowTitle),0,4046h
label EndSocketAttempt near
  mov [ClientSocket],0
; verify valid input received
  cmp [GoodInput],1 ; valid
  jne TryIsochronousRead
  mov [GoodInput],0 ; reset
; point to input buffer
  mov esi,offset(InBuffer)
; verify mjpg image present
  cmp [dword(esi)],0E0FFD8FFh
  jne RawIsochronousRead
  cmp [word(edi-2)],0D9FFh
  jne RawIsochronousRead
; take usb camera testshot
  cmp [SnapShot],2 ; take
  jne EndTakeTestShot ; no
  mov [SnapShot],0 ; done
; write usb camera jpeg image
  call jpg2raw ; webview.raw
label EndTakeTestShot near
; make rgb from mjpg
  mov [GoodImage],1
  call jpg2rgb
  cmp [GoodImage],1
  jne RawIsochronousRead
; trigger presentation manager screen update
  call WinInvalidateRect c,[hwndClient],0,0
  test eax,eax ; check for errors
  jnz RawIsochronousRead ; next
  pop ebp ; restore register
  call ShowReturnCode ; zero
; show appropriate error message
  call WinLoadString c,[@@hab],0,03h,244,offset(szMessageText+11)
  call WinMessageBox c,1,1,offset(szMessageText),offset(szWindowTitle),0,4046h
  call WinPostQueueMsg c,[hmq],2Ah,0,0 ; WM_QUIT
label EndObtainMsgQueue near
  call WinTerminate c,[@@hab]
label EndObtainThread near
; exit the obtain thread
  call DosExit c,0,0
endp ObtainThread

dataseg
hex2ascii db '0123456789ABCDEF'

codeseg
proc ShowReturnCode near
; prepare message text
  lea edi,[szMessageText]
  mov [byte(edi+0Ah)],' '
  mov [byte(edi+09h)],']'
  mov [byte(edi)],'['
; convert return code
  sub ecx,ecx ; count
  sub edx,edx ; index
  mov cl,08h ; digits
label ConvertDigit near
  mov dl,al ; one byte
  and dl,0Fh ; hex digit
  mov dl,[hex2ascii+edx]
  mov [edi+ecx],dl ; ascii
  shr eax,4 ; next one
  loop ConvertDigit
  ret ; return
endp ShowReturnCode

udataseg
outBuffer db Ix*Iy*1 dup(?)
rgbBuffer db Ix*Iy*3 dup(?)

dataseg
RexxFile db 'webview.cmd',0

udataseg
ArgList dd ?,?
ReturnCode dd ?
Result dd ?,?

dataseg
bmpFileHeader dw 4D42h,0,0,0,0,14+12,0,12,0,Ix,Iy,1,24
szOutput db 'webview.bmp',0

udataseg
BytesDone dd ?
fhOutput dd ?

codeseg
proc rgb2bmp near
; write usb camera bitmap file
  mov [dword(bmpFileHeader)+2],14+12+Ix*Iy*3 ; file size
  call DosOpen c,offset(szOutput),offset(fhOutput),offset(ActionTaken),0,0,012h,0191h,0
  test eax,eax ; check for errors
  jnz NotBitmap ; failure
  call DosWrite c,[fhOutput],offset(bmpFileHeader),14+12,offset(BytesDone)
  test eax,eax ; check for errors
  jnz EndBitmap ; failure
  call DosWrite c,[fhOutput],offset(rgbBuffer),Ix*Iy*3,offset(BytesDone)
  test eax,eax ; check for errors
  jnz EndBitmap ; failure
label EndBitmap near
; close bmp output file
  call DosClose c,[fhOutput]
label NotBitmap near
  ret ; return
endp rgb2bmp

dataseg
zsOutput db 'webview.raw',0

codeseg
proc jpg2raw near
; write usb camera jpeg file
  call DosOpen c,offset(zsOutput),offset(fhOutput),offset(ActionTaken),0,0,012h,0191h,0
  test eax,eax ; check for errors
  jnz NotOutput ; failure
  mov eax,edi ; after
  sub eax,esi ; start
  call DosWrite c,[fhOutput],esi,eax,offset(BytesDone)
; close raw output file
  call DosClose c,[fhOutput]
label NotOutput near
  ret ; return
endp jpg2raw

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

codeseg
proc ProcessArguments near
; scan for forward slash
  mov al,[edi] ; character
  inc edi ; next position
  cmp al,00h ; terminator
  je EndScanString ; done
  cmp al,'/' ; parameter
  jne ProcessArguments
; tcp/ip ipaddr argument
  cmp [byte(edi)],'a'
  jne NotIpAddress
  call dec2bin ; convert
  shrd ebx,edx,8 ; keep
; check 1st ip number
  shr edx,8 ; zeroes
  jnz ProcessArguments
  cmp [byte(edi)],'.'
  jne ProcessArguments
  call dec2bin ; convert
  shrd ebx,edx,8 ; keep
; check 2nd ip number
  shr edx,8 ; zeroes
  jnz ProcessArguments
  cmp [byte(edi)],'.'
  jne ProcessArguments
  call dec2bin ; convert
  shrd ebx,edx,8 ; keep
; check 3rd ip number
  shr edx,8 ; zeroes
  jnz ProcessArguments
  cmp [byte(edi)],'.'
  jne ProcessArguments
  call dec2bin ; convert
  shrd ebx,edx,8 ; keep
; check 4th ip number
  shr edx,8 ; zeroes
  jnz ProcessArguments
; store validated ip address
  mov [dword(SocketAddress+4)],ebx
  jmp ProcessArguments
label NotIpAddress near
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
; skip sanity checks
  ret ; return
endp ProcessArguments

end MainRoutine
