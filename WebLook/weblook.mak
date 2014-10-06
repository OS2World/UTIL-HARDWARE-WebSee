weblook.exe: weblook.def weblook.obj
  link386 /a:16 /map /nod weblook,weblook.exe,,os2+so32dll+usbcalls,weblook
  markexe mpunsafe weblook.exe
  @pause

weblook.obj: weblook.asm weblook.inc weblook.mak
  tasm /la /m /oi weblook.asm,weblook.obj
