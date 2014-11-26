:: // strip-start "Fmod"
copy /Y "%~1\api\studio\lib\fmodstudio.dll" "%~2\fmodstudio.dll"
copy /Y "%~1\api\studio\lib\fmodstudioL.dll" "%~2\fmodstudioL.dll"
copy /Y "%~1\api\lowlevel\lib\fmod.dll" "%~2\fmod.dll"
copy /Y "%~1\api\lowlevel\lib\fmodL.dll" "%~2\fmodL.dll"
:: // strip-end
exit /B 0