mingw32-g++.exe -L..\_resources\ddk\lib -L..\_resources\sl\lib -o ..\..\bin\Push.exe .objs\batch.o .objs\file.o .objs\GUI\cache.o .objs\GUI\copy.o .objs\GUI\gui.o .objs\GUI\main.o .objs\Hardware\disk.o .objs\Hardware\hwinfo.o .objs\ini.o .objs\push.o .objs\RAMdisk\ramdisk.o .objs\ring0.o  .objs\icon.res  -lslc -lsladl -lcomctl32 -ld3d9 -lpsapi -lntdll -ladvapi32 -lsetupapi -luser32 -lshell32 -lcomdlg32 -lstdlibprocess -lslgui -lslfile -lsldriver -lslmodule