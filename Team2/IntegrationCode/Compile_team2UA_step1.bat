@SET PATH=C:/Program Files (x86)/Esterel Technologies/Esterel SCADE 6.4.2/SCADE/bin;%PATH%
@echo off
cd .
call uaadaptor.exe -n "C:\Program Files (x86)\Esterel Technologies\Esterel SCADE 6.4.2\SCADE Display\config\Workspace\environments\UAPageCreator\A661Norm\a661.xml"  -k "..\UserApplication\KCG\kcg_trace.xml" -d "..\DefinitionFile\project2.sgfx" "..\UserApplication\project2.sdy" -outdir "..\UserApplication\KCG"
pause