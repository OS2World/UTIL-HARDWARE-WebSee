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

; precalculated aanidct coefficients
s1 = cos4a ; cos(4pi/16)*2E30
s2 = cos2a+cos6a ; (cos(2pi/16)+cos(6pi/16))*2E30
s3 = cos4a ; cos(4pi/16)*2E30
s4 = cos2a-cos6a ; (cos(2pi/16)-cos(6pi/16))*2E30
s5 = cos2a ; cos(2pi/16)*2E30

; aanidct algorithm (29 additions and 5 multiplications)
; Y.Arai, T.Agui, M.Nakajima. "A Fast DCT-SQ Scheme for Images".
; Transactions of IEICE, vol. E71, nø. 11, 1988, pp. 1095-1097.
;    step1    ,  step2 ,  step3 ,  step4 ,  step5 ,  step6
; b0=m[0]+m[4],        ,d2=b2*s1,e2=d2-b6,f0=b0+b6,m[0]=f0+c3
; b4=m[0]-m[4],        ,        ,        ,f6=b0-b6,m[7]=f0-c3
; b6=m[2]+m[6],        ,        ,        ,f4=b4+e2,m[1]=f4+f7
; b2=m[2]-m[6],        ,        ,        ,f2=b4-e2,m[6]=f4-f7
; b3=m[5]+m[3],c3=b1+b3,d5=b5*s2,e5=d8-d7,f7=e7-c3,m[2]=f2+f1
; b5=m[5]-m[3],c1=b1-b3,d1=c1*s3,e7=d8-d5,f1=d1-f7,m[5]=f2-f1
; b1=m[1]+m[7],c8=b5+b7,d7=b7*s4,        ,f5=d5-f1,m[3]=f6+f5
; b7=m[1]-m[7],        ,d8=c8*s5,        ,        ,m[4]=f6-f5
;    step1    ,  step2 ,  step3 ,  step4 ,  step5 ,  step6
; derived from Stefan Kuhr, thesis2side.pdf, figure 3.11

codeseg
proc aanidct near
; point to first matrix
  sub ebp,ebp ; offset
label NextPassCols near
; b0=m[0]+m[4],b4=m[0]-m[4]
  mov ebx,[mcu+ebp+0*8*4]
  mov eax,[mcu+ebp+4*8*4]
  lea edi,[ebx+eax] ; b0
  sub ebx,eax ; b4
; b6=m[2]+m[6],b2=m[2]-m[6]
  mov ecx,[mcu+ebp+2*8*4]
  mov eax,[mcu+ebp+6*8*4]
  lea esi,[ecx+eax] ; b6
  sub ecx,eax ; b2
; d2=b2*s1;e2=d2-b6
  mov eax,s1 ; s1
  imul ecx ; d2/2E4
  shld edx,eax,3 ; d2
  sub edx,esi ; e2
; f0=b0+b6,f6=b0-b6
  lea ecx,[edi+esi] ; f0
  sub edi,esi ; f6
; f4=b4+e2,f2=b4-e2
  lea eax,[ebx+edx] ; f4
  sub ebx,edx ; f2
  push edi ; f6
  push ebx ; f2
  push eax ; f4
  push ecx ; f0
; b3=m[5]+m[3],b5=m[5]-m[3]
  mov ebx,[mcu+ebp+5*8*4]
  mov eax,[mcu+ebp+3*8*4]
  lea edi,[ebx+eax] ; b3
  sub ebx,eax ; b5
; b1=m[1]+m[7],b7=m[1]-m[7]
  mov ecx,[mcu+ebp+1*8*4]
  mov eax,[mcu+ebp+7*8*4]
  lea esi,[ecx+eax] ; b1
  sub ecx,eax ; b7
; c3=b1+b3,c1=b1-b3
  lea eax,[esi+edi] ; c3
  sub esi,edi ; c1
  push eax ; c3
; c8=b5+b7,d8=c8*s5
  lea edx,[ebx+ecx] ; c8
  mov eax,s5 ; s5
  imul edx ; d8/2E4
  shld edx,eax,3 ; d8
  mov edi,edx ; d8
; d7=b7*s4,e5=d8-d7
  mov eax,s4 ; s4
  imul ecx ; d7/2E4
  shld edx,eax,3 ; d7
  mov ecx,edi ; d8
  sub edi,edx ; e5
; d5=b5*s2,e7=d8-d5
  mov eax,s2 ; s2
  imul ebx ; d5/2E4
  shld edx,eax,3 ; d5
  sub ecx,edx ; e7
; d1=c1*s3,f7=e7-c3
  mov eax,s3 ; s3
  imul esi ; c1/2E4
  shld edx,eax,3 ; d1
  pop ebx ; c3
  sub ecx,ebx ; f7
; f1=d1-f7,f5=e5-f1
  sub edx,ecx ; f1
  sub edi,edx ; f5
; m[0]=f0+c3,m[7]=f0-c3
  pop esi ; f0
  lea eax,[esi+ebx] ; m[0]
  sub esi,ebx ; m[7]
  mov [mcu+ebp+0*8*4],eax
  mov [mcu+ebp+7*8*4],esi
; m[1]=f4+f7,m[6]=f4-f7
  pop esi ; f4
  lea eax,[esi+ecx] ; m[1]
  sub esi,ecx ; m[7]
  mov [mcu+ebp+1*8*4],eax
  mov [mcu+ebp+6*8*4],esi
