// Standard Library
#include <iostream>
#include <string>

#include "getopt_pp.h"

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TBranch.h"


// Global variables
TTree *tree;
TTree *reftree;
std::string treeName="SimValidation";


void showHelp() {
  std::cout << "SimulationValidationTool command line option(s) help" << std::endl;
  std::cout << "\t -i , --inputFileName <ROOT FILENAME>" << std::endl;
  std::cout << "\t -r , --referenceFileName <ROOT FILENAME>" << std::endl;
}


int main(int argc, char **argv) {
  void ParseRootFile(std::string rootFileName, std::string refFileName);
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
  return 1;
}



void ParseRootFile(std::string rootFileName, std::string refFileName) {
  void CompareHistogram(std::string branchName);

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
    rootFile->Close();
    return;
  }
    
  // Check for a reference file
  TFile *refFile;
  refFile = new TFile(refFileName.c_str());
  if (refFile->IsZombie()) {
    std::cout << "WARNING: No valid reference ROOT file given." << std::endl;
    return;
  }
  else {
    reftree = (TTree*) refFile->Get(treeName.c_str());
    // Check if it found the tree
    if (reftree==0) {
      std::cout<<"WARNING: no reference data in a tree named "<<treeName<<" found in "<<refFileName<<". To generate statistics, provide a valid reference ROOT file."<<std::endl;
      refFile->Close();
      rootFile->Close();
      return;
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
    std::string branchName=branch->GetName();
    // Call Function
    CompareHistogram(branchName); 
  }
  refFile->Close();
  rootFile->Close();
}

void CompareHistogram(std::string branchName) {
  int nbins=100;
  double lowLimit = 0;
  double highLimit = -9999;
  std::string title="";
  
  // Check whether the reference file contains this branch
  bool hasReferenceBranch = reftree->GetBranchStatus(branchName.c_str());
  if (!hasReferenceBranch) {
    std::cout<<"WARNING: branch "<<branchName<<" not found in reference file. No comparison statistics will be made for this branch"<<std::endl;
    return;
  }
  
  // Create Histograms to work with from input file
  TH1D *h = new TH1D(("plt_"+branchName).c_str(),title.c_str(),nbins,lowLimit,highLimit); // automatic limits
  if ( h->GetSumw2N() == 0 ) h->Sumw2();
  tree->Draw((branchName + ">> plt_"+branchName).c_str(),"","goff");

  // Create Histograms to work with from reference file
  TH1D *href = (TH1D*) h->Clone(); // To work with Histograms of identical size we clone h
  href->SetName(("ref_"+branchName).c_str());
  href->Reset();
  
  if( href->GetSumw2N() == 0 )href->Sumw2();
  
  //Filling
  reftree->Draw((branchName + ">> ref_"+branchName).c_str(),"","goff"); 
  
  // Normalise reference number of events to data
  double scale = (double)tree->GetEntries()/(double)reftree->GetEntries();
  href->Scale(scale);

  // Calculate Comparison Statistics
  double ks = h->KolmogorovTest(href); // Kolmogorov Test
  double chi2test = h->Chi2Test(href,"UW"); // weighted Chi2 Test p-value
  
  // Input File Data
  double std = h->GetStdDev(); // Standard Deviation
  double std_error = h->GetStdDevError(); // Error on Standard Deviation
  double skew = h->GetSkewness(); // Skewness
  double mean = h->GetMean(); // Mean
  double mean_error = h->GetMeanError(); // Error on Mean
  double max = h->GetMaximum(); // Maximum
  double min = h->GetMinimum (); // Minimum  
  
  // Reference File Data
  double std_ref = href->GetStdDev(); // Standard Deviation
  double std_error_ref = href->GetStdDevError(); // Error on Standard Deviation
  double skew_ref = href->GetSkewness(); // Skewness
  double mean_ref = href->GetMean(); // Mean
  double mean_error_ref = href->GetMeanError(); // Error on Mean
  double max_ref = href->GetMaximum(); // Maxmimum
  double min_ref = href->GetMinimum (); // Minimum
  
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
  std::cout<<"Chi2 test: "<<chi2test<<std::endl;
  std::cout<<""<<std::endl;
  
  std::cout<<"Testing branches: "<<branchName<<std::endl;
  
  // Running Tests on Comparisons
  // Both means should lie within 1 std from the other mean (h compared to href and vice versa)
  bool pass = true;
  if (mean>(mean_ref+std_ref) || mean<(mean_ref-std_ref)) {
    std::cout<<"Error: Mean outside of 1 Standard Deviation"<<std::endl;
    pass = false;
  }
  
  // Arbitrary account of the difference in std error
  if((std_error/std_error_ref)>1.01|| (std_error_ref/std_error)>1.01 || (std_error_ref/std_error)<0.99 || (std_error/std_error_ref)<0.99) {
    std::cout<<"Error: Standard Deviation Error to large"<<std::endl;
    pass = false;
  }
  
  // Mean values and Errors on Mean Values
  if(mean>(mean_ref+mean_error_ref) || mean<(mean_ref-mean_error_ref) || mean_ref>(mean+mean_error) || mean_ref<(mean-mean_error)) {
    std::cout<<"Error: Mean Value outside error bounds"<<std::endl;
    pass = false;
  }
    
  // Simple Tests on Max and Min
  if(max<min_ref || max_ref<min) {
    std::cout<<"Error: Max, Min reversed"<<std::endl;
    pass = false;
  }
  
  if (pass) {
    std::cout<<"All Tests Passed"<<std::endl;
  }
    
  std::cout<<"---- "<<"Finished working with branches: "<<branchName<<" ----"<<std::endl;
  std::cout<<""<<std::endl;
  
  delete href;
  delete h;
}


