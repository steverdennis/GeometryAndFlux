#!/bin/bash

g++ -o gevgen_simple -DSTANDALONE -I$GENIE/src -I$ROOTSYS/include \
  `genie-config --libs` \
  `root-config --libs` -lGeom -lEGPythia6 \
  -L$PYTHIA6 -lPythia6 \
  -L$LOG4CPP_LIB -llog4cpp -lnsl \
  `xml2-config --libs`  \
  gevgen_simple.C

