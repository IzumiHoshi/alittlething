set cpu_core=0
setlocal ENABLEDELAYEDEXPANSION
FOR /F "tokens=1 delims=\r skip=1" %%i in ('wmic cpu get NumberOfLogicalProcessors') do (
    set line=%%i
    set line=!line:~0,-1!
    if not "!line!" == "" (
        :intercept
        if "!line:~-1!"==" " set "line=!line:~0,-1!" & goto intercept
        set /a cpu_core=!cpu_core!+!line!
    )
)
echo cpu=!cpu_core!
endlocal
