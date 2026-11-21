@echo off
cd /d "%~dp0"
echo Mesure du moteur audio en cours. Cela peut prendre quelques minutes.
bench_audio.exe --full > performance-PC.csv
if errorlevel 1 (
 echo Echec du banc audio.
 pause
 exit /b 1
)
echo Termine : performance-PC.csv
pause
