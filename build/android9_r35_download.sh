#!/bin/bash
function usage()
{
cat << EOF
 Please input your username!
 USAGE: [-u USER_NAME]
EOF
 exit 1
}

if [ $# -eq 0 ]; then
	usage
fi

# check pass argument
while getopts "u:" arg
do
  case $arg in
    u)
      USER_NAME=$OPTARG
      ;;
    ?)
      usage
     ;;
  esac
done

GERRIT_URL="ssh://$USER_NAME@kd-ci.kjicorp.com:29418"
HOME_ROOT=$PWD

echo "GERRIT_URL=$GERRIT_URL"
repo init -u $GERRIT_URL/android-mirror/platform/manifest --repo-url $GERRIT_URL/public/repo -b android-9.0.0_r35

while true;do
.repo/repo/repo sync
if [ $? = 0 ]; then
	echo -e "\e[0;31;1mrepo sync 成功！\e[0m"
	.repo/repo/repo start --all master
	exit 0
else
	echo -e "\e[0;31;1mrepo sync失败，重新开始！\e[0m"
	sleep 5
fi
done
