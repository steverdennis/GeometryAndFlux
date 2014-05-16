// a sample script for bringing together geometry and flux
// use:
// $ genie -b -q 'gevgen_simple.C+("myfile.gdml","mygsimple.root",1000,42)'
// for 1000 events w/ run # 42
// assumes that cross-section file can be found at $GENIEXSECFILE
// 
// R. Hatcher <rhatcher@fnal.gov> 2014-05-11

#if !defined(__CINT__) || defined(__MAKECINT__)
// hide from CINT, but not ACLiC
  #include <iostream>
  #include <string>  
  using namespace std;  // UnitUtils.h assumes this and #include <string>

  // GENIE includes
  #include "Numerical/RandomGen.h"
  #include "Utils/UnitUtils.h"
  #include "Geo/ROOTGeomAnalyzer.h"
  #include "FluxDrivers/GSimpleNtpFlux.h"
  #include "Utils/UnitUtils.h"
  #include "Utils/AppInit.h"
  #include "Numerical/RandomGen.h"
  #include "EVGCore/EventRecord.h"
  #include "EVGDrivers/GMCJDriver.h"
  #include "EVGDrivers/GMCJMonitor.h"
  #include "Ntuple/NtpMCFormat.h"
  #include "Ntuple/NtpWriter.h"

  // ROOT includes
  #include "TSystem.h"
  #include "TStopwatch.h"
  #include "TLorentzVector.h"
  #include "TNtuple.h"
  #include "TFile.h"

  // other includes
  #include <iomanip>
  #include <sstream>
  #include <set>
  #include <stdlib.h>  // for strtol, putenv, unsetenv

  #ifdef STANDALONE
    #ifndef MACOSX
      #include <getopt.h>
    #else
      #include <unistd.h>
    #endif
    #ifndef GETOPTDONE  // some getopt.h's have this, some don't
      #define GETOPTDONE (-1)
    #endif
  #endif

#endif 

// my own occassional laziness:
using namespace std;
using namespace genie;
using namespace genie::flux;

// forward declarations
GFluxI* getFluxDriver(std::string fluxstring);

/// main routine for ACLiC
void gevgen_simple(std::string geomfile="mylardetector.gdml",
                   std::string fluxstring="myfakeflux.gsimple.root",
                   long int nentries = 1000,
                   int runnum = 42,
                   long int seed = 12345) 
{
  /// Generate a gntp.<run#>.ghep.root file


  genie::utils::app_init::MesgThresholds("Messenger_laconic.xml");
  genie::utils::app_init::RandGen(seed);

  /// Geometry
  genie::geometry::ROOTGeomAnalyzer* geomDriver =
    new genie::geometry::ROOTGeomAnalyzer(geomfile);
  // ROOT geometries coming from GDML files are assumed have the units:
  double lunits = genie::utils::units::UnitFromString("cm");
  double dunits = genie::utils::units::UnitFromString("g_cm3");
  geomDriver->SetLengthUnits(lunits);
  geomDriver->SetDensityUnits(dunits);
  TGeoVolume* topvol = geomDriver->GetGeometry()->GetTopVolume();
  cout << "Top Volume is \"" << topvol->GetName() << "\"" << endl;

  /// Flux ... to support options we spill this off it's own function
  GFluxI* fluxDriver = getFluxDriver(fluxstring);

  /// Cross Sections
  const char* xsecfile = gSystem->Which(".","$GENIEXSECFILE");
  cout << "looking for xsec file: " << xsecfile << endl;
  genie::utils::app_init::XSecTable(xsecfile,true);

  /// Prepare the MC Job Driver
  genie::GMCJDriver* mcjDriver = new genie::GMCJDriver;
  mcjDriver->UseFluxDriver(fluxDriver);
  mcjDriver->UseGeomAnalyzer(geomDriver);
  mcjDriver->UseSplines();
  mcjDriver->Configure();
  mcjDriver->ForceSingleProbScale(); // unweighted events

  /// Prepare to write events into TTree
  genie::NtpWriter ntupleWriter(kNFGHEP,runnum);
  ntupleWriter.Initialize();
  /// and a way to monitor prograss
  genie::GMCJMonitor jobMonitor(runnum);
  jobMonitor.SetRefreshRate(1); // yeah, i really want to know...
  cout << "check \"genie-mcjob-" << runnum << ".status\""
       << " from another terminal for current event status" << endl;

  /// main event generation loop
  int ievt = 0;
  while ( ievt < nentries ) {
    genie::EventRecord* event = mcjDriver->GenerateEvent();
    ntupleWriter.AddEventRecord(ievt,event);
    jobMonitor.Update(ievt,event);
    ++ievt;
    delete event; // we took ownership from mcjDriver ... don't leak memory
  }

  /// Close output file
  ntupleWriter.Save();

  /// if using GSimpleNtpFlux calculate the exposure (POTs used)
  genie::flux::GSimpleNtpFlux* gsimple =
    dynamic_cast<genie::flux::GSimpleNtpFlux*>(fluxDriver);
  if ( gsimple ) {
    double fpot = gsimple->UsedPOTs();
    double pscale = mcjDriver->GlobProbScale();
    double pots = fpot / pscale;
    cout << "this represented " << pots << " POTs "
         << " (pscale=" << pscale << ")" << endl;
  }

  cout << "DONE - results should be in: "
       << "\"gntp." << runnum << ".ghep.root\"" << flush << endl;

  /// Clean up
  delete fluxDriver;  // not owned by mcjDriver
  delete geomDriver;  // not owned by mcjDriver
  delete mcjDriver;

  return;

}

///////////////////////////////////////////////////////////////////////////

GFluxI* getFluxDriver(std::string fluxstring)
{
  // support either GSimpleNtpFlux driver or histograms
  GFluxI* fdriver = 0;
  if ( fluxstring.find(".gsimple.root") != std::string::npos ) {

    // GSimpleNtpFlux
    GSimpleNtpFlux* gsflux = new GSimpleNtpFlux();
    gsflux->LoadBeamSimData(fluxstring,"abitrary string");
    fdriver = gsflux;

  } else if ( fluxstring.find(".hist.root") != std::string::npos ) {

    // histograms
    cerr << "not yet implemented .hist.root" << endl;
    exit(1);

  } else {

    cerr << "fluxstring must either be .gsimple.root or .hist.root" << endl;
    exit(1);

  }
  return fdriver;
}

///////////////////////////////////////////////////////////////////////////

#ifdef STANDALONE
/// main routine for stand-alone
int main(int argc, char** argv) 
{
#ifndef MACOSX
  optind = 0; // getopt.h:  reset getopt to start of args
#else
  optind = 1; // skip 0th argument (exe name) for MACOSX
#endif

  std::string geomfile="mylardetector.gdml";
  std::string fluxstring="myfakeflux.gsimple.root";
  long int nentries = 1000;
  int runnum = 42;
  long int seed = 12345;
  bool dohelp = false;
  int  doexit = 0;

  const char* opts = "g:f:n:r:h";
  int c;
  while ( ( c = getopt(argc,argv,opts) ) != GETOPTDONE ) {
    //cout << "opt \"" << char(c);
    //if (optarg) cout << "\" value \"" << optarg << "\"";
    //cout << endl;
    switch (c) {
    case 'g': geomfile   = std::string(optarg); break;
    case 'f': fluxstring = std::string(optarg); break;
    case 'n': nentries   = atoi(optarg);        break;
    case 'r': runnum     = atoi(optarg);        break;
    case 's': seed       = atoi(optarg);        break;
    case 'h': dohelp = true;                    break;
    default:
      cerr << "unknown option \"" << c << "\" value \"" 
           << optarg << "\"" << endl;
      dohelp = true;
      doexit = 1;
    }
  }

  if ( dohelp ) {
    cout << "usage: " << argv[0] << ": generate GENIE events" << endl;
    cout << "  -g <geomfile>"   << endl;
    cout << "  -f <fluxstring>" << endl;
    cout << "  -n <#events>"    << endl;
    cout << "  -4 <run#>"      << endl;
    cout << "  -s <seed#>"      << endl;
    cout << "  -h              this help" << endl;
    exit(doexit);
  }

  cout << argv[0] << " -g " << geomfile << " -f " << fluxstring
       << " -n " << nentries << " -r " << runnum << " -s " << seed
       << endl;

  gevgen_simple(geomfile,fluxstring,nentries,runnum,seed);
}
#endif


