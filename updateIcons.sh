#!/bin/bash
# This script aims at copying all image files described in the readme in the current directory.
# it also warns the user if the license has been changed.

DECLARATION_FILE=img/LICENSE

echo
echo Check of icon declaration and effective source and license...
echo

for icon in `grep png $DECLARATION_FILE`
do
  targetFile=`echo $icon | cut -d ':' -f 1`
  sourceFile=`echo $icon | cut -d ':' -f 2`
  oldLicense=`echo $icon | cut -d ':' -f 3-`  

  if [[ $sourceFile =~ ^/.+ ]]
    then
    nameAndLicense=`rpm -qf $sourceFile --queryformat '%{NAME}:%{LICENSE}\n'`
    echo "$targetFile (Provided by: $nameAndLicense)"
    
    cp $sourceFile img/$targetFile
    if [[ $oldLicense != $nameAndLicense ]]
    then
      echo "  ************* Warning ($targetFile) !! *************"
      echo "  Declared Name and License: <$oldLicense>"
      echo "  Actual Name and License: <$nameAndLicense>"
      echo
    fi
  fi

done

echo

