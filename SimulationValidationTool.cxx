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

// ROOT includes
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

// Global variables
bool hasConfig=true;
bool hasValidReference = true;
TTree *tree;
TTree *reftree;
std::string treeName="SimValidation";


void showHelp() {
  std::cout << "SimulationValidationTool command line option(s) help" << std::endl;
  std::cout << "\t -i , --inputFileName <ROOT FILENAME>" << std::endl;
  std::cout << "\t -r , --referenceFileName <ROOT FILENAME>" << std::endl;
}


int main(int argc, char **argv) {

  std::string inputFileName;
  std::string refFileName;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }
  
  ops >> GetOpt::Option('i', "inputFile", inputFileName, "");
  ops >> GetOpt::Option('r', "refFile", refFileName, "");

  if (inputFileName.empty() || refFileName.empty()) {
    std::cout << "Missing file name input." << std::endl;
    showHelp();
    return 0;
  }
  
  // Call Function
  ParseRootFile(inputFileName, refFileName);
}



void ParseRootFile(string rootFileName, string refFileName) {
  // Check the input root file can be opened and contains a tree with the right name
  std::cout<<"Processing "<<rootFileName<<std::endl;
  TFile *rootFile;
  rootFile = new TFile(rootFileName.c_str());
  if (rootFile->IsZombie()) {
    std::cout<<"Error: file "<<rootFileName<<" not found"<<std::endl;
    return;
  }
  
  tree = (TTree*) rootFile->Get(treeName.c_str());
  
  // Check if it found the tree
  if (tree==0) {
    std::cout<<"Error: no data in a tree named "<<treeName<<std::endl;
    return;
  }
    
  // Check for a reference file
  TFile *refFile;
  if (refFileName.length() > 0) {
    refFile = new TFile(refFileName.c_str());
    if (refFile->IsZombie()) {
      std::cout<<"WARNING: No valid reference ROOT file given";
      if (refFileName.length()>0) cout<<" Bad ROOT file: "<<refFileName;
      std::cout <<std::endl;
      hasValidReference = false;
    }
  }
  if (hasValidReference) {
    reftree = (TTree*) refFile->Get(treeName.c_str());
    // Check if it found the tree
    if (reftree==0) {
      std::cout<<"WARNING: no reference data in a tree named "<<treeName<<" found in "<<refFileName<<". To generate statistics, provide a valid reference ROOT file."<<std::endl;
      hasValidReference = false;
    }
  }
    
  // Get a list of all the branches in the main tree
  TObjArray* branches = tree->GetListOfBranches();
  TIter briter(branches);
  TBranch *branch;
  
  std::cout<<""<<std::endl;
  std::cout<<"Statistics on branches"<<std::endl;
  std::cout<<""<<std::endl;
  
  // Loop through Branches
  while( (branch=(TBranch *)briter.Next() )) {
    string branchName=branch->GetName();
    // Call Function
    CompareHistogram(branchName); 
  }
}

void CompareHistogram(string branchName) {
  int notSetVal=-9999;
  string config="";
  int nbins=100;
  double lowLimit=0;
  double highLimit=notSetVal;
  string title="";
  bool hasReferenceBranch=hasValidReference;
  
  // Check whether the reference file contains this branch
  if (hasValidReference) {
    hasReferenceBranch=reftree->GetBranchStatus(branchName.c_str());
    if (!hasReferenceBranch) cout<<"WARNING: branch "<<branchName<<" not found in reference file. No comparison statistics will be made for this branch"<<endl;
  }
  
  // Read the config information
  if (config.length()>0) {
    // title, nbins, low limit, high limit separated by commas
    title=GetBitBeforeComma(config); // config now has this bit chopped off ready for the next parsing stage

    // Number of bins
    try {
      string nbinString=GetBitBeforeComma(config);
      std::string::size_type sz;   // alias of size_t
      nbins = std::stoi (nbinString,&sz); // hopefully the next chunk is turnable into an integer
    }
    catch (exception &e) {
      nbins=0;
    }
    
    // Low bin limit
    try {
      string lowString=GetBitBeforeComma(config);
      lowLimit = std::stod (lowString); // hopefully the next chunk is turnable into an double
    }
    catch (exception &e) {
      lowLimit=0;
    }
    
    // High bin limit
    try {
      string highString=GetBitBeforeComma(config);
      highLimit = std::stod (highString); // hopefully the next chunk is turnable into an double
    }
    catch (exception &e) {
      highLimit=notSetVal;
    }
  }
  
  // Create Histograms to work with from input file
  TCanvas *c = new TCanvas (("plot_"+branchName).c_str(),("plot_"+branchName).c_str(),900,600);
  TH1D *h;
  h = new TH1D(("plt_"+branchName).c_str(),title.c_str(),nbins,lowLimit,highLimit);
  if( h->GetSumw2N() == 0 )h->Sumw2();
  tree->Draw((branchName + ">> plt_"+branchName).c_str());

  // Create Histograms to work with from reference file
  TH1D *href = (TH1D*) h->Clone(); // To work with Histograms of identical size we clone h
  href->SetName(("ref_"+branchName).c_str());
  href->Reset();
  
  if( href->GetSumw2N() == 0 )href->Sumw2();
  
  //Filling
  reftree->Draw((branchName + ">> ref_"+branchName).c_str());
   
  // Normalise reference number of events to data
  Double_t scale = (double)tree->GetEntries()/(double)reftree->GetEntries();
  href->Scale(scale);

  // Calculate Comparison Statistics
  Double_t ks = h->KolmogorovTest(href); // Kolmogorov Test
  
  // Input File Data
  Double_t std = h->GetStdDev(); // Standard Deviation
  Double_t std_error = h->GetStdDevError(); // Error on Standard Deviation
  Double_t skew = h->GetSkewness(); // Skewness
  Double_t mean = h->GetMean(); // Mean
  Double_t mean_error = h->GetMeanError(); // Error on Mean
  Double_t max = h->GetMaximum(); // Maximum
  Double_t min = h->GetMinimum (); // Minimum  
  
  // Reference File Data
  Double_t std_ref = href->GetStdDev(); // Standard Deviation
  Double_t std_error_ref = href->GetStdDevError(); // Error on Standard Deviation
  Double_t skew_ref = href->GetSkewness(); // Skewness
  Double_t mean_ref = href->GetMean(); // Mean
  Double_t mean_error_ref = href->GetMeanError(); // Error on Mean
  Double_t max_ref = href->GetMaximum(); // Maxmimum
  Double_t min_ref = href->GetMinimum (); // Minimum
  
  Double_t chisq;
  Int_t ndf;
  Double_t p_value = ChiSquared(h, href, chisq, ndf, false);
  
  std::cout<<"Comparing branches: "<<branchName<<std::endl;
  std::cout<<""<<std::endl;
  std::cout<<"Mean: "<<mean<<" ; Reference Mean:"<<mean_ref<<std::endl;
  std::cout<<"Mean Error: "<<mean_error<<" ; Reference Mean Error:"<<mean_error_ref<<std::endl;
  std::cout<<"Maximum: "<<max<<" ; Reference Maximum:"<<max_ref<<std::endl;
  std::cout<<"Minimum: "<<min<<" ; Reference Minimum:"<<min_ref<<std::endl;
  std::cout<<"Skewness: "<<skew<<" ; Reference Skewness:"<<skew_ref<<std::endl;
  std::cout<<"Std: "<<std<<" ; Reference Std:"<<std_ref<<std::endl;
  std::cout<<"Std Error: "<<std_error<<" ; Reference Std Error:"<<std_error_ref<<std::endl;
  std::cout<<"Kolmogorov: "<<ks<<std::endl;
  std::cout<<"P-value: "<<p_value<<std::endl;
  std::cout<<"Chi-square: "<<chisq<<std::endl;
  std::cout<<"DoF: "<<chisq/(double)ndf<<std::endl;
  std::cout<<""<<std::endl;
  
  std::cout<<"Testing branches: "<<branchName<<std::endl;
  
  // Running Tests on Comparisons
  if (mean>(mean+std_ref) || mean<(mean-std_ref) || (std_error/std_error_ref)>std_ref || (std_error_ref/std_error)>std_ref || (std_error_ref/std_error)>std || (std_error/std_error_ref)>std || 
  mean>(mean_ref+mean_error_ref) || mean<(mean_ref-mean_error_ref) || mean_ref>(mean+mean_error) || mean_ref<(mean-mean_error)  || max<min_ref || max_ref<min || p_value<0.99) {
    
    // Both means should lie within 1 std from the other mean (h compared to href and vice versa)
    if (mean>(mean_ref+std_ref) || mean<(mean_ref-std_ref)) {
      std::cout<<"Error: Mean outside of 1 Standard Deviation"<<std::endl;
    }
    
    // Arbitrary account of the difference in std error
    if((std_error/std_error_ref)>1.01|| (std_error_ref/std_error)>1.01 || (std_error_ref/std_error)<0.99 || (std_error/std_error_ref)<0.99) {
      std::cout<<"Error: Standard Deviation Error to large"<<std::endl;
    }
    
    // Mean values and Errors on Mean Values
    if(mean>(mean_ref+mean_error_ref) || mean<(mean_ref-mean_error_ref) || mean_ref>(mean+mean_error) || mean_ref<(mean-mean_error)) {
      std::cout<<"Error: Mean Value outside error bounds"<<std::endl;
    }
    
    // Simple Tests on Max and Min
    if(max<min_ref || max_ref<min) {
      std::cout<<"Error: Max, Min reversed"<<std::endl;
    }
    
    // Arbitrary P-value test
    if(p_value<0.99) {
      std::cout<<"Error: P-value less than 0.99"<<std::endl;
    }
  }
  else {
    std::cout<<"All Tests Passed"<<std::endl;
  }
  
  std::cout<<"---- "<<"Finished working with branches: "<<branchName<<" ----"<<std::endl;
  std::cout<<""<<std::endl;
  
  delete href;
  delete h;
}

string GetBitBeforeComma(string& input) {
  string output;
  int pos=input.find_first_of(',');
  if (pos <=0) {
    boost::trim(input);
    output=input;
    input="";
  }
  else {
    output=input.substr(0,pos);
    boost::trim(output);
    input=input.substr(pos+1);
  }
  return output;
}

double ChiSquared(TH1 *h1, TH1 *h2, double &chisq, int &ndf, bool isAverage) {
    // Calculate a chi squared per degree of freedom
    chisq=0;
    ndf=0;
    
    for (int x=1; x<=h1->GetNbinsX();x++) {
        for (int y=1; y<=h1->GetNbinsY();y++) {
            double val1 = h1->GetBinContent(x,y);
            double  val2 = h2->GetBinContent(x,y);
            double err1 = h1->GetBinError(x,y);
            double err2 = h2->GetBinError(x,y);
            // We won't count the bin (or increase the degrees of freedom) if we don't have enough info
            // This is the case if either uncertainty is zero or  if a value or uncertainty is not a number
            if ((std::isnan(val1) || std::isnan(val2) || std::isnan(err1) || std::isnan(err2))) {
                // cout<<"Insufficient information for bin  ("<<x<<","<<y<<"): do not include in chi square calculation"<<endl;
                continue;
            }
            
            if (err1 == 0 || err2 == 0) { // Should never be the case!
                //cout<<"Insufficient information for bin  ("<<x<<","<<y<<"): do not include in chi square calculation"<<endl;
                continue;
            }
            
            ndf++;
            double numerator = pow((val1 - val2), 2);
            double denominator = pow(err1,2) + pow(err2,2);
            chisq += numerator/denominator;
        }
    }
    return TMath::Prob(chisq, ndf);
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

string BranchNameToEnglish(string branchname) {
    int pos = branchname.find_first_of("_");
    int initpos=pos;
    while (pos >=0) {
        branchname.replace(pos,1," ");
        pos = branchname.find_first_of("_");
    }
    string output = branchname.substr(initpos+1,branchname.length());
    output[0]=toupper(output[0]);
    return output;
}
