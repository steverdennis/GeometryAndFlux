// a simple script for loading a GDML file and drawing it with OpenGL
//
// R. Hatcher <rhatcher@fnal.gov> 2014-05-07
#include <iostream>
#include <string>
using namespace std;

#include "TGeoManager.h"

void load_draw_gdml(string fname = "myxtru.gdml",
                    string vname = "vWorld")
{
  TGeoManager::Import(fname.c_str());
  if (gGeoManager) gGeoManager->GetVolume(vname.c_str())->Draw("ogl");
  else { cerr << "could not load: " << fname << endl; }
}
