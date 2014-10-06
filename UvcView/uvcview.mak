uvcview.exe: uvcview.def uvcview.obj uvcview.res
  link386 /a:16 /map /nod uvcview,uvcview.exe,,os2+usbcalls,uvcview
  markexe mpunsafe uvcview.exe
  rc uvcview.res uvcview.exe
  @pause

uvcview.obj: uvcview.asm uvcview.inc uvcview.mak
  tasm /la /m /oi uvcview.asm,uvcview.obj

uvcview.res: uvcview.rc
  rc /r uvcview.rc uvcview.res
