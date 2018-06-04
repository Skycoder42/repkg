#!/bin/sh
set -e

/home/sky/Programming/QtProjects/build-repkg-System_Qt_5_11_0-Debug/repkg update --stdin

updatePkg=$(/usr/bin/repkg list)
if [ -n "$updatePkg" ]; then
	echo -e "\e[36m>>>\e[0m \e[33mSome packages need to be rebuilt. See list below:\e[0m "
	echo -e "\e[36m>>>\e[0m $updatePkg"
	echo -e "\e[36m>>>\e[0m Run \e[32mrepkg\e[0m to automatically rebuild all packages that need one."
fi
