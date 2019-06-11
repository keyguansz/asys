git rm -r app/build
git rm -r build
git rm local.properties
git rm -r .gradle
git rm -r *.iml
git rm -r .idea
git status 
pause
git add --all .
git commit -m "gitignore ok"
pause
git status 
git log
