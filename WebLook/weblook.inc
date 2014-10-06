macro qTable qTableRow,qf0,qf1,qf2,qf3,qf4,qf5,qf6,qf7
qTableRow&0=qf0
qTableRow&1=qf1
qTableRow&2=qf2
qTableRow&3=qf3
qTableRow&4=qf4
qTableRow&5=qf5
qTableRow&6=qf6
qTableRow&7=qf7
endm

; chrominance - quantization factors
qTable c0,012,020,030,040,084,084,084,084
qTable c1,020,032,042,054,084,084,084,084
qTable c2,030,042,056,066,084,084,084,084
qTable c3,040,054,066,084,084,084,084,084
qTable c4,084,084,084,084,084,084,084,084
qTable c5,084,084,084,084,084,084,084,084
qTable c6,084,084,084,084,084,084,084,084
qTable c7,084,084,084,084,084,084,084,084

; luminance - quantization factors
qTable y0,008,010,010,010,011,013,015,018
qTable y1,010,014,014,014,014,014,014,015
qTable y2,010,014,014,015,017,019,022,026
qTable y3,010,014,015,016,018,021,024,028
qTable y4,011,014,017,018,020,022,026,030
qTable y5,013,014,019,021,022,026,029,034
qTable y6,015,014,022,024,026,029,033,038
qTable y7,018,015,026,028,030,034,038,042

; precalculated cos(a*pi/16)*2E30
cos0a = 1073741824 ; cos(0pi/16)*2E30
cos1a = 1053110175 ; cos(1pi/16)*2E30
cos2a = 0992008093 ; cos(2pi/16)*2E30
cos3a = 0892783697 ; cos(3pi/16)*2E30
cos4a = 0759250124 ; cos(4pi/16)*2E30
cos5a = 0596538995 ; cos(5pi/16)*2E30
cos6a = 0410903206 ; cos(6pi/16)*2E30
cos7a = 0209476638 ; cos(7pi/16)*2E30

udataseg
mcu dd 4*64 dup(?)

; precalculated aanfdct coefficients
a1 = cos4a ; cos(4pi/16)*2E30
a2 = cos2a-cos6a ; (cos(2pi/16)-cos(6pi/16))*2E30
a3 = cos4a ; cos(4pi/16)*2E30
a4 = cos2a+cos6a ; (cos(2pi/16)+cos(6pi/16))*2E30
a5 = cos6a ; cos(6pi/16)*2E30

; aanfdct algorithm (29 additions and 5 multiplications)
; Y.Arai, T.Agui, M.Nakajima. "A Fast DCT-SQ Scheme for Images".
; Transactions of IEICE, vol. E71, nø. 11, 1988, pp. 1095-1097.
;    step1    ,  step2 ,  step3 ,  step4 ,  step5 ,  step6
; b0=m[0]+m[7],c0=b0+b3,d2=c2+c3,e2=d2*a1,        ,m[0]=c0+c1
; b7=m[0]-m[7],c3=b0-b3,        ,        ,        ,m[4]=c0-c1
; b1=m[1]+m[6],c1=b1+b2,        ,        ,        ,m[2]=c3+e2
; b6=m[1]-m[6],c2=b1-b2,        ,        ,        ,m[6]=c3-e2
; b2=m[2]+m[5],c4=b4+b5,d8=c4-c6,e4=c4*a2,f4=e4+e8,m[5]=f7+f4
; b5=m[2]-m[5],c5=b5+b6,        ,e5=c5*a3,f6=e6+e8,m[3]=f7-f4
; b3=m[3]+m[4],c6=b6+b7,        ,e6=c6*a4,f5=b7+e5,m[1]=f5+f6
; b4=m[3]-m[4],        ,        ,e8=d8*a5,f7=b7-e5,m[7]=f5-f6
;    step1    ,  step2 ,  step3 ,  step4 ,  step5 ,  step6
; derived from Stefan Kuhr, thesis2side.pdf, figure 3.10

codeseg
proc aanfdct near
; point to first matrix
  sub ebp,ebp ; offset
label NextColsPass near
; b3=m[3]+m[4],b4=m[3]-m[4]
  mov edx,[mcu+ebp+3*8*4]
  mov edi,[mcu+ebp+4*8*4]
  lea esi,[edx+edi] ; b3
  sub edx,edi ; b4
  push esi ; b3
; b2=m[2]+m[5],b5=m[2]-m[5]
  mov ecx,[mcu+ebp+2*8*4]
  mov edi,[mcu+ebp+5*8*4]
  lea esi,[ecx+edi] ; b2
  sub ecx,edi ; b5
  push esi ; b2
; b1=m[1]+m[6],b6=m[1]-m[6]
  mov ebx,[mcu+ebp+1*8*4]
  mov edi,[mcu+ebp+6*8*4]
  lea esi,[ebx+edi] ; b1
  sub ebx,edi ; b6
  push esi ; b1
; b0=m[0]+m[7],b7=m[0]-m[7]
  mov eax,[mcu+ebp+0*8*4]
  mov edi,[mcu+ebp+7*8*4]
  lea esi,[eax+edi] ; b0
  sub eax,edi ; b7
  push esi ; b0
; c5=b6+b5,c4=b4+b5,c6=b6+b7
  lea esi,[ebx+ecx] ; c5
  lea ecx,[edx+ecx] ; c4
  lea ebx,[ebx+eax] ; c6
  mov edx,ecx ; c4
  push eax ; b7
; d8=c4-c6,e8=d8*a5
  sub edx,ebx ; d8
  mov eax,a5 ; a5
  imul edx ; e8/2E2
  shld edx,eax,2 ; e8
  mov edi,edx ; e8
; e4=c4*a2,f4=e4+e8
  mov eax,a2 ; a2
  imul ecx ; e4/2E2
  shld edx,eax,2 ; e4
  lea ecx,[edx+edi] ; f4
; e6=c6*a4,f6=e6+e8
  mov eax,a4 ; a4
  imul ebx ; e6/2E2
  shld edx,eax,2 ; e6
  lea ebx,[edx+edi] ; f6
