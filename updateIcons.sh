#!/bin/bash
# This script aims at copying all image files described in the readme in the current directory.
# it also warns the user if the license has been changed.
for icon in `grep png Readme`
do
  targetFile=`echo $icon | cut -d ':' -f 1`
  sourceFile=`echo $icon | cut -d ':' -f 2`
  oldLicense=`echo $icon | cut -d ':' -f 3`
  
  if [[ $sourceFile =~ ^/.+ ]]
    then
    rpmFile=`rpm -qf $sourceFile`
    license=`rpm -q --queryformat '%{LICENSE}\n' $rpmFile`
    echo "$targetFile (RPM: $rpmFile)"
    
    cp $sourceFile $targetFile
    if [[ $oldLicense != $license ]]
    then
      echo "Warning !!"
      echo "Previous License: <$oldLicense>"
      echo "New License: <$license>"
    fi
    echo
  fi

done