; m[2]=f2+f1,m[5]=f2-f1
  pop esi ; f2
  lea eax,[esi+edx] ; m[2]
  sub esi,edx ; m[5]
  mov [mcu+ebp+2*8*4],eax
  mov [mcu+ebp+5*8*4],esi
; m[3]=f6+f5,m[4]=f6-f5
  pop esi ; f6
  lea eax,[esi+edi] ; m[3]
  sub esi,edi ; m[4]
  mov [mcu+ebp+3*8*4],eax
  mov [mcu+ebp+4*8*4],esi
; bump to next matrix column
  lea ebp,[ebp+1*4] ; offset
  test ebp,01Ch ; eight
  jnz NextPassCols
; point to start of matrix
  lea ebp,[ebp-8*4] ; offset
label NextPassRows near
; b0=m[0]+m[4],b4=m[0]-m[4]
  mov ebx,[mcu+ebp+0*1*4]
  mov eax,[mcu+ebp+4*1*4]
  lea edi,[ebx+eax] ; b0
  sub ebx,eax ; b4
; b6=m[2]+m[6],b2=m[2]-m[6]
  mov ecx,[mcu+ebp+2*1*4]
  mov eax,[mcu+ebp+6*1*4]
  lea esi,[ecx+eax] ; b6
  sub ecx,eax ; b2
; d2=b2*s1;e2=d2-b6
  mov eax,s1 ; s1
  imul ecx ; d2/2E4
  shld edx,eax,3 ; d2
  sub edx,esi ; e2
; f0=b0+b6,f6=b0-b6
  lea ecx,[edi+esi] ; f0
  sub edi,esi ; f6
; f4=b4+e2,f2=b4-e2
  lea eax,[ebx+edx] ; f4
  sub ebx,edx ; f2
  push edi ; f6
  push ebx ; f2
  push eax ; f4
  push ecx ; f0
; b3=m[5]+m[3],b5=m[5]-m[3]
  mov ebx,[mcu+ebp+5*1*4]
  mov eax,[mcu+ebp+3*1*4]
  lea edi,[ebx+eax] ; b3
  sub ebx,eax ; b5
; b1=m[1]+m[7],b7=m[1]-m[7]
  mov ecx,[mcu+ebp+1*1*4]
  mov eax,[mcu+ebp+7*1*4]
  lea esi,[ecx+eax] ; b1
  sub ecx,eax ; b7
; c3=b1+b3,c1=b1-b3
  lea eax,[esi+edi] ; c3
  sub esi,edi ; c1
  push eax ; c3
; c8=b5+b7,d8=c8*s5
  lea edx,[ebx+ecx] ; c8
  mov eax,s5 ; s5
  imul edx ; d8/2E4
  shld edx,eax,3 ; d8
  mov edi,edx ; d8
; d7=b7*s4,e5=d8-d7
  mov eax,s4 ; s4
  imul ecx ; d7/2E4
  shld edx,eax,3 ; d7
  mov ecx,edi ; d8
  sub edi,edx ; e5
; d5=b5*s2,e7=d8-d5
  mov eax,s2 ; s2
  imul ebx ; d5/2E4
  shld edx,eax,3 ; d5
  sub ecx,edx ; e7
; d1=c1*s3,f7=e7-c3
  mov eax,s3 ; s3
  imul esi ; c1/2E4
  shld edx,eax,3 ; d1
  pop ebx ; c3
  sub ecx,ebx ; f7
; f1=d1-f7,f5=e5-f1
  sub edx,ecx ; f1
  sub edi,edx ; f5
; m[0]=f0+c3,m[7]=f0-c3
  pop esi ; f0
  lea eax,[esi+ebx] ; m[0]
  sub esi,ebx ; m[7]
  mov [mcu+ebp+0*1*4],eax
  mov [mcu+ebp+7*1*4],esi
; m[1]=f4+f7,m[6]=f4-f7
  pop esi ; f4
  lea eax,[esi+ecx] ; m[1]
  sub esi,ecx ; m[7]
  mov [mcu+ebp+1*1*4],eax
  mov [mcu+ebp+6*1*4],esi
; m[2]=f2+f1,m[5]=f2-f1
  pop esi ; f2
  lea eax,[esi+edx] ; m[2]
  sub esi,edx ; m[5]
  mov [mcu+ebp+2*1*4],eax
  mov [mcu+ebp+5*1*4],esi
; m[3]=f6+f5,m[4]=f6-f5
  pop esi ; f6
  lea eax,[esi+edi] ; m[3]
  sub esi,edi ; m[4]
  mov [mcu+ebp+3*1*4],eax
  mov [mcu+ebp+4*1*4],esi
; bump to next matrix row
  lea ebp,[ebp+8*4] ; offset
  test ebp,0E0h ; eight
  jnz NextPassRows
; start next matrix
  cmp ebp,400h ; done
  jb NextPassCols
  ret ; return
endp aanidct

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

