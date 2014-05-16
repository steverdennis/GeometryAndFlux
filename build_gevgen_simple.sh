#!/bin/bash


if [ -z $LOG4CPP_LIB ] ; then
  echo "Need LOG4CPP_LIB environment variable set"
  return 
fi

command="g++ -o gevgen_simple -DSTANDALONE \
  -Wl,--no-as-needed \
  `root-config --cflags ` \
  -I${GENIE}/src \
  gevgen_simple.C \
  -L$LOG4CPP_LIB -llog4cpp -lnsl \
  `xml2-config --libs`  \
  `root-config --glibs ` \
  -lGeom -lEGPythia6 \
  -L$PYTHIA6 -lPythia6 \
  `genie-config --libs` \
  "

echo $command
$command
