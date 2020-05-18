#!/bin/bash

BRANCH=${1?No branch specified}
MESS=${2?No commit message specified}

git add .
git commit --author="xhercules <xhercules95@gmail.com>" -m $MESS
git push origin $BRANCH
