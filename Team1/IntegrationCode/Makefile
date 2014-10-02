include Makefile.$(shell uname)

ROOT=./
CC_FLAGS=-ansi -I . -I ../UserApplication/KCG/ -I ../../CommunicationInterface/ -I "C:/Program Files (x86)/Esterel Technologies/Esterel SCADE 6.4.2/SCADE/" -I "C:/Program Files (x86)/Esterel Technologies/Esterel SCADE 6.4.2/SCADE/include" -Wall -Wno-unused-variable -Wno-unused-function -Wno-unused-value $(OS_D)
SRC=team1UA.c ../UserApplication/KCG/kcg_consts.c ../UserApplication/KCG/kcg_types.c ../UserApplication/KCG/project1.c ../UserApplication/KCG/ua.c

OBJ=$(SRC:.c=.o)

team1UA$(OS_EXE_EXT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(OS_LIBS)

libua.a: $(OBJ)
	ar rcs $@ $(OBJ)

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) team1UA$(OS_EXE_EXT)

.PHONY: clean