; aanidct - quantization coefficients - 1st row
d00 = 4194304 ; 2E23*(cos(4pi/16)*cos(4pi/16))
d01 = 5817667 ; 2E23*(cos(4pi/16)*cos(1pi/16))
d02 = 5480122 ; 2E23*(cos(4pi/16)*cos(2pi/16))
d03 = 4931980 ; 2E23*(cos(4pi/16)*cos(3pi/16))
d04 = 4194304 ; 2E23*(cos(4pi/16)*cos(4pi/16))
d05 = 3295444 ; 2E23*(cos(4pi/16)*cos(5pi/16))
d06 = 2269941 ; 2E23*(cos(4pi/16)*cos(6pi/16))
d07 = 1157206 ; 2E23*(cos(4pi/16)*cos(7pi/16))
; aanidct - quantization coefficients - 2nd row
d10 = 5817667 ; 2E23*(cos(1pi/16)*cos(4pi/16))
d11 = 8069336 ; 2E23*(cos(1pi/16)*cos(1pi/16))
d12 = 7601148 ; 2E23*(cos(1pi/16)*cos(2pi/16))
d13 = 6840852 ; 2E23*(cos(1pi/16)*cos(3pi/16))
d14 = 5817667 ; 2E23*(cos(1pi/16)*cos(4pi/16))
d15 = 4570911 ; 2E23*(cos(1pi/16)*cos(5pi/16))
d16 = 3148499 ; 2E23*(cos(1pi/16)*cos(6pi/16))
d17 = 1605091 ; 2E23*(cos(1pi/16)*cos(7pi/16))
; aanidct - quantization coefficients - 3rd row
d20 = 5480122 ; 2E23*(cos(2pi/16)*cos(4pi/16))
d21 = 7601148 ; 2E23*(cos(2pi/16)*cos(1pi/16))
d22 = 7160125 ; 2E23*(cos(2pi/16)*cos(2pi/16))
d23 = 6443942 ; 2E23*(cos(2pi/16)*cos(3pi/16))
d24 = 5480122 ; 2E23*(cos(2pi/16)*cos(4pi/16))
d25 = 4305704 ; 2E23*(cos(2pi/16)*cos(5pi/16))
d26 = 2965821 ; 2E23*(cos(2pi/16)*cos(6pi/16))
d27 = 1511962 ; 2E23*(cos(2pi/16)*cos(7pi/16))
; aanidct - quantization coefficients - 4th row
d30 = 4931980 ; 2E23*(cos(3pi/16)*cos(4pi/16))
d31 = 6840852 ; 2E23*(cos(3pi/16)*cos(1pi/16))
d32 = 6443942 ; 2E23*(cos(3pi/16)*cos(2pi/16))
d33 = 5799395 ; 2E23*(cos(3pi/16)*cos(3pi/16))
d34 = 4931980 ; 2E23*(cos(3pi/16)*cos(4pi/16))
d35 = 3875032 ; 2E23*(cos(3pi/16)*cos(5pi/16))
d36 = 2669168 ; 2E23*(cos(3pi/16)*cos(6pi/16))
d37 = 1360730 ; 2E23*(cos(3pi/16)*cos(7pi/16))
; aanidct - quantization coefficients - 5th row
d40 = 4194304 ; 2E23*(cos(4pi/16)*cos(4pi/16))
d41 = 5817667 ; 2E23*(cos(4pi/16)*cos(1pi/16))
d42 = 5480122 ; 2E23*(cos(4pi/16)*cos(2pi/16))
d43 = 4931980 ; 2E23*(cos(4pi/16)*cos(3pi/16))
d44 = 4194304 ; 2E23*(cos(4pi/16)*cos(4pi/16))
d45 = 3295444 ; 2E23*(cos(4pi/16)*cos(5pi/16))
d46 = 2269941 ; 2E23*(cos(4pi/16)*cos(6pi/16))
d47 = 1157206 ; 2E23*(cos(4pi/16)*cos(7pi/16))
; aanidct - quantization coefficients - 6th row
d50 = 3295444 ; 2E23*(cos(5pi/16)*cos(4pi/16))
d51 = 4570911 ; 2E23*(cos(5pi/16)*cos(1pi/16))
d52 = 4305704 ; 2E23*(cos(5pi/16)*cos(2pi/16))
d53 = 3875032 ; 2E23*(cos(5pi/16)*cos(3pi/16))
d54 = 3295444 ; 2E23*(cos(5pi/16)*cos(4pi/16))
d55 = 2589213 ; 2E23*(cos(5pi/16)*cos(5pi/16))
d56 = 1783481 ; 2E23*(cos(5pi/16)*cos(6pi/16))
d57 = 0909211 ; 2E23*(cos(5pi/16)*cos(7pi/16))
; aanidct - quantization coefficients - 7th row
d60 = 2269941 ; 2E23*(cos(6pi/16)*cos(4pi/16))
d61 = 3148499 ; 2E23*(cos(6pi/16)*cos(1pi/16))
d62 = 2965821 ; 2E23*(cos(6pi/16)*cos(2pi/16))
d63 = 2669168 ; 2E23*(cos(6pi/16)*cos(3pi/16))
d64 = 2269941 ; 2E23*(cos(6pi/16)*cos(4pi/16))
d65 = 1783481 ; 2E23*(cos(6pi/16)*cos(5pi/16))
d66 = 1228483 ; 2E23*(cos(6pi/16)*cos(6pi/16))
d67 = 0626275 ; 2E23*(cos(6pi/16)*cos(7pi/16))
; aanidct - quantization coefficients - 8th row
d70 = 1157206 ; 2E23*(cos(7pi/16)*cos(4pi/16))
d71 = 1605091 ; 2E23*(cos(7pi/16)*cos(1pi/16))
d72 = 1511962 ; 2E23*(cos(7pi/16)*cos(2pi/16))
d73 = 1360730 ; 2E23*(cos(7pi/16)*cos(3pi/16))
d74 = 1157206 ; 2E23*(cos(7pi/16)*cos(4pi/16))
d75 = 0909211 ; 2E23*(cos(7pi/16)*cos(5pi/16))
d76 = 0626275 ; 2E23*(cos(7pi/16)*cos(6pi/16))
d77 = 0319272 ; 2E23*(cos(7pi/16)*cos(7pi/16))

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
; luminance - aanidct - scaled dequantization table
yqi dd d00*y00,d01*y01,d02*y02,d03*y03,d04*y04,d05*y05,d06*y06,d07*y07
    dd d10*y10,d11*y11,d12*y12,d13*y13,d14*y14,d15*y15,d16*y16,d17*y17
    dd d20*y20,d21*y21,d22*y22,d23*y23,d24*y24,d25*y25,d26*y26,d27*y27
    dd d30*y30,d31*y31,d32*y32,d33*y33,d34*y34,d35*y35,d36*y36,d37*y37
    dd d40*y40,d41*y41,d42*y42,d43*y43,d44*y44,d45*y45,d46*y46,d47*y47
    dd d50*y50,d51*y51,d52*y52,d53*y53,d54*y54,d55*y55,d56*y56,d57*y57
    dd d60*y60,d61*y61,d62*y62,d63*y63,d64*y64,d65*y65,d66*y66,d67*y67
    dd d70*y70,d71*y71,d72*y72,d73*y73,d74*y74,d75*y75,d76*y76,d77*y77
; luminance - huffman decode tables - dc coefficients
ydd db 00h,00h,01h,02h,03h,03h,04h,04h ; two 4 byte tables
ydt db 05h,06h,07h,08h,09h,0Ah,0Bh,0Eh,0Eh,0Eh,0Eh,0Eh,0Eh,0Eh
; luminance - huffman decode tables - ac coefficients
yad db 10h,10h,10h,10h,20h,20h,20h,20h ; nine 8 byte tables
    db 30h,30h,30h,30h,00h,00h,40h,40h,11h,11h,11h,11h,50h,50h,21h,21h
    db 12h,12h,12h,12h,13h,13h,14h,14h,60h,60h,31h,31h,15h,15h,16h,16h
    db 70h,70h,22h,22h,17h,17h,41h,23h,18h,18h,19h,19h,1Ah,1Ah,80h,32h
    db 24h,24h,1Bh,1Bh,1Ch,1Ch,51h,25h,1Dh,1Dh,0Fh,0Fh,42h,33h,26h,27h
; luminance - huffman decode table - ac coefficients
yat db 28h,28h,90h,160,61h,71h,81h,91h,161,52h,62h,72h,82h,92h,162,43h
    db 53h,63h,73h,83h,93h,163,34h,44h,54h,64h,74h,84h,94h,164,35h,45h
    db 55h,65h,75h,85h,95h,165,36h,46h,56h,66h,76h,86h,96h,166,37h,47h
    db 57h,67h,77h,87h,97h,167,38h,48h,58h,68h,78h,88h,98h,168,29h,39h
    db 49h,59h,69h,79h,89h,99h,169,2Ah,3Ah,4Ah,5Ah,6Ah,7Ah,8Ah,9Ah,170
    db 2Bh,3Bh,4Bh,5Bh,6Bh,7Bh,8Bh,9Bh,171,2Ch,3Ch,4Ch,5Ch,6Ch,7Ch,8Ch
    db 9Ch,172,2Dh,3Dh,4Dh,5Dh,6Dh,7Dh,8Dh,9Dh,173,1Eh,2Eh,3Eh,4Eh,5Eh
    db 6Eh,7Eh,8Eh,9Eh,174,1Fh,2Fh,3Fh,4Fh,5Fh,6Fh,7Fh,8Fh,9Fh,175,0Eh

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

dataseg
; chrominance - aanidct - scaled dequantization table
cqi dd d00*c00,d01*c01,d02*c02,d03*c03,d04*c04,d05*c05,d06*c06,d07*c07
    dd d10*c10,d11*c11,d12*c12,d13*c13,d14*c14,d15*c15,d16*c16,d17*c17
    dd d20*c20,d21*c21,d22*c22,d23*c23,d24*c24,d25*c25,d26*c26,d27*c27
    dd d30*c30,d31*c31,d32*c32,d33*c33,d34*c34,d35*c35,d36*c36,d37*c37
    dd d40*c40,d41*c41,d42*c42,d43*c43,d44*c44,d45*c45,d46*c46,d47*c47
    dd d50*c50,d51*c51,d52*c52,d53*c53,d54*c54,d55*c55,d56*c56,d57*c57
    dd d60*c60,d61*c61,d62*c62,d63*c63,d64*c64,d65*c65,d66*c66,d67*c67
    dd d70*c70,d71*c71,d72*c72,d73*c73,d74*c74,d75*c75,d76*c76,d77*c77
; chrominance - huffman decode tables - dc coefficients
cdd db 00h,00h,01h,01h,02h,02h,02h,02h ; two 4 byte tables
cdt db 03h,04h,05h,06h,07h,08h,09h,0Ah,0Bh,0Eh,0Eh,0Eh,0Eh,0Eh
; chrominance - huffman decode tables - ac coefficients
cad db 00h,00h,00h,00h,10h,10h,10h,10h ; nine 8 byte tables
    db 20h,20h,20h,20h,30h,30h,11h,11h,40h,40h,50h,50h,12h,12h,13h,13h
    db 60h,60h,21h,21h,14h,14h,15h,15h,70h,70h,16h,16h,17h,17h,31h,22h
    db 23h,23h,18h,18h,80h,41h,24h,19h,1Ah,1Ah,1Bh,1Bh,1Ch,1Ch,90h,32h
    db 33h,33h,25h,25h,0Fh,0Fh,51h,26h,27h,27h,1Dh,1Dh,160,61h,42h,43h
; chrominance - huffman decode table - ac coefficients
cat db 1Eh,1Eh,1Eh,1Eh,52h,52h,1Fh,1Fh,71h,81h,91h,161,62h,72h,82h,92h
    db 162,53h,63h,73h,83h,93h,163,34h,44h,54h,64h,74h,84h,94h,164,35h
    db 45h,55h,65h,75h,85h,95h,165,36h,46h,56h,66h,76h,86h,96h,166,37h
    db 47h,57h,67h,77h,87h,97h,167,28h,38h,48h,58h,68h,78h,88h,98h,168
    db 29h,39h,49h,59h,69h,79h,89h,99h,169,2Ah,3Ah,4Ah,5Ah,6Ah,7Ah,8Ah
    db 9Ah,170,2Bh,3Bh,4Bh,5Bh,6Bh,7Bh,8Bh,9Bh,171,2Ch,3Ch,4Ch,5Ch,6Ch
    db 7Ch,8Ch,9Ch,172,2Dh,3Dh,4Dh,5Dh,6Dh,7Dh,8Dh,9Dh,173,2Eh,3Eh,4Eh
    db 5Eh,6Eh,7Eh,8Eh,9Eh,174,2Fh,3Fh,4Fh,5Fh,6Fh,7Fh,8Fh,9Fh,175,0Eh

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

dataseg
GoodImage db 0

dataseg
CntRestart dd 0
UseRestart dd 0

udataseg
dcValue dd 3 dup(?)

codeseg
proc decoder near
; clear mcu values
  sub eax,eax ; zero
  mov ecx,4*64 ; dwords
  mov edi,offset(mcu)
  rep stosd ; clear
; fetch huffman state
  mov eax,[hcInBits]
  mov edi,[hcOffset]
  mov ecx,[hcStatus]
; point to first matrix
  sub ebp,ebp ; offset
; setup luminance tables
  sub esi,esi ; offset
label NextDCdecode near
; decode dc huffman code
label VerifyBits0 near
; check enough bits present
  cmp cl,16 ; maximum size
  jnb EndVerifyBits0 ; ok
; obtain additional bits
  shl eax,8 ; alignment
  mov al,[outBuffer+edi]
  inc edi ; next position
; disregard stuffed zero byte
  cmp [word(outBuffer+edi-1)],00FFh
  jne EndSkipZero0 ; use byte
  inc edi ; next position
label EndSkipZero0 near
  add cl,8 ; bits per byte
  jmp VerifyBits0 ; check
label EndVerifyBits0 near
; determine first 0 bit
  sub ebx,ebx ; zeroes
  ror eax,cl ; alignment
  shld ebx,eax,16 ; string
  not ebx ; first 1 bit
  bsr dx,bx ; bit index
  not ebx ; first 0 bit
; determine decode table
  cmp dl,13 ; boundary
  jna dcLongTable
; access short tables
  push ecx ; save reg
  mov cl,dl ; pos 0 bit
  mov dh,15 ; maximum pos
  sub cl,2 ; pos index bits
  sub dh,dl ; table number
  shr ebx,cl ; align index
  shr edx,6 ; align number
  pop ecx ; restore reg
  and edx,04h ; number
  and ebx,03h ; index
  xor ebx,edx ; address
  mov bl,[ydd+esi+ebx]
  jmp EndDCdecode
label dcLongTable near
; access longer table
  mov bl,13 ; maximum pos
  sub bl,dl ; table index
  and ebx,0Fh ; address
  mov bl,[ydt+esi+ebx]
label EndDCDecode near
; check valid symbol
  cmp bl,0Eh ; <BAD>
  jne dcValid ; ok
  mov [GoodImage],0
  jmp EndDecoder
label dcValid near
; strip huffman code
  rol eax,cl ; alignment
  sub cl,[yds+esi+ebx]
; check data available
  sub edx,edx ; zero
  cmp bl,00h ; minimum
  jna ApplyReference
; obtain dc huffman data
label VerifyBits1 near
; check enough bits present
  cmp cl,11 ; maximum size
  jnb EndVerifyBits1 ; ok
; obtain additional bits
  shl eax,8 ; alignment
  mov al,[outBuffer+edi]
  inc edi ; next position
; disregard stuffed zero byte
  cmp [word(outBuffer+edi-1)],00FFh
  jne EndSkipZero1 ; use byte
  inc edi ; next position
label EndSkipZero1 near
  add cl,8 ; bits per byte
  jmp VerifyBits1 ; check
label EndVerifyBits1 near
; obtain dc data value
  ror eax,cl ; alignment
  cmc ; negative borrow
  sbb edx,0 ; negative
  push ecx ; save reg
  mov cl,bl ; data size
  shld edx,eax,cl ; value
  pop ecx ; restore reg
  adc edx,0 ; negative
; strip dc data value
  rol eax,cl ; alignment
  sub cl,bl ; data size
label ApplyReference near
; add reference dc value
  mov ebx,ebp ; matrix index
  shr ebx,6 ; derive index
  sub ebx,1 ; adjust index
  adc ebx,0 ; proper index
  add edx,[dcValue+ebx*4]
; set reference dc value
  mov [dcValue+ebx*4],edx
; apply dequantize factor
  push eax ; save register
  mov eax,[yqi+esi] ; first
  imul edx ; DC*2E0*DQ*2E23
  shld edx,eax,25 ; DC*2E16
  pop eax ; restore register
; store dc component value
  mov [mcu+ebp*4],edx
; bump to next element
  lea ebp,[ebp+1] ; offset
label NextACdecode near
; decode ac huffman code
label VerifyBits2 near
; check enough bits present
  cmp cl,16 ; maximum size
  jnb EndVerifyBits2 ; ok
; obtain additional bits
  shl eax,8 ; alignment
  mov al,[outBuffer+edi]
  inc edi ; next position
; disregard stuffed zero byte
  cmp [word(outBuffer+edi-1)],00FFh
  jne EndSkipZero2 ; use byte
  inc edi ; next position
label EndSkipZero2 near
  add cl,8 ; bits per byte
  jmp VerifyBits2 ; check
label EndVerifyBits2 near
; determine first 0 bit
  sub ebx,ebx ; zeroes
  ror eax,cl ; alignment
  shld ebx,eax,16 ; string
  not ebx ; first 1 bit
  bsr dx,bx ; bit index
  not ebx ; first 0 bit
; determine decode table
  cmp dl,6 ; boundary
  jna acLongTable
; access short tables
  push ecx ; save reg
  mov cl,dl ; pos 0 bit
  mov dh,15 ; maximum pos
  sub cl,3 ; pos index bits
  sub dh,dl ; table number
  shr ebx,cl ; align index
  shr edx,5 ; align number
  pop ecx ; restore reg
  and edx,78h ; number
  and ebx,07h ; index
  xor ebx,edx ; address
  mov bl,[yad+esi+ebx]
  jmp EndACdecode
label acLongTable near
; access longer table
  and ebx,7Fh ; address
  mov bl,[yat+esi+ebx]
label EndACDecode near
; check valid symbol
  cmp bl,0Eh ; <BAD>
  jne acValid ; ok
  mov [GoodImage],0
  jmp EndDecoder
label acValid near
; strip huffman code
  rol eax,cl ; alignment
  sub cl,[yas+esi+ebx]
; check end of block
  cmp bl,00h ; <EOB>
  jne NotEndOfBlock
; skip to last position
  or ebp,03Fh ; complete
  jmp EndDecode ; done
label NotEndOfBlock near
; check insert zeroes
  cmp bl,0Fh ; <ZRL>
  jne NotSkipZeroes
; skip to next position
  add ebp,00Fh ; sixteen
  jmp EndDecode ; done
label NotSkipZeroes near
; verify data available
  jna EndDecode ; absent
; skip leading zeroes
  mov dl,bl ; symbol
  and edx,0Fh ; zeroes
  add ebp,edx ; counter
; obtain ac huffman data
label VerifyBits3 near
; check enough bits present
  cmp cl,11 ; maximum size
  jnb EndVerifyBits3 ; ok
; obtain additional bits
  shl eax,8 ; alignment
  mov al,[outBuffer+edi]
  inc edi ; next position
; disregard stuffed zero byte
  cmp [word(outBuffer+edi-1)],00FFh
  jne EndSkipZero3 ; use byte
  inc edi ; next position
label EndSkipZero3 near
  add cl,8 ; bits per byte
  jmp VerifyBits3 ; check
label EndVerifyBits3 near
; obtain ac data value
  shr ebx,4 ; data size
  sub edx,edx ; zero bits
  ror eax,cl ; alignment
  cmc ; negative borrow
  sbb edx,0 ; negative
  push ecx ; save reg
  mov cl,bl ; data size
  shld edx,eax,cl ; value
  pop ecx ; restore reg
  adc edx,0 ; negative
; strip ac data value
  rol eax,cl ; alignment
  sub cl,bl ; data size
; prepare zigzag sequence
  mov ebx,ebp ; matrix index
  mov ch,bl ; matrix index
  and bl,03Fh ; zigzag index
  and ch,0C0h ; isolate matrix
  mov bl,[zzo+ebx] ; sequence
; apply dequantize factor
  push eax ; save register
  mov eax,[yqi+esi+ebx*4]
  imul edx ; AC*2E0*DQ*2E23
  shld edx,eax,25 ; AC*2E16
  pop eax ; restore register
; store ac component value
  xor bl,ch ; use matrix
  mov [mcu+ebx*4],edx
label EndDecode near
; bump to next element
  lea ebp,[ebp+1] ; offset
  test ebp,03Fh ; more
  jnz NextACdecode
; start next luminance
  cmp ebp,080h ; done
  jb NextDCdecode