; e5=c5*a3,f5=b7+e5,f7=b7-e5
  mov eax,a3 ; a3
  imul esi ; e5/2E2
  shld edx,eax,2 ; e5
  pop esi ; b7
  lea eax,[esi+edx] ; f5
  sub esi,edx ; f7
; m[5]=f7+f4,m[3]=f7-f4
  lea edx,[esi+ecx] ; m[5]
  sub esi,ecx ; m[3]
  mov [mcu+ebp+5*8*4],edx
  mov [mcu+ebp+3*8*4],esi
  pop esi ; b0
  pop edx ; b1
; m[1]=f5+f6,m[7]=f5-f6
  lea ecx,[eax+ebx] ; m[1]
  sub eax,ebx ; m[7]
  mov [mcu+ebp+1*8*4],ecx
  mov [mcu+ebp+7*8*4],eax
  pop ecx ; b2
  pop ebx ; b3
; c0=b0+b3,c3=b0-b3
  lea edi,[esi+ebx] ; c0
  sub esi,ebx ; c3
; c1=b1+b2,c2=b1-b2
  lea ebx,[edx+ecx] ; c1
  sub edx,ecx ; c2
; d2=c2+c3,e2=d2*a1
  add edx,esi ; d2
  mov eax,a1 ; a1
  imul edx ; e2/2E2
  shld edx,eax,2 ; e2
; m[2]=c3+e2,m[6]=c3-e2
  lea ecx,[esi+edx] ; m[2]
  sub esi,edx ; m[6]
  mov [mcu+ebp+2*8*4],ecx
  mov [mcu+ebp+6*8*4],esi
; m[0]=c0+c1,m[4]=c0-c1
  lea eax,[edi+ebx] ; m[0]
  sub edi,ebx ; m[4]
  mov [mcu+ebp+0*8*4],eax
  mov [mcu+ebp+4*8*4],edi
; bump to next matrix column
  lea ebp,[ebp+1*4] ; offset
  test ebp,01Ch ; eight
  jnz NextColsPass
; point to start of matrix
  lea ebp,[ebp-8*4] ; offset
label NextRowsPass near
; b3=m[3]+m[4],b4=m[3]-m[4]
  mov edx,[mcu+ebp+3*1*4]
  mov edi,[mcu+ebp+4*1*4]
  lea esi,[edx+edi] ; b3
  sub edx,edi ; b4
  push esi ; b3
; b2=m[2]+m[5],b5=m[2]-m[5]
  mov ecx,[mcu+ebp+2*1*4]
  mov edi,[mcu+ebp+5*1*4]
  lea esi,[ecx+edi] ; b2
  sub ecx,edi ; b5
  push esi ; b2
; b1=m[1]+m[6],b6=m[1]-m[6]
  mov ebx,[mcu+ebp+1*1*4]
  mov edi,[mcu+ebp+6*1*4]
  lea esi,[ebx+edi] ; b1
  sub ebx,edi ; b6
  push esi ; b1
; b0=m[0]+m[7],b7=m[0]-m[7]
  mov eax,[mcu+ebp+0*1*4]
  mov edi,[mcu+ebp+7*1*4]
  lea esi,[eax+edi] ; b0
  sub eax,edi ; b7
  push esi ; b0
; c5=b6+b5,c4=b4+b5,c6=b6+b7
  lea esi,[ebx+ecx] ; c5
  lea ecx,[edx+ecx] ; c4
  lea ebx,[ebx+eax] ; c6
  mov edx,ecx ; c4
  push eax ; b7
; d8=c4-c6,e8=d8*a5
  sub edx,ebx ; d8
  mov eax,a5 ; a5
  imul edx ; e8/2E2
  shld edx,eax,2 ; e8
  mov edi,edx ; e8
; e4=c4*a2,f4=e4+e8
  mov eax,a2 ; a2
  imul ecx ; e4/2E2
  shld edx,eax,2 ; e4
  lea ecx,[edx+edi] ; f4
; e6=c6*a4,f6=e6+e8
  mov eax,a4 ; a4
  imul ebx ; e6/2E2
  shld edx,eax,2 ; e6
  lea ebx,[edx+edi] ; f6
; e5=c5*a3,f5=b7+e5,f7=b7-e5
  mov eax,a3 ; a3
  imul esi ; e5/2E2
  shld edx,eax,2 ; e5
  pop esi ; b7
  lea eax,[esi+edx] ; f5
  sub esi,edx ; f7
; m[5]=f7+f4,m[3]=f7-f4
  lea edx,[esi+ecx] ; m[5]
  sub esi,ecx ; m[3]
  mov [mcu+ebp+5*1*4],edx
  mov [mcu+ebp+3*1*4],esi
  pop esi ; b0
  pop edx ; b1
; m[1]=f5+f6,m[7]=f5-f6
  lea ecx,[eax+ebx] ; m[1]
  sub eax,ebx ; m[7]
  mov [mcu+ebp+1*1*4],ecx
  mov [mcu+ebp+7*1*4],eax
  pop ecx ; b2
  pop ebx ; b3
; c0=b0+b3,c3=b0-b3
  lea edi,[esi+ebx] ; c0
  sub esi,ebx ; c3
; c1=b1+b2,c2=b1-b2
  lea ebx,[edx+ecx] ; c1
  sub edx,ecx ; c2
; d2=c2+c3,e2=d2*a1
  add edx,esi ; d2
  mov eax,a1 ; a1
  imul edx ; e2/2E2
  shld edx,eax,2 ; e2
; m[2]=c3+e2,m[6]=c3-e2
  lea ecx,[esi+edx] ; m[2]
  sub esi,edx ; m[6]
  mov [mcu+ebp+2*1*4],ecx
  mov [mcu+ebp+6*1*4],esi
; m[0]=c0+c1,m[4]=c0-c1
  lea eax,[edi+ebx] ; m[0]
  sub edi,ebx ; m[4]
  mov [mcu+ebp+0*1*4],eax
  mov [mcu+ebp+4*1*4],edi
; bump to next matrix row
  lea ebp,[ebp+8*4] ; offset
  test ebp,0E0h ; eight
  jnz NextRowsPass
; start next matrix
  cmp ebp,400h ; done
  jb NextColsPass
  ret ; return
endp aanfdct

; aanfdct - quantization coefficients - 1st row
q00 = 0134217728 ; 2E26/(cos(4pi/16)*cos(4pi/16))
q01 = 0096765589 ; 2E26/(cos(4pi/16)*cos(1pi/16))
q02 = 0102725802 ; 2E26/(cos(4pi/16)*cos(2pi/16))
q03 = 0114142795 ; 2E26/(cos(4pi/16)*cos(3pi/16))
q04 = 0134217728 ; 2E26/(cos(4pi/16)*cos(4pi/16))
q05 = 0170826765 ; 2E26/(cos(4pi/16)*cos(5pi/16))
q06 = 0248002024 ; 2E26/(cos(4pi/16)*cos(6pi/16))
q07 = 0486473469 ; 2E26/(cos(4pi/16)*cos(7pi/16))
; aanfdct - quantization coefficients - 2nd row
q10 = 0096765589 ; 2E26/(cos(1pi/16)*cos(4pi/16))
q11 = 0069764102 ; 2E26/(cos(1pi/16)*cos(1pi/16))
q12 = 0074061176 ; 2E26/(cos(1pi/16)*cos(2pi/16))
q13 = 0082292369 ; 2E26/(cos(1pi/16)*cos(3pi/16))
q14 = 0096765589 ; 2E26/(cos(1pi/16)*cos(4pi/16))
q15 = 0123159234 ; 2E26/(cos(1pi/16)*cos(5pi/16))
q16 = 0178799495 ; 2E26/(cos(1pi/16)*cos(6pi/16))
q17 = 0350727825 ; 2E26/(cos(1pi/16)*cos(7pi/16))
; aanfdct - quantization coefficients - 3rd row
q20 = 0102725802 ; 2E26/(cos(2pi/16)*cos(4pi/16))
q21 = 0074061176 ; 2E26/(cos(2pi/16)*cos(1pi/16))
q22 = 0078622925 ; 2E26/(cos(2pi/16)*cos(2pi/16))
q23 = 0087361113 ; 2E26/(cos(2pi/16)*cos(3pi/16))
q24 = 0102725802 ; 2E26/(cos(2pi/16)*cos(4pi/16))
q25 = 0130745146 ; 2E26/(cos(2pi/16)*cos(5pi/16))
q26 = 0189812531 ; 2E26/(cos(2pi/16)*cos(6pi/16))
q27 = 0372330673 ; 2E26/(cos(2pi/16)*cos(7pi/16))
; aanfdct - quantization coefficients - 4th row
q30 = 0114142795 ; 2E26/(cos(3pi/16)*cos(4pi/16))
q31 = 0082292369 ; 2E26/(cos(3pi/16)*cos(1pi/16))
q32 = 0087361113 ; 2E26/(cos(3pi/16)*cos(2pi/16))
q33 = 0097070468 ; 2E26/(cos(3pi/16)*cos(3pi/16))
q34 = 0114142795 ; 2E26/(cos(3pi/16)*cos(4pi/16))
q35 = 0145276222 ; 2E26/(cos(3pi/16)*cos(5pi/16))
q36 = 0210908384 ; 2E26/(cos(3pi/16)*cos(6pi/16))
q37 = 0413711678 ; 2E26/(cos(3pi/16)*cos(7pi/16))
; aanfdct - quantization coefficients - 5th row
q40 = 0134217728 ; 2E26/(cos(4pi/16)*cos(4pi/16))
q41 = 0096765589 ; 2E26/(cos(4pi/16)*cos(1pi/16))
q42 = 0102725802 ; 2E26/(cos(4pi/16)*cos(2pi/16))
q43 = 0114142795 ; 2E26/(cos(4pi/16)*cos(3pi/16))
q44 = 0134217728 ; 2E26/(cos(4pi/16)*cos(4pi/16))
q45 = 0170826765 ; 2E26/(cos(4pi/16)*cos(5pi/16))
q46 = 0248002024 ; 2E26/(cos(4pi/16)*cos(6pi/16))
q47 = 0486473469 ; 2E26/(cos(4pi/16)*cos(7pi/16))
; aanfdct - quantization coefficients - 6th row
q50 = 0170826765 ; 2E26/(cos(5pi/16)*cos(4pi/16))
q51 = 0123159234 ; 2E26/(cos(5pi/16)*cos(1pi/16))
q52 = 0130745146 ; 2E26/(cos(5pi/16)*cos(2pi/16))
q53 = 0145276222 ; 2E26/(cos(5pi/16)*cos(3pi/16))
q54 = 0170826765 ; 2E26/(cos(5pi/16)*cos(4pi/16))
q55 = 0217421231 ; 2E26/(cos(5pi/16)*cos(5pi/16))
q56 = 0315646704 ; 2E26/(cos(5pi/16)*cos(6pi/16))
q57 = 0619163281 ; 2E26/(cos(5pi/16)*cos(7pi/16))
; aanfdct - quantization coefficients - 7th row
q60 = 0248002024 ; 2E26/(cos(6pi/16)*cos(4pi/16))
q61 = 0178799495 ; 2E26/(cos(6pi/16)*cos(1pi/16))
q62 = 0189812531 ; 2E26/(cos(6pi/16)*cos(2pi/16))
q63 = 0210908384 ; 2E26/(cos(6pi/16)*cos(3pi/16))
q64 = 0248002024 ; 2E26/(cos(6pi/16)*cos(4pi/16))
q65 = 0315646704 ; 2E26/(cos(6pi/16)*cos(5pi/16))
q66 = 0458247987 ; 2E26/(cos(6pi/16)*cos(6pi/16))
q67 = 0898885762 ; 2E26/(cos(6pi/16)*cos(7pi/16))
; aanfdct - quantization coefficients - 8th row
q70 = 0486473469 ; 2E26/(cos(7pi/16)*cos(4pi/16))
q71 = 0350727825 ; 2E26/(cos(7pi/16)*cos(1pi/16))
q72 = 0372330673 ; 2E26/(cos(7pi/16)*cos(2pi/16))
q73 = 0413711678 ; 2E26/(cos(7pi/16)*cos(3pi/16))
q74 = 0486473469 ; 2E26/(cos(7pi/16)*cos(4pi/16))
q75 = 0619163281 ; 2E26/(cos(7pi/16)*cos(5pi/16))
q76 = 0898885762 ; 2E26/(cos(7pi/16)*cos(6pi/16))
q77 = 1763227847 ; 2E26/(cos(7pi/16)*cos(7pi/16))

dataseg
; luminance - aanfdct - scaled quantization table
yqt dd q00/y00,q01/y01,q02/y02,q03/y03,q04/y04,q05/y05,q06/y06,q07/y07
    dd q10/y10,q11/y11,q12/y12,q13/y13,q14/y14,q15/y15,q16/y16,q17/y17
    dd q20/y20,q21/y21,q22/y22,q23/y23,q24/y24,q25/y25,q26/y26,q27/y27
    dd q30/y30,q31/y31,q32/y32,q33/y33,q34/y34,q35/y35,q36/y36,q37/y37
    dd q40/y40,q41/y41,q42/y42,q43/y43,q44/y44,q45/y45,q46/y46,q47/y47
    dd q50/y50,q51/y51,q52/y52,q53/y53,q54/y54,q55/y55,q56/y56,q57/y57
    dd q60/y60,q61/y61,q62/y62,q63/y63,q64/y64,q65/y65,q66/y66,q67/y67
    dd q70/y70,q71/y71,q72/y72,q73/y73,q74/y74,q75/y75,q76/y76,q77/y77
; luminance - huffman size table - dc coefficients
yds db 02,03,03,03,03,03,04,05,06,07,08,09
; luminance - huffman code table - dc coefficients
ydc dw 0000h,0002h,0003h,0004h,0005h,0006h,000Eh,001Eh,003Eh,007Eh,00FEh,01FEh
; luminance - huffman size table - ac coefficients
yas db 04,00,00,00,00,00,00,00,00,00,00,00,00,00,00,11
    db 02,04,05,06,06,07,07,08,09,09,09,10,10,11,16,16
    db 02,05,08,09,10,11,12,12,15,16,16,16,16,16,16,16
    db 03,07,10,12,16,16,16,16,16,16,16,16,16,16,16,16
    db 04,09,12,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 05,11,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 07,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 08,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 10,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
; luminance - huffman code table - ac coefficients
yac dw 0000Ah,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,007F9h
    dw 00000h,0000Ch,0001Ch,0003Ah,0003Bh,0007Ah,0007Bh,000FAh,001F8h,001F9h,001FAh,003F9h,003FAh,007F8h,0FFEBh,0FFF5h
    dw 00001h,0001Bh,000F9h,001F7h,003F8h,007F7h,00FF6h,00FF7h,07FC0h,0FFBEh,0FFC7h,0FFD0h,0FFD9h,0FFE2h,0FFECh,0FFF6h
    dw 00004h,00079h,003F7h,00FF5h,0FF96h,0FF9Eh,0FFA6h,0FFAEh,0FFB6h,0FFBFh,0FFC8h,0FFD1h,0FFDAh,0FFE3h,0FFEDh,0FFF7h
    dw 0000Bh,001F6h,00FF4h,0FF8Fh,0FF97h,0FF9Fh,0FFA7h,0FFAFh,0FFB7h,0FFC0h,0FFC9h,0FFD2h,0FFDBh,0FFE4h,0FFEEh,0FFF8h
    dw 0001Ah,007F6h,0FF89h,0FF90h,0FF98h,0FFA0h,0FFA8h,0FFB0h,0FFB8h,0FFC1h,0FFCAh,0FFD3h,0FFDCh,0FFE5h,0FFEFh,0FFF9h
    dw 00078h,0FF84h,0FF8Ah,0FF91h,0FF99h,0FFA1h,0FFA9h,0FFB1h,0FFB9h,0FFC2h,0FFCBh,0FFD4h,0FFDDh,0FFE6h,0FFF0h,0FFFAh
    dw 000F8h,0FF85h,0FF8Bh,0FF92h,0FF9Ah,0FFA2h,0FFAAh,0FFB2h,0FFBAh,0FFC3h,0FFCCh,0FFD5h,0FFDEh,0FFE7h,0FFF1h,0FFFBh
    dw 003F6h,0FF86h,0FF8Ch,0FF93h,0FF9Bh,0FFA3h,0FFABh,0FFB3h,0FFBBh,0FFC4h,0FFCDh,0FFD6h,0FFDFh,0FFE8h,0FFF2h,0FFFCh
    dw 0FF82h,0FF87h,0FF8Dh,0FF94h,0FF9Ch,0FFA4h,0FFACh,0FFB4h,0FFBCh,0FFC5h,0FFCEh,0FFD7h,0FFE0h,0FFE9h,0FFF3h,0FFFDh
    dw 0FF83h,0FF88h,0FF8Eh,0FF95h,0FF9Dh,0FFA5h,0FFADh,0FFB5h,0FFBDh,0FFC6h,0FFCFh,0FFD8h,0FFE1h,0FFEAh,0FFF4h,0FFFEh

dataseg
; chrominance - aanfdct - scaled quantization table
cqt dd q00/c00,q01/c01,q02/c02,q03/c03,q04/c04,q05/c05,q06/c06,q07/c07
    dd q10/c10,q11/c11,q12/c12,q13/c13,q14/c14,q15/c15,q16/c16,q17/c17
    dd q20/c20,q21/c21,q22/c22,q23/c23,q24/c24,q25/c25,q26/c26,q27/c27
    dd q30/c30,q31/c31,q32/c32,q33/c33,q34/c34,q35/c35,q36/c36,q37/c37
    dd q40/c40,q41/c41,q42/c42,q43/c43,q44/c44,q45/c45,q46/c46,q47/c47
    dd q50/c50,q51/c51,q52/c52,q53/c53,q54/c54,q55/c55,q56/c56,q57/c57
    dd q60/c60,q61/c61,q62/c62,q63/c63,q64/c64,q65/c65,q66/c66,q67/c67
    dd q70/c70,q71/c71,q72/c72,q73/c73,q74/c74,q75/c75,q76/c76,q77/c77
; chrominance - huffman size table - dc coefficients
cds db 02,02,02,03,04,05,06,07,08,09,10,11
; chrominance - huffman code table - dc coefficients
cdc dw 0000h,0001h,0002h,0006h,000Eh,001Eh,003Eh,007Eh,00FEh,01FEh,03FEh,07FEh
; chrominance - huffman size table - ac coefficients
cas db 02,00,00,00,00,00,00,00,00,00,00,00,00,00,00,10
    db 02,04,05,05,06,06,07,07,08,09,09,09,09,11,14,15
    db 03,06,08,08,09,10,11,11,16,16,16,16,16,16,16,16
    db 04,08,10,10,16,16,16,16,16,16,16,16,16,16,16,16
    db 05,09,12,12,16,16,16,16,16,16,16,16,16,16,16,16
    db 05,11,15,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 06,12,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 07,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 09,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 10,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
    db 12,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
; chrominance - huffman code table - ac coefficients
cac dw 00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,00000h,003FAh
    dw 00001h,0000Bh,0001Ah,0001Bh,0003Ah,0003Bh,00079h,0007Ah,000F9h,001F7h,001F8h,001F9h,001FAh,007F9h,03FE0h,07FC3h
    dw 00004h,00039h,000F7h,000F8h,001F6h,003F9h,007F7h,007F8h,0FFB7h,0FFC0h,0FFC9h,0FFD2h,0FFDBh,0FFE4h,0FFEDh,0FFF6h
    dw 0000Ah,000F6h,003F7h,003F8h,0FF97h,0FF9Fh,0FFA7h,0FFAFh,0FFB8h,0FFC1h,0FFCAh,0FFD3h,0FFDCh,0FFE5h,0FFEEh,0FFF7h
    dw 00018h,001F5h,00FF6h,00FF7h,0FF98h,0FFA0h,0FFA8h,0FFB0h,0FFB9h,0FFC2h,0FFCBh,0FFD4h,0FFDDh,0FFE6h,0FFEFh,0FFF8h
    dw 00019h,007F6h,07FC2h,0FF91h,0FF99h,0FFA1h,0FFA9h,0FFB1h,0FFBAh,0FFC3h,0FFCCh,0FFD5h,0FFDEh,0FFE7h,0FFF0h,0FFF9h
    dw 00038h,00FF5h,0FF8Ch,0FF92h,0FF9Ah,0FFA2h,0FFAAh,0FFB2h,0FFBBh,0FFC4h,0FFCDh,0FFD6h,0FFDFh,0FFE8h,0FFF1h,0FFFAh
    dw 00078h,0FF88h,0FF8Dh,0FF93h,0FF9Bh,0FFA3h,0FFABh,0FFB3h,0FFBCh,0FFC5h,0FFCEh,0FFD7h,0FFE0h,0FFE9h,0FFF2h,0FFFBh
    dw 001F4h,0FF89h,0FF8Eh,0FF94h,0FF9Ch,0FFA4h,0FFACh,0FFB4h,0FFBDh,0FFC6h,0FFCFh,0FFD8h,0FFE1h,0FFEAh,0FFF3h,0FFFCh
    dw 003F6h,0FF8Ah,0FF8Fh,0FF95h,0FF9Dh,0FFA5h,0FFADh,0FFB5h,0FFBEh,0FFC7h,0FFD0h,0FFD9h,0FFE2h,0FFEBh,0FFF4h,0FFFDh
    dw 00FF4h,0FF8Bh,0FF90h,0FF96h,0FF9Eh,0FFA6h,0FFAEh,0FFB6h,0FFBFh,0FFC8h,0FFD1h,0FFDAh,0FFE3h,0FFECh,0FFF5h,0FFFEh

udataseg
hcInBits dd ?
hcOffset dd ?
hcStatus dd ?

dataseg
; zig-zag ordering
zzo db 00,01,08,16,09,02,03,10
    db 17,24,32,25,18,11,04,05
    db 12,19,26,33,40,48,41,34
    db 27,20,13,06,07,14,21,28
    db 35,42,49,56,57,50,43,36
    db 29,22,15,23,30,37,44,51
    db 58,59,52,45,38,31,39,46
    db 53,60,61,54,47,55,62,63

udataseg
dcValue dd 3 dup(?)

codeseg
proc encoder near
; fetch huffman state
  mov edi,[hcOffset]
  mov ecx,[hcStatus]
; point to first matrix
  sub ebp,ebp ; offset
; setup luminance tables
  sub esi,esi ; offset
label NextDCelement near
; dc component quantization
  mov edx,[mcu+ebp*4] ; value
; apply quantization factor
  mov eax,[yqt+esi] ; first
  imul edx ; DC*2E18*QF*2E26
  sar edx,15 ; keep 12 bits
  adc edx,0 ; round result
; fetch previous dc value
  mov ebx,ebp ; matrix index
  shr ebx,6 ; derive index
  sub ebx,1 ; adjust index
  adc ebx,0 ; proper index
  mov eax,[dcValue+ebx*4]
; store current dc value
  mov [dcValue+ebx*4],edx
; huffman code generation
  sub edx,eax ; new-old
  jnz dcNonZeroValue
; handle dc zero value
  sub ebx,ebx ; no data
  jmp dcHuffman ; code
label dcNonZeroValue near
; determine dc value size
  mov eax,edx ; dc value
  jns dcScanBits ; positive
  neg eax ; make positive
  dec edx ; proper data
label dcScanBits near
; bit scan left to right
  bsr bx,ax ; bit index
  inc ebx ; bit length
label dcHuffman near
; get huffman state
  mov al,cl ; string
; obtain huffman size
  mov cl,[yds+esi+ebx*1]
  shl eax,cl ; make room
; append huffman code
  xor ax,[ydc+esi+ebx*2]
  add cl,ch ; bit length
; try to emit next byte
  cmp cl,8 ; bits per byte
  jb dcNotEmitCode ; too few
  sub cl,8 ; bits per byte
  ror eax,cl ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz dcEndZeroCode1 ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label dcEndZeroCode1 near
; try to emit next byte
  cmp cl,8 ; bits per byte
  jb dcEndEmitCode ; too few
  sub cl,8 ; bits per byte
  rol eax,8 ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz dcEndZeroCode2 ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label dcEndZeroCode2 near
; cannot emit another byte
label dcEndEmitCode near
; realign on bit boundary
  rol eax,cl ; position
label dcNotEmitCode near
; append huffman data
  test ebx,ebx ; present
  jz dcNotEmitData ; no
  mov ch,cl ; bit length
  mov cl,bl ; bit length
  ror edx,cl ; alignment
  shld eax,edx,cl ; append
  add cl,ch ; bit length
; try to emit next byte
  cmp cl,8 ; bits per byte
  jb dcNotEmitData ; too few
  sub cl,8 ; bits per byte
  ror eax,cl ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz dcEndZeroData1 ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label dcEndZeroData1 near
; try to emit next byte
  cmp cl,8 ; bits per byte
  jb dcEndEmitData ; too few
  sub cl,8 ; bits per byte
  rol eax,8 ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz dcEndZeroData2 ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label dcEndZeroData2 near
; cannot emit another byte
label dcEndEmitData near
; realign on bit boundary
  rol eax,cl ; position
label dcNotEmitData near
; update huffman state
  mov ch,cl ; length
  mov cl,al ; string
; bump to next element
  lea ebp,[ebp+1] ; offset
label NextACelement near
; prepare zigzag sequence
  mov ebx,ebp ; matrix index
  and bl,03Fh ; zigzag index
  mov bl,[zzo+ebx] ; sequence
; ac component quantization
  mov edx,ebp ; matrix index
  and dl,0C0h ; isolate matrix
  xor dl,bl ; zigzag sequence
  mov edx,[mcu+edx*4] ; value
; apply quantization factor
  mov eax,[yqt+esi+ebx*4]
  imul edx ; AC*2E18*QF*2E26
  sar edx,15 ; keep 12 bits
  adc edx,0 ; round result
; huffman code generation
  test edx,edx ; value
  jnz acNonZeroValue
; update zero counter
  add ecx,04000000h
  cmp bl,03Fh ; done
  jne EndEncode ; next
; setup EOB huffman code
  sub ebx,ebx ; no data
  and ecx,0FFFFh ; reset
  jmp acHuffman ; code
label acNonZeroValue near
; setup ZRL huffman code
  mov bl,0Fh ; special
label VerifyCounter near
; verify zero counter
  cmp ecx,40000000h
  jb ValidZeroCount
; update zero counter
  sub ecx,40000000h
  jmp acHuffman ; code
label ValidZeroCount near
; determine ac value size
  test edx,edx ; value
  mov eax,edx ; ac value
  jns acScanBits ; positive
  neg eax ; make positive
  dec edx ; proper data
label acScanBits near
; bit scan left to right
  bsr bx,ax ; bit index
  inc ebx ; bit length
; concatenate zero counter
  lea eax,[ecx*4] ; count
  and ecx,0FFFFh ; reset
  shld ebx,eax,4 ; index
label acHuffman near
; get huffman state
  mov al,cl ; string
; obtain huffman size
  mov cl,[yas+esi+ebx*1]
  shl eax,cl ; make room
; append huffman code
  xor ax,[yac+esi+ebx*2]
  add cl,ch ; bit length
; try to emit next byte
  cmp cl,8 ; bits per byte
  jb acNotEmitCode ; too few
  sub cl,8 ; bits per byte
  ror eax,cl ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz acEndZeroCode1 ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label acEndZeroCode1 near
; try to emit next byte
  cmp cl,8 ; bits per byte
  jb acEndEmitCode ; too few
  sub cl,8 ; bits per byte
  rol eax,8 ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz acEndZeroCode2 ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label acEndZeroCode2 near
; cannot emit another byte
label acEndEmitCode near
; realign on bit boundary
  rol eax,cl ; position
label acNotEmitCode near
; update huffman state
  mov ch,cl ; length
  mov cl,al ; string
; handle ZRL hufman code
  cmp bl,0Fh ; special
  je VerifyCounter
; append huffman data
  shr ebx,04h ; present
  jz EndEncode ; absent
  mov cl,bl ; bit length
  ror edx,cl ; alignment
  shld eax,edx,cl ; append
  add cl,ch ; bit length
; try to emit next byte
  cmp cl,8 ; bits per byte
  jb acNotEmitData ; too few
  sub cl,8 ; bits per byte
  ror eax,cl ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz acEndZeroData1 ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label acEndZeroData1 near
; try to emit next byte
  cmp cl,8 ; bits per byte
  jb acEndEmitData ; too few
  sub cl,8 ; bits per byte
  rol eax,8 ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz acEndZeroData2 ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label acEndZeroData2 near
; cannot emit another byte
label acEndEmitData near
; realign on bit boundary
  rol eax,cl ; position
label acNotEmitData near
; update huffman state
  mov ch,cl ; length
  mov cl,al ; string
label EndEncode near
; bump to next element
  lea ebp,[ebp+1] ; offset
  test ebp,03Fh ; more
  jnz NextACelement
; reset zero counter
  and ecx,0000FFFFh
; start next luminance
  cmp ebp,080h ; done
  jb NextDCelement
; start next chrominance
  mov esi,cqt-yqt ; offset
  cmp ebp,100h ; done
  jb NextDCelement
; store huffman state
  mov [hcOffset],edi
  mov [hcStatus],ecx
  ret ; return
endp encoder

dataseg
; mcu 4:2:2 ordering
yyo db 00,01,02,03,32,33,34,35
    db 04,05,06,07,36,37,38,39
    db 08,09,10,11,40,41,42,43
    db 12,13,14,15,44,45,46,47
    db 16,17,18,19,48,49,50,51
    db 20,21,22,23,52,53,54,55
    db 24,25,26,27,56,57,58,59
    db 28,29,30,31,60,61,62,63

; precalculated constant
c128 = 33554432 ; 128*2E18

; precalculated rgb2ycc coefficients
cbm = 1212555137 ; 0.564639986*2E31
crm = 1531532078 ; 0.713175199*2E31
ybc = 0245843928 ; 0.11448*2E31
ygc = 1259735383 ; 0.58661*2E31
yrc = 0641904337 ; 0.29891*2E31

udataseg
hdrsize = HdrEOI-HdrSOI
outHeader db hdrsize dup(?)
outBuffer db Ix*Iy*1 dup(?)

codeseg
proc rgb2jpg near
; clear old dc values
  sub eax,eax ; zero
  mov [dcValue+0],eax
  mov [dcValue+4],eax
  mov [dcValue+8],eax
; clear huffman state
  mov [hcOffset],eax
  mov [hcStatus],eax
; scan image to build minimum coded units
  mov ebp,offset(rgbBuffer)+3*Ix*(Iy-1)
  push esi ; save
label ScanImage near
; set horizontal units
  mov cl,Ix/16 ; 4:2:2
label BuildNextUnit near
; build minimum coded unit
  sub edi,edi ; mcu index
label NextUnitRow near
  mov ch,8 ; columns
label rgb2ycc near
; rgb2y0 conversion
  sub edx,edx ; clear
  mov dl,[ebp+0] ; B0
  mov eax,ybc ; ybf*2E31
  shl edx,18 ; B0*2E18
  mul edx ; ybf*B0*2E18
  mov ebx,edx ; apply
  sub edx,edx ; clear
  mov dl,[ebp+1] ; G0
  mov eax,ygc ; ygf*2E31
  shl edx,18 ; G0*2E18
  mul edx ; ygf*G0*2E18
  add ebx,edx ; apply
  sub edx,edx ; clear
  mov dl,[ebp+2] ; R0
  mov eax,yrc ; yrf*2E31
  shl edx,18 ; R0*2E18
  mul edx ; yrf*R0*2E18
  add ebx,edx ; apply
; rgb2y0 midpoint shift
  lea eax,[ebx-c128/2]
  movzx edx,[yyo+edi]
  mov [mcu+edx*8+0],eax
; rgb2y1 conversion
  sub edx,edx ; clear
  mov dl,[ebp+3] ; B1
  mov eax,ybc ; ybf*2E31
  shl edx,18 ; B1*2E18
  mul edx ; ybf*B1*2E18
  mov esi,edx ; apply
  sub edx,edx ; clear
  mov dl,[ebp+4] ; G1
  mov eax,ygc ; ygf*2E31
  shl edx,18 ; G1*2E18
  mul edx ; ygf*G1*2E18
  add esi,edx ; apply
  sub edx,edx ; clear
  mov dl,[ebp+5] ; R1
  mov eax,yrc ; yrf*2E31
  shl edx,18 ; R1*2E18
  mul edx ; yrf*R1*2E18
  add esi,edx ; apply
; rgb2y1 midpoint shift
  lea eax,[esi-c128/2]
  movzx edx,[yyo+edi]
  mov [mcu+edx*8+4],eax
; total Y0+Y1 values
  add ebx,esi ; Y*2E19
; total B0+B1 values
  sub eax,eax ; clear
  sub edx,edx ; clear
  mov al,[ebp+0] ; B0
  mov dl,[ebp+3] ; B1
  add edx,eax ; B*2E1
  shl edx,17 ; B*2E18
; yr2cb conversion
  mov eax,cbm ; cbf*2E31
  sub edx,ebx ; (B-Y)*2E19
  imul edx ; cbf*(B-Y)*2E18
  mov [mcu+512+edi*4],edx
; total R0+R1 values
  sub eax,eax ; clear
  sub edx,edx ; clear
  mov al,[ebp+2] ; R0
  mov dl,[ebp+5] ; R1
  add edx,eax ; R*2E1
  shl edx,17 ; R*2E18
; yb2cr conversion
  mov eax,crm ; crf*2E31
  sub edx,ebx ; (R-Y)*2E19
  imul edx ; crf*(R-Y)*2E18
  mov [mcu+768+edi*4],edx
; bump to next column
  lea ebp,[ebp+6]
  lea edi,[edi+1]
  dec ch ; columns
  jnz rgb2ycc ; more
; bump to next row
  lea ebp,[ebp-16*3-1*3*Ix]
  cmp edi,64 ; complete
  jb NextUnitRow ; more
; put minimum coded unit
  push ebp ecx ; save
  call aanfdct ; forward
  call encoder ; huffman
  pop ecx ebp ; restore
; bump pointer horizontally
  lea ebp,[ebp+16*3+8*3*Ix]
  dec cl ; horizontal units
  jnz BuildNextUnit ; more
; bump pointer vertically
  lea ebp,[ebp-(8+1)*3*Ix]
; verify image complete
  cmp ebp,offset(rgbBuffer)
  ja ScanImage ; more
  call header ; mpjg
  pop esi ; restore
  call writer
  ret ; return
endp rgb2jpg

dataseg
; start of image
HdrSOI db 0FFh,0D8h
; application segment
HdrAPP db 0FFh,0E0h,0,16,'J','F','I','F',0,1,1,0,0,1,0,1,0,0
; comment
HdrCOM db 0FFh,0FEh,0,15,"Wim's WebLook"
; define quantization tables - zigzag order
HdrDQTy db 0FFh,0DBh,0,67,00h ; luminance
        db y00,y01,y10,y20,y11,y02,y03,y12
        db y21,y30,y40,y31,y22,y13,y04,y05
        db y14,y23,y32,y41,y50,y60,y51,y42
        db y33,y24,y15,y06,y07,y16,y25,y34
        db y43,y52,y61,y70,y71,y62,y53,y44
        db y35,y26,y17,y27,y36,y45,y54,y63
        db y72,y73,y64,y55,y46,y37,y47,y56
        db y65,y74,y75,y66,y57,y67,y76,y77
HdrDQTc db 0FFh,0DBh,0,67,01h ; chrominance
        db c00,c01,c10,c20,c11,c02,c03,c12
        db c21,c30,c40,c31,c22,c13,c04,c05
        db c14,c23,c32,c41,c50,c60,c51,c42
        db c33,c24,c15,c06,c07,c16,c25,c34
        db c43,c52,c61,c70,c71,c62,c53,c44
        db c35,c26,c17,c27,c36,c45,c54,c63
        db c72,c73,c64,c55,c46,c37,c47,c56
        db c65,c74,c75,c66,c57,c67,c76,c77
; start of frame - baseline dct - 3 components - 4:2:2 chroma subsampling
HdrSOF db 0FFh,0C0h,0,17,8,high(Iy),low(Iy),high(Ix),low(Ix),3,1,21h,0,2,11h,1,3,11h,1
; define huffman tables
HdrDHTyd db 0FFh,0C4h,0,031,00h ; luminance dc table
       db 00h,01h,05h,01h,01h,01h,01h,01h,01h,00h,00h,00h,00h,00h,00h,00h
       db 00h,01h,02h,03h,04h,05h,06h,07h,08h,09h,0Ah,0Bh
HdrDHTya db 0FFh,0C4h,0,181,10h ; luminance ac table
       db 00h,02h,01h,03h,03h,02h,04h,03h,05h,05h,04h,04h,00h,00h,01h,7Dh
       db 001h,002h,003h,000h,004h,011h,005h,012h,021h,031h,041h,006h,013h,051h,061h,007h
       db 022h,071h,014h,032h,081h,091h,0A1h,008h,023h,042h,0B1h,0C1h,015h,052h,0D1h,0F0h
       db 024h,033h,062h,072h,082h,009h,00Ah,016h,017h,018h,019h,01Ah,025h,026h,027h,028h
       db 029h,02Ah,034h,035h,036h,037h,038h,039h,03Ah,043h,044h,045h,046h,047h,048h,049h
       db 04Ah,053h,054h,055h,056h,057h,058h,059h,05Ah,063h,064h,065h,066h,067h,068h,069h
       db 06Ah,073h,074h,075h,076h,077h,078h,079h,07Ah,083h,084h,085h,086h,087h,088h,089h
       db 08Ah,092h,093h,094h,095h,096h,097h,098h,099h,09Ah,0A2h,0A3h,0A4h,0A5h,0A6h,0A7h
       db 0A8h,0A9h,0AAh,0B2h,0B3h,0B4h,0B5h,0B6h,0B7h,0B8h,0B9h,0BAh,0C2h,0C3h,0C4h,0C5h
       db 0C6h,0C7h,0C8h,0C9h,0CAh,0D2h,0D3h,0D4h,0D5h,0D6h,0D7h,0D8h,0D9h,0DAh,0E1h,0E2h
       db 0E3h,0E4h,0E5h,0E6h,0E7h,0E8h,0E9h,0EAh,0F1h,0F2h,0F3h,0F4h,0F5h,0F6h,0F7h,0F8h
       db 0F9h,0FAh
HdrDHTcd db 0FFh,0C4h,0,031,01h ; chrominance dc table
       db 00h,03h,01h,01h,01h,01h,01h,01h,01h,01h,01h,00h,00h,00h,00h,00h
       db 00h,01h,02h,03h,04h,05h,06h,07h,08h,09h,0Ah,0Bh
HdrDHTca db 0FFh,0C4h,0,181,11h ; chrominance ac table
       db 00h,02h,01h,02h,04h,04h,03h,04h,07h,05h,04h,04h,00h,01h,02h,77h
       db 000h,001h,002h,003h,011h,004h,005h,021h,031h,006h,012h,041h,051h,007h,061h,071h
       db 013h,022h,032h,081h,008h,014h,042h,091h,0A1h,0B1h,0C1h,009h,023h,033h,052h,0F0h
       db 015h,062h,072h,0D1h,00Ah,016h,024h,034h,0E1h,025h,0F1h,017h,018h,019h,01Ah,026h
       db 027h,028h,029h,02Ah,035h,036h,037h,038h,039h,03Ah,043h,044h,045h,046h,047h,048h
       db 049h,04Ah,053h,054h,055h,056h,057h,058h,059h,05Ah,063h,064h,065h,066h,067h,068h
       db 069h,06Ah,073h,074h,075h,076h,077h,078h,079h,07Ah,082h,083h,084h,085h,086h,087h
       db 088h,089h,08Ah,092h,093h,094h,095h,096h,097h,098h,099h,09Ah,0A2h,0A3h,0A4h,0A5h
       db 0A6h,0A7h,0A8h,0A9h,0AAh,0B2h,0B3h,0B4h,0B5h,0B6h,0B7h,0B8h,0B9h,0BAh,0C2h,0C3h
       db 0C4h,0C5h,0C6h,0C7h,0C8h,0C9h,0CAh,0D2h,0D3h,0D4h,0D5h,0D6h,0D7h,0D8h,0D9h,0DAh
       db 0E2h,0E3h,0E4h,0E5h,0E6h,0E7h,0E8h,0E9h,0EAh,0F2h,0F3h,0F4h,0F5h,0F6h,0F7h,0F8h
       db 0F9h,0FAh
; start of scan - 3 components
HdrSOS db 0FFh,0DAh,0,12,3,1,00h,2,11h,3,11h,0,63,0
; end of image
HdrEOI db 0FFh,0D9h

codeseg
proc header near
; copy mjpg header
  mov ecx,hdrsize
  mov edi,offset(outHeader)
  mov esi,offset(hdrSOI)
  rep movsb ; store
  ret ; return
endp header

codeseg
proc writer near
; obtain huffman state
  mov ecx,[hcStatus]
  mov edi,[hcOffset]
; emit remaining bits
  test ch,ch ; incomplete
  jz AppendTrailer ; no
  mov al,0FFh ; filler
  mov ah,cl ; string
  mov cl,ch ; length
  shr eax,cl ; alignment
  mov [outBuffer+edi],al
  inc edi ; next position
; cater for zero stuffing
  xor al,0FFh ; ones/zeros
  jnz AppendTrailer ; no
  mov [outBuffer+edi],al
  inc edi ; next position
label AppendTrailer near
; append end of image header
  mov ax,[word(HdrEOI)]
  mov [outBuffer+edi],al
  inc edi ; next position
  mov [outBuffer+edi],ah
  inc edi ; next position
; address mjpg output buffer
  mov esi,offset(outHeader)
; lea edi,[edi+esi+hdrsize]
  add edi,hdrsize ; header
  add edi,esi ; end
  ret ; return
endp writer
