git status
pause
@SET /a tryCnt=0
@SET /a MaxTryCnt=10
:PULL_LOOP
set /a tryCnt=%tryCnt%+1
ECHO try # %tryCnt%
git pull
IF ERRORLEVEL == 0 (
    ECHO "git pull success!!!"
) ELSE (
	IF %tryCnt% LEQ %MaxTryCnt% (
		GOTO PULL_LOOP
	)
)
pause
exit