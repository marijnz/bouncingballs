:: // strip-start "Fmod"
copy /Y "%~1\api\studio\lib\fmodstudio64.dll" "%~2\fmodstudio64.dll"
copy /Y "%~1\api\studio\lib\fmodstudioL64.dll" "%~2\fmodstudioL64.dll"
copy /Y "%~1\api\lowlevel\lib\fmod64.dll" "%~2\fmod64.dll"
copy /Y "%~1\api\lowlevel\lib\fmodL64.dll" "%~2\fmodL64.dll"
:: // strip-end
exit /B 0