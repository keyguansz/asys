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
@REM >=1
IF ERRORLEVEL 1 (
	IF %tryCnt% LEQ %MaxTryCnt% (
		GOTO PUSH_LOOP
	)
) ELSE (
	ECHO "git push success!!!"
)
ECHO git status
git status
pause
exit


pause
exit