This material concerns the use of GENIE with an emphasis on geometry and
fluxes.  Associated with the presentation are some example files and scripts.
Careful of cut-and-paste from the powerpoint or PDF presentations as they
could have their use of single quote (') and double quote (") corrupted
by powerpoint to use stylistic quotes (curve opposite ways) which don't
correspond to normal keyboard symbols.

Here are some of the notes on what students and "touch and learn"

Examine the GDML geometry files in a text editor
   mysimplegeom.gdml  -- cylinder, box, partial sphere + boolean shapes
   myxtru.gdml        -- extrusion shape, frustrated cone
look at each of the major sections 
   <define>
   <materials>
   <solids>
   <structure>
   <setup>
with an emphasis on <structure> which defines the geometry hierarchy (very
simple in these cases with just volumes placed in the world) and the
differences between <volume> and <physvol>.  Try changing the material 
of a volume; try modifying the placement or rotation of a physical volume.

Use ROOT to visualize the GDML files:

run ROOT and at the prompt type:

  TGeoManager::Import("mysimplegeom.gdml");
  gGeoManager->GetTopVolume()->Draw("ogl");

or

  gGeoManager->GetVolume("vWorld")->Draw("ogl");

for the later command try using a different volume found in the geometry.

Use ROOT's PDG database to get information about particles;  GENIE uses
the same class but initializes it slightly different in order to extend
the list of known particles to include some GENIE extensions.

  TDatabasePDG* tpdg = TDatabasePDG::Instance();
  TParticlePDG* apdg = tpdg->GetParticle("pi0");
  int pdg_pi0 = apdg->PdgCode();
  cout << "pi0 is " << pdg_pi0 
       << "  pi+ is " << tpdg->GetParticle("pi+")->PdgCode() << endl;

Start to use 'genie' executable as a "super"-ROOT.  This is ROOT + loaded
GENIE libraries + letting the interpreter and the compiler know where
GENIE header files are located.

Examine myfakefluxgen.C to see how a GSimpleNtpFlux tree is defined and filled.
Run it using:

  genie -b -q myfakefluxgen.C

for the CINT interpreter, and

  genie -b -q myfakefluxgen.C+

to compile, load and run it as a shared library.

Examine the flux distributions using ROOT:

  $ root myfakeflux.gsimple.root
  flux->Show(0);
  flux->Draw("vtxy:vtxx","","COLZ");
  flux->Draw("E");

Try running one of the standard genie applications 'gevgen' to generate
a ntuple of GHEP event records.  To do this you'll need a set of cross
section splines, ala:

  export GENIEXSECFILE=/path/to/gxspl.xml

The 'gevgen' app can take a text file of value pairs to define a flux.  One
random example can be found as 'flux.vec'.  Modify that to your own definition.

Run the executable for 100 event, run # 42, nu_mu w/ energy in the range
0.5 to 6GeV, in the above flux profile off Water + Fe56 nuclei.

  gevgen -n 100 -r 42 -p 14 -e 0.5,6.0 -f flux.vec \
     -t 1000260560[0.82],1000080160[0.16],1000010010[0.02] \
     --cross-sections $GENIEXSECFILE

This will print out lots of text including a dump of each generated event.
Examine the event structure.

Run again with more events and less screen output:

  gevgen -n 100 -r 42 -p 14 -e 0.5,6.0 -f flux.vec \
     -t 1000260560[0.82],1000080160[0.16],1000010010[0.02] \
     --cross-sections $GENIEXSECFILE \
     --event-record-print-level 0 \
     --message-thresholds Messenger_laconic.xml

Browse the resulting file using ROOT/genie

  $ genie gntp.42.ghep.root
  TBrowser tb; // double click on 'genv' and 'gconfig' object
  .q

Start a new interactive 'genie' session to explore how to drill down into
the stored GHEP events:

  $ genie
  genie::PDGLibrary::Instance();  // this initialize the TDatabasePDG
                                  // with GENIE extensions; doing it early
                                  // means that output won't clutter results
  TFile* myfile = TFle::Open("gntp.42.ghep.root"); 
  TTree* mytree = 0;
  myfile->GetObject("gtree",mytree);

  Long64_t nEntries = mytree->GetEntries();
  cout << nEntries << endl;

  genie::NtpMCEventRecord* myentry = new genie::NtpMCEventRecord();
  mytree->SetBranchAddress("gmcrec",&myentry); // gmcrec is branch name

  mytree->GetEntry(0);  // retrieve an event

  myentry->PrintToStream(cout);  // formatted printout of the event
  
  // print one particle in the event only
  myentry->event->Particle(0)->Print(); cout << endl;

  myentry->event->XSec()  // leave off ";" to have ROOT print result value

  // vertex info is boring (x,y,z,t)=(0,0,0,0) because gevgen used
  // the PointGeometryAnalyzer and not a real geometry
  // nor did it use a flux that varied in space
  myentry->event->Vertex()->Print();


Examine the 'plot_gntp.C' script to see how it extends the concepts above
to count the number of pi0's in each event and make a plot of the E_nu 
distribution.   Run it using the command:

  $ genie 'plot_gnpt.C("gntp.42.ghep.root")'

Use the single quotes (') to hide the ()'s from the shell; use the double
quote to quote the string passed as an argument to the script.  This allows
one to use the same script for GHEP ntuple files with different names.

Build the executable for simple event generator that uses GDML geometry
and GSimpleNtpFlux:

 g++ -o gevgen_simple -DSTANDALONE -I$GENIE/src -I$ROOTSYS/include \
   `genie-config --libs` \
   `root-config --libs` -lGeom -lEGPythia6 \
   `pythia6-config --libs`
   `log4cpp-config --libs`
   `xml2-config --libs`  \
   gevgen_simple.C

Add -DMACOSX if on a Mac OS X machine.

if "log4cpp-config" isn't available use
    -L${LOG4CPP_FQ_DIR}/lib -llog4cpp -lnsl \
where $LOG4CPP_FQ_DIR is is where log4cpp is installed.

if "pythia6-config" isn't available use
    -L${PYTHIA_FQ_DIR}/lib -lPythia6
(note lack of "6" on path).

This should also run under 
   $ genie gevgen_simple.C+
but that sometimes leads to
   (1) scary warnings about hidden overloaded methods (depending on ROOT's
       build and configuration of ACLiC's warning level)
   (2) a crash after everything is done and ROOT is cleaning up and
       shutting down -- some bad interaction with AlgFactory's singleton
       cleaner and deleting algorithms.

Run it (assumes cross-sections file $GENIEXSECFILE), e.g.:

   ./gevgen_simple -n 1000

Use -h to give the command options; defaults should work based on previous 
steps.  Check "genie-mcjob-<runnum>.status" from a separate terminal to
see information about the latest event generated.  Results will be written
a file named "gntp.<runnum>.ghep.root".

   genie plot_gntp.C

---------------------------------------------------------------------------
Robert Hatcher <rhatcher@fnal.gov>  updated 2014-05-09