@SET PATH=C:/Program Files (x86)/Esterel Technologies/Esterel SCADE 6.4.2/SCADE A661/bin;%PATH%
@echo off
cd .
start A661Server.exe "..\DefinitionFile\project2DF\server_conf.xml" "..\DefinitionFile\project2DF\project2.bin"
