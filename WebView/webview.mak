webview.exe: webview.def webview.obj webview.res
  link386 /a:16 /map /nod webview,webview.exe,,os2+so32dll,webview
  markexe mpunsafe webview.exe
  rc webview.res webview.exe
  @pause

webview.obj: webview.asm webview.inc webview.mak
  tasm /la /m /oi webview.asm,webview.obj

webview.res: webview.rc
  rc /r webview.rc webview.res