; start next chrominance
  mov esi,cqi-yqi ; offset
  cmp ebp,100h ; done
  jb NextDCdecode
; check restart markers
  cmp [UseRestart],0
  jna EndDecoder
  dec [CntRestart]
  jnz EndDecoder
  mov edx,[UseRestart]
  mov [CntRestart],edx
label VerifyBits4 near
; check enough bits present
  cmp cl,16 ; maximum size
  jnb EndVerifyBits4 ; ok
; obtain additional bits
  shl eax,8 ; alignment
  mov al,[outBuffer+edi]
  inc edi ; next position
; disregard stuffed zero byte
  cmp [word(outBuffer+edi-1)],00FFh
  jne EndSkipZero4 ; use byte
  inc edi ; next position
label EndSkipZero4 near
  add cl,8 ; bits per byte
  jmp VerifyBits4 ; check
label EndVerifyBits4 near
; skip filler byte present
  cmp ax,0FFFFh ; filler
  jne CheckMarker ; no
  sub cl,8 ; bits per byte
  jmp VerifyBits4 ; skip
label CheckMarker near
; verify restart marker
  cmp ah,0FFh ; marker
  jb EndDecoder ; no
  cmp al,0D0h ; restart
  jb EndDecoder ; no
  cmp al,0D7h ; restart
  ja EndDecoder ; no
label ResetDecoder near
  sub eax,eax ; zero
  mov [dcValue+0],eax
  mov [dcValue+4],eax
  mov [dcValue+8],eax
  sub ecx,ecx ; zero
label EndDecoder near
; store huffman state
  mov [hcOffset],edi
  mov [hcStatus],ecx
  mov [hcInBits],eax
  ret ; return
endp decoder

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

; precalculated ycc2rgb coefficients
gcb = 0369517511 ; 0.34414*2E30
gcr = 0766801986 ; 0.71414*2E30
mcb = 1902670512 ; 1.772*2E30
mcr = 1505386037 ; 1.402*2E30

codeseg
proc jpg2rgb near
  call reader ; mjpg
; clear old dc values
  sub eax,eax ; zero
  mov [dcValue+0],eax
  mov [dcValue+4],eax
  mov [dcValue+8],eax
; clear huffman state
  mov [hcOffset],eax
  mov [hcStatus],eax
; read image to build minimum coded units
  mov ebp,offset(rgbBuffer)+3*Ix*(Iy-1)
  push esi ; save
label ReadImage near
; set horizontal units
  mov cl,Ix/16 ; 4:2:2
label ReadNextUnit near
; get minimum coded unit
  push ebp ecx ; save
  call decoder ; huffman
  call aanidct ; inverse
  pop ecx ebp ; restore
; scan minimum coded unit
  sub edi,edi ; mcu index
label UnitRowNext near
  mov ch,8 ; columns
label ycc2rgb near
; y2rgb midpoint shift
  movzx edx,[yyo+edi]
  mov ebx,[mcu+edx*8+0]
  mov esi,[mcu+edx*8+4]
  add ebx,c128 ; 128*2E18
  add esi,c128 ; 128*2E18
  sar ebx,18 ; Y0*2E0
  sar esi,18 ; Y1*2E0
; ycb2b conversion
  mov edx,[mcu+512+edi*4]
  mov eax,mcb ; mfb*2E30
  imul edx ; mfb*Cb*2E16
  sar edx,16 ; mfb*Cb
; calculate 1st B value
  lea eax,[ebx+edx] ; B0
  test ah,ah ; validate
  jz StoreThisB0 ; correct
  mov al,000h ; minimum
  js StoreThisB0 ; correct
  mov al,0FFh ; maximum
label StoreThisB0 near
  mov [ebp+0],al ; B0
; calculate 2nd B value
  lea eax,[esi+edx] ; B1
  test ah,ah ; validate
  jz StoreThisB1 ; correct
  mov al,000h ; minimum
  js StoreThisB1 ; correct
  mov al,0FFh ; maximum
label StoreThisB1 near
  mov [ebp+3],al ; B1
; ycr2r conversion
  mov edx,[mcu+768+edi*4]
  mov eax,mcr ; mfr*2E30
  imul edx ; mfr*Cr*2E16
  sar edx,16 ; mfr*Cr
; calculate 1st R value
  lea eax,[ebx+edx] ; R0
  test ah,ah ; validate
  jz StoreThisR0 ; correct
  mov al,000h ; minimum
  js StoreThisR0 ; correct
  mov al,0FFh ; maximum
label StoreThisR0 near
  mov [ebp+2],al ; R0
; calculate 2nd R value
  lea eax,[esi+edx] ; R1
  test ah,ah ; validate
  jz StoreThisR1 ; correct
  mov al,000h ; minimum
  js StoreThisR1 ; correct
  mov al,0FFh ; maximum
label StoreThisR1 near
  mov [ebp+5],al ; R1
; ycbcr2g conversion
  mov edx,[mcu+512+edi*4]
  mov eax,gcb ; gfb*2E30
  imul edx ; gfb*Cb*2E16
  sar edx,16 ; gfb*Cb
  sub ebx,edx ; Y0-gfb*Cb
  sub esi,edx ; Y1-gfb*Cb
  mov edx,[mcu+768+edi*4]
  mov eax,gcr ; gfr*2E30
  imul edx ; gfr*Cr*2E16
  sar edx,16 ; gfr*Cr
  sub esi,edx ; G1
  sub ebx,edx ; G0
  mov edx,esi ; G1
; validate 1st G value
  test bh,bh ; validate
  jz StoreThisG0 ; correct
  mov bl,000h ; minimum
  js StoreThisG0 ; correct
  mov bl,0FFh ; maximum
label StoreThisG0 near
  mov [ebp+1],bl ; G0
; validate 2nd G value
  test dh,dh ; validate
  jz StoreThisG1 ; correct
  mov dl,000h ; minimum
  js StoreThisG1 ; correct
  mov dl,0FFh ; maximum
label StoreThisG1 near
  mov [ebp+4],dl ; G1
; bump to next column
  lea ebp,[ebp+6]
  lea edi,[edi+1]
  dec ch ; columns
  jnz ycc2rgb ; more
; bump to next row
  lea ebp,[ebp-16*3-1*3*Ix]
  cmp edi,64 ; complete
  jb UnitRowNext ; more
; bump pointer horizontally
  lea ebp,[ebp+16*3+8*3*Ix]
  dec cl ; horizontal units
  jnz ReadNextUnit ; more
; bump pointer vertically
  lea ebp,[ebp-(8+1)*3*Ix]
; verify image complete
  cmp ebp,offset(rgbBuffer)
  ja ReadImage ; more
  pop esi ; restore
  ret ; return
endp jpg2rgb

dataseg
; aanidct - dequantization table
iqt dd d00,d01,d02,d03,d04,d05,d06,d07
    dd d10,d11,d12,d13,d14,d15,d16,d17
    dd d20,d21,d22,d23,d24,d25,d26,d27
    dd d30,d31,d32,d33,d34,d35,d36,d37
    dd d40,d41,d42,d43,d44,d45,d46,d47
    dd d50,d51,d52,d53,d54,d55,d56,d57
    dd d60,d61,d62,d63,d64,d65,d66,d67
    dd d70,d71,d72,d73,d74,d75,d76,d77

codeseg
proc reader near
; process SOI marker
  sub ecx,ecx ; zero
  mov [UseRestart],ecx
  lea esi,[esi+2]
label NextMarker near
; address next marker
  mov ch,[esi+2] ; >length
  mov cl,[esi+3] ; <length
  lea esi,[esi+ecx+2]
; avoid buffer overflow
  cmp esi,edi ; boundary
  jb UseMarker ; accept
  mov [GoodImage],0
  ret ; return
label UseMarker near
; process markers
  mov eax,[esi]
; process DRI marker
  cmp eax,0400DDFFh
  je Restart
; process DQT marker
  cmp eax,4300DBFFh
  je Factors
; process SOS marker
  cmp eax,0C00DAFFh
  je Huffman
; skip other markers
  jmp NextMarker
label Restart near
; set interval value
  mov ch,[esi+4] ; >value
  mov cl,[esi+5] ; <value
  mov [CntRestart],ecx
  mov [UseRestart],ecx
  jmp NextMarker
label Factors near
; update dequantization
  sub ebx,ebx ; zero
  sub ecx,ecx ; zero
  sub edx,edx ; zero
  mov cl,64 ; factors
; update luminance
  mov ebp,offset(yqi)
  cmp [byte(esi+4)],0
  je SetTable ; ok
; update chrominance
  mov ebp,offset(cqi)
  cmp [byte(esi+4)],1
  je SetTable ; ok
  jmp NextMarker
label SetTable near
  mov bl,[zzo+ecx-1]
  mov dl,[esi+ecx+4]
; obtain scaling factor
  mov eax,[iqt+ebx*4]
  mul edx ; compression
; set dequantize factor
  mov [ebp+ebx*4],eax
  loop SetTable
  jmp NextMarker
label Huffman near
; copy huffman codes
  lea esi,[esi+14]
  mov ecx,edi ; end
  sub ecx,esi ; size
  mov edi,offset(outBuffer)
  rep movsb ; copy
  ret ; return
endp reader

; precalculated rgb2ycc coefficients
cbm = 1212555137 ; 0.564639986*2E31
crm = 1531532078 ; 0.713175199*2E31
ybc = 0245843928 ; 0.11448*2E31
ygc = 1259735383 ; 0.58661*2E31
yrc = 0641904337 ; 0.29891*2E31

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
HdrCOM db 0FFh,0FEh,0,15,"Wim's UvcView"
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

dataseg
ssOutput db 'uvcview.jpg',0

codeseg
proc writer near
; open jpg output file
  call DosOpen c,offset(ssOutput),offset(fhOutput),offset(ActionTaken),0,0,012h,0191h,0
  test eax,eax ; check for errors
  jnz NotWriter ; failure
; write headers to jpg output file
  call DosWrite c,[fhOutput],offset(HdrSOI),HdrEOI-HdrSOI,offset(BytesDone)
  test eax,eax ; check for errors
  jnz EndWriter ; failure
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
; address huffman buffer
  mov esi,offset(outBuffer)
; write huffman to jpg output file
  call DosWrite c,[fhOutput],esi,edi,offset(BytesDone)
  test eax,eax ; check for errors
  jnz EndWriter ; failure
label EndWriter near
; close jpg output file
  call DosClose c,[fhOutput]
label NotWriter near
  ret ; return
endp writer
