/* uvcview.cmd - image post processing */
signal on error; signal on failure; signal on halt;
signal on novalue; signal on syntax;

/* convert bitmap to portable network graphic */
/*'gbmsize.exe -h 240 -w 320 uvcview.bmp uvcview.png'*/
return

/* return signal information */
error: failure: halt: novalue: syntax:
parse source system invokation filename
info=condition('c')||' condition raised at line '||sigl||' of'||'0d0a'x
info=info||filename||'0d0a'x||'- '||sourceline(sigl)
return info
