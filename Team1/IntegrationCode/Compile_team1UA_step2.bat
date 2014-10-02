@SET PATH=C:/cygwin/bin;%PATH%
@SET INCLUDE=C:/cygwin/usr/include
@SET LIB=C:/cygwin/lib
@SET CYGWIN=nodosfilewarning
@IF "-B"=="%1" SET ARG=%1
@C:/cygwin/bin/make.exe %ARG%
pause