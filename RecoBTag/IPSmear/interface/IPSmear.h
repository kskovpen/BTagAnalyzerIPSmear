#ifndef RecoBTag_IPSmear_IPSmear_h
#define RecoBTag_IPSmear_IPSmear_h

#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TF1.h"
#include "TRandom3.h"

class IPSmear
{
   
 public:
   
   IPSmear() {};
   virtual ~IPSmear() {};
   
   void init(bool isData,bool applySmearing);

   std::vector<double> smear(double chi2,double eta,int npix,double p,double pt,int nstrip,double dxy,double dz,double dist,double length,double dxyErr,double dzErr);
   
 private:
   
   TFile *fcorDxyErr;
   TFile *fcorDzErr;

   TFile *fcorDxyBin0;
   TFile *fcorDxyBin1;
   TFile *fcorDxyBin2;
   TFile *fcorDxyBin3;
   TFile *fcorDxyBin4;
   
   TFile *fcorDzBin0;
   TFile *fcorDzBin1;
   TFile *fcorDzBin2;
   TFile *fcorDzBin3;
   TFile *fcorDzBin4;
   
   bool isData;
   bool applySmearing;
   
   bool trackPass(double teta,double tpt,double chi2,int npix,int npixstrip,
		  double dxy,double dz,double dist,double length);
   
   int trackCategory(double chi2,double eta,int npix,double p);

   double corDxyErr(double mc,int icat);
   
   double corDzErr(double mc,int icat);
   
   int findBinDxy(double val,int icat);
   
   int findBinDz(double val,int icat);
   
   TH1D *histDxyErrMC[10];
   TH1D *histDxyErrData[10];
   
   TH1D *histDzErrMC[10];
   TH1D *histDzErrData[10];

   double parDxyMC[5][10][10];
   double parDxyData[5][10][10];
   
   double parDzMC[5][10][10];
   double parDzData[5][10][10];
   
   int nBinsDxyErr;
   int nBinsDzErr;
   
   double rangeDxyErrBins[10][100];
   double rangeDxyErrBinsCor[10][100];
   
   double rangeDzErrBins[10][100];
   double rangeDzErrBinsCor[10][100];
   
   double rangeDxyBins[10][100];
   
   double rangeDzBins[10][100];
   
   TRandom3 *ran;
   
   TF1 *gausDzMC[5][10][3];
   TF1 *gausDzData[5][10][3];
   
   TF1 *funcDxy2DxyErrMC[10];
   TF1 *funcDxy2DxyErrData[10];
   
   TF1 *gausDxyMC[5][10][3];
   TF1 *gausDxyData[5][10][3];
};

#endif
