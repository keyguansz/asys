git status
pause
git add --all
git commit -m 'auto-commit'
git status
pause
@SET /a tryCnt=0
@SET /a MaxTryCnt=10
:PUSH_LOOP
set /a tryCnt=%tryCnt%+1
ECHO try # %tryCnt%
git push
IF ERRORLEVEL == 0 (
    ECHO "git push success!!!"
) ELSE (
	IF %tryCnt% LEQ %MaxTryCnt% (
		GOTO PUSH_LOOP
	)
)
pause
exit


pause
exit