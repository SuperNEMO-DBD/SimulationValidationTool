// Standard Library
#include <iostream>
#include <fstream>
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TText.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TVector3.h"
#include "TDictionary.h"
#include "TBranch.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TLatex.h"
#include "TF1.h"


using namespace std;

string treeName="SimValidation";

int main(int argc, char **argv);
void ParseRootFile(string rootFileName, string refFileName);
string GetBitBeforeComma(string& input);
void CompareHistogram(string branchName);

string BranchNameToEnglish(string branchname);
string exec(const char* cmd);
double ChiSquared(TH1 *h1, TH1 *h2, double &chisq, int &ndf, bool isAverage);
