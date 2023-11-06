cl /f /nologo /AS /E li.c | xstr -c -
xstr
# TMP= cl /nologo /W4 /FPi87 /Gs3rsyf /AS /O2xazob2V0 /Feli d:x.c /Zl d:xs.c ..\\my_smap\\dosmems.obj /link /f /b /packc /packf /noe
TMP= cl /nologo /W4 /FPi87 /Gs3rsyf /AS /O2xazob2V0 /Feli d:x.c /Zl d:xs.c /link /f /b /packc /packf /noe /st:8192
rm d:x.c x.obj strings d:xs.c xs.obj
size li.exe
# pklite li
# rm li.bak
/wwpack/wwpack P li.exe
rm li.old
