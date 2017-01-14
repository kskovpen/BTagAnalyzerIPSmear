#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <iostream>

#include <boost/bind.hpp>

#include "RecoBTag/IPSmear/interface/IPSmear.h"

void IPSmear::init(bool isdata,bool applysmearing)
{
   TString base = TString( getenv ("CMSSW_BASE") );

   isData = isdata;
   applySmearing = applysmearing;
   
   ran = new TRandom3();

   if( applySmearing )
     {	
	fcorDxyErr = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDxyErr.root","READ");
	fcorDzErr = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDzErr.root","READ");

	fcorDxyBin0 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDxyBin0.root","READ");
	fcorDxyBin1 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDxyBin1.root","READ");
	fcorDxyBin2 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDxyBin2.root","READ");
	fcorDxyBin3 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDxyBin3.root","READ");
	fcorDxyBin4 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDxyBin4.root","READ");
	
	fcorDzBin0 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDzBin0.root","READ");
	fcorDzBin1 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDzBin1.root","READ");
	fcorDzBin2 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDzBin2.root","READ");
	fcorDzBin3 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDzBin3.root","READ");
	fcorDzBin4 = new TFile(base+"/src/RecoBTag/IPSmear/data/outputDzBin4.root","READ");
	
	for(int icat=0;icat<10;icat++)
	  {
	     std::string hnameDxyErrMC = "HistMCCat"+std::string(Form("%d",icat));
	     histDxyErrMC[icat] = (TH1D*)fcorDxyErr->Get(hnameDxyErrMC.c_str());
	     
	     std::string hnameDxyErrData = "HistDataCat"+std::string(Form("%d",icat));
	     histDxyErrData[icat] = (TH1D*)fcorDxyErr->Get(hnameDxyErrData.c_str());
	     
	     std::string hnameDzErrMC = "HistMCCat"+std::string(Form("%d",icat));
	     histDzErrMC[icat] = (TH1D*)fcorDzErr->Get(hnameDzErrMC.c_str());
	     
	     std::string hnameDzErrData = "HistDataCat"+std::string(Form("%d",icat));
	     histDzErrData[icat] = (TH1D*)fcorDzErr->Get(hnameDzErrData.c_str());
	     
	     // Dxy
	     std::string hnameDxyMC = "FuncMCCat"+std::string(Form("%d",icat));
	     TF1* FuncDxyMC[5];
	     FuncDxyMC[0] = (TF1*)fcorDxyBin0->Get(hnameDxyMC.c_str());
	     FuncDxyMC[1] = (TF1*)fcorDxyBin1->Get(hnameDxyMC.c_str());
	     FuncDxyMC[2] = (TF1*)fcorDxyBin2->Get(hnameDxyMC.c_str());
	     FuncDxyMC[3] = (TF1*)fcorDxyBin3->Get(hnameDxyMC.c_str());
	     FuncDxyMC[4] = (TF1*)fcorDxyBin4->Get(hnameDxyMC.c_str());
	     
	     std::string hnameDxyData = "FuncDataCat"+std::string(Form("%d",icat));
	     TF1* FuncDxyData[5];
	     FuncDxyData[0] = (TF1*)fcorDxyBin0->Get(hnameDxyData.c_str());
	     FuncDxyData[1] = (TF1*)fcorDxyBin1->Get(hnameDxyData.c_str());
	     FuncDxyData[2] = (TF1*)fcorDxyBin2->Get(hnameDxyData.c_str());
	     FuncDxyData[3] = (TF1*)fcorDxyBin3->Get(hnameDxyData.c_str());
	     FuncDxyData[4] = (TF1*)fcorDxyBin4->Get(hnameDxyData.c_str());
	     
	     for(int ib=0;ib<5;ib++)
	       {
		  parDxyMC[ib][icat][0] = FuncDxyMC[ib]->GetParameter(0);
		  parDxyMC[ib][icat][1] = FuncDxyMC[ib]->GetParameter(1);
		  parDxyMC[ib][icat][2] = FuncDxyMC[ib]->GetParameter(2);
		  parDxyMC[ib][icat][3] = FuncDxyMC[ib]->GetParameter(3);
		  parDxyMC[ib][icat][4] = FuncDxyMC[ib]->GetParameter(4);
		  parDxyMC[ib][icat][5] = FuncDxyMC[ib]->GetParameter(5);
		  
		  parDxyData[ib][icat][0] = FuncDxyData[ib]->GetParameter(0);
		  parDxyData[ib][icat][1] = FuncDxyData[ib]->GetParameter(1);
		  parDxyData[ib][icat][2] = FuncDxyData[ib]->GetParameter(2);
		  parDxyData[ib][icat][3] = FuncDxyData[ib]->GetParameter(3);
		  parDxyData[ib][icat][4] = FuncDxyData[ib]->GetParameter(4);
		  parDxyData[ib][icat][5] = FuncDxyData[ib]->GetParameter(5);
		  
		  std::string gnameDxy = "gausDxyCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"MC1";
		  gausDxyMC[ib][icat][0] = new TF1(gnameDxy.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDxyMC[ib][icat][0]->SetParameter(0,parDxyMC[ib][icat][0]);
		  gausDxyMC[ib][icat][0]->SetParameter(1,parDxyMC[ib][icat][1]);
		  
		  gnameDxy = "gausDxyCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"MC2";
		  gausDxyMC[ib][icat][1] = new TF1(gnameDxy.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDxyMC[ib][icat][1]->SetParameter(0,parDxyMC[ib][icat][0]*parDxyMC[ib][icat][2]);
		  gausDxyMC[ib][icat][1]->SetParameter(1,parDxyMC[ib][icat][3]);
		  
		  gnameDxy = "gausDxyCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"MC3";
		  gausDxyMC[ib][icat][2] = new TF1(gnameDxy.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDxyMC[ib][icat][2]->SetParameter(0,parDxyMC[ib][icat][0]*parDxyMC[ib][icat][4]);
		  gausDxyMC[ib][icat][2]->SetParameter(1,parDxyMC[ib][icat][5]);
		  
		  gnameDxy = "gausDxyCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"Data1";
		  gausDxyData[ib][icat][0] = new TF1(gnameDxy.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDxyData[ib][icat][0]->SetParameter(0,parDxyData[ib][icat][0]);
		  gausDxyData[ib][icat][0]->SetParameter(1,parDxyData[ib][icat][1]);
		  
		  gnameDxy = "gausDxyCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"Data2";
		  gausDxyData[ib][icat][1] = new TF1(gnameDxy.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDxyData[ib][icat][1]->SetParameter(0,parDxyData[ib][icat][0]*parDxyData[ib][icat][2]);
		  gausDxyData[ib][icat][1]->SetParameter(1,parDxyData[ib][icat][3]);
		  
		  gnameDxy = "gausDxyCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"Data3";
		  gausDxyData[ib][icat][2] = new TF1(gnameDxy.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDxyData[ib][icat][2]->SetParameter(0,parDxyData[ib][icat][0]*parDxyData[ib][icat][4]);
		  gausDxyData[ib][icat][2]->SetParameter(1,parDxyData[ib][icat][5]);
	       }
	     
	     // Dz
	     std::string hnameDzMC = "FuncMCCat"+std::string(Form("%d",icat));
	     TF1* FuncDzMC[5];
	     FuncDzMC[0] = (TF1*)fcorDzBin0->Get(hnameDzMC.c_str());
	     FuncDzMC[1] = (TF1*)fcorDzBin1->Get(hnameDzMC.c_str());
	     FuncDzMC[2] = (TF1*)fcorDzBin2->Get(hnameDzMC.c_str());
	     FuncDzMC[3] = (TF1*)fcorDzBin3->Get(hnameDzMC.c_str());
	     FuncDzMC[4] = (TF1*)fcorDzBin4->Get(hnameDzMC.c_str());
	     
	     std::string hnameDzData = "FuncDataCat"+std::string(Form("%d",icat));
	     TF1* FuncDzData[5];
	     FuncDzData[0] = (TF1*)fcorDzBin0->Get(hnameDzData.c_str());
	     FuncDzData[1] = (TF1*)fcorDzBin1->Get(hnameDzData.c_str());
	     FuncDzData[2] = (TF1*)fcorDzBin2->Get(hnameDzData.c_str());
	     FuncDzData[3] = (TF1*)fcorDzBin3->Get(hnameDzData.c_str());
	     FuncDzData[4] = (TF1*)fcorDzBin4->Get(hnameDzData.c_str());
	     
	     for(int ib=0;ib<5;ib++)
	       {
		  parDzMC[ib][icat][0] = FuncDzMC[ib]->GetParameter(0);
		  parDzMC[ib][icat][1] = FuncDzMC[ib]->GetParameter(1);
		  parDzMC[ib][icat][2] = FuncDzMC[ib]->GetParameter(2);
		  parDzMC[ib][icat][3] = FuncDzMC[ib]->GetParameter(3);
		  parDzMC[ib][icat][4] = FuncDzMC[ib]->GetParameter(4);
		  parDzMC[ib][icat][5] = FuncDzMC[ib]->GetParameter(5);
		  
		  parDzData[ib][icat][0] = FuncDzData[ib]->GetParameter(0);
		  parDzData[ib][icat][1] = FuncDzData[ib]->GetParameter(1);
		  parDzData[ib][icat][2] = FuncDzData[ib]->GetParameter(2);
		  parDzData[ib][icat][3] = FuncDzData[ib]->GetParameter(3);
		  parDzData[ib][icat][4] = FuncDzData[ib]->GetParameter(4);
		  parDzData[ib][icat][5] = FuncDzData[ib]->GetParameter(5);
		  
		  std::string gnameDz = "gausDzCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"MC1";
		  gausDzMC[ib][icat][0] = new TF1(gnameDz.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDzMC[ib][icat][0]->SetParameter(0,parDzMC[ib][icat][0]);
		  gausDzMC[ib][icat][0]->SetParameter(1,parDzMC[ib][icat][1]);
		  
		  gnameDz = "gausDzCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"MC2";
		  gausDzMC[ib][icat][1] = new TF1(gnameDz.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDzMC[ib][icat][1]->SetParameter(0,parDzMC[ib][icat][0]*parDzMC[ib][icat][2]);
		  gausDzMC[ib][icat][1]->SetParameter(1,parDzMC[ib][icat][3]);
		  
		  gnameDz = "gausDzCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"MC3";
		  gausDzMC[ib][icat][2] = new TF1(gnameDz.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDzMC[ib][icat][2]->SetParameter(0,parDzMC[ib][icat][0]*parDzMC[ib][icat][4]);
		  gausDzMC[ib][icat][2]->SetParameter(1,parDzMC[ib][icat][5]);
		  
		  gnameDz = "gausDzCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"Data1";
		  gausDzData[ib][icat][0] = new TF1(gnameDz.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDzData[ib][icat][0]->SetParameter(0,parDzData[ib][icat][0]);
		  gausDzData[ib][icat][0]->SetParameter(1,parDzData[ib][icat][1]);
		  
		  gnameDz = "gausDzCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"Data2";
		  gausDzData[ib][icat][1] = new TF1(gnameDz.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDzData[ib][icat][1]->SetParameter(0,parDzData[ib][icat][0]*parDzData[ib][icat][2]);
		  gausDzData[ib][icat][1]->SetParameter(1,parDzData[ib][icat][3]);
		  
		  gnameDz = "gausDzCat"+std::string(Form("%d",icat))+"Bin"+std::string(Form("%d",ib))+"Data3";
		  gausDzData[ib][icat][2] = new TF1(gnameDz.c_str(),"[0]/sqrt(2*pi)/[1]*exp(-x*x/2/[1]/[1])");
		  gausDzData[ib][icat][2]->SetParameter(0,parDzData[ib][icat][0]*parDzData[ib][icat][4]);
		  gausDzData[ib][icat][2]->SetParameter(1,parDzData[ib][icat][5]);
	       }	     
	  }
     }   
   
   nBinsDxyErr = 5;
   
   rangeDxyErrBins[0][0] = 0.0000;
   rangeDxyErrBins[0][1] = 0.0040;
   rangeDxyErrBins[0][2] = 0.0080;
   rangeDxyErrBins[0][3] = 0.0120;
   rangeDxyErrBins[0][4] = 0.0200;
   rangeDxyErrBins[0][5] = 0.0500;

   rangeDxyErrBins[1][0] = 0.0000;
   rangeDxyErrBins[1][1] = 0.0030;
   rangeDxyErrBins[1][2] = 0.0060;
   rangeDxyErrBins[1][3] = 0.0100;
   rangeDxyErrBins[1][4] = 0.0160;
   rangeDxyErrBins[1][5] = 0.0400;
   
   rangeDxyErrBins[2][0] = 0.0000;
   rangeDxyErrBins[2][1] = 0.0030;
   rangeDxyErrBins[2][2] = 0.0050;
   rangeDxyErrBins[2][3] = 0.0080;
   rangeDxyErrBins[2][4] = 0.0120;
   rangeDxyErrBins[2][5] = 0.0250;
   
   rangeDxyErrBins[3][0] = 0.0000;
   rangeDxyErrBins[3][1] = 0.0040;
   rangeDxyErrBins[3][2] = 0.0070;
   rangeDxyErrBins[3][3] = 0.0100;
   rangeDxyErrBins[3][4] = 0.0140;
   rangeDxyErrBins[3][5] = 0.0300;
   
   rangeDxyErrBins[4][0] = 0.0000;
   rangeDxyErrBins[4][1] = 0.0070;
   rangeDxyErrBins[4][2] = 0.0120;
   rangeDxyErrBins[4][3] = 0.0160;
   rangeDxyErrBins[4][4] = 0.0210;
   rangeDxyErrBins[4][5] = 0.0500;
   
   rangeDxyErrBins[5][0] = 0.0000;
   rangeDxyErrBins[5][1] = 0.0030;
   rangeDxyErrBins[5][2] = 0.0060;
   rangeDxyErrBins[5][3] = 0.0090;
   rangeDxyErrBins[5][4] = 0.0130;
   rangeDxyErrBins[5][5] = 0.0350;
   
   rangeDxyErrBins[6][0] = 0.0000;
   rangeDxyErrBins[6][1] = 0.0012;
   rangeDxyErrBins[6][2] = 0.0015;
   rangeDxyErrBins[6][3] = 0.0020;
   rangeDxyErrBins[6][4] = 0.0025;
   rangeDxyErrBins[6][5] = 0.0070;
   
   rangeDxyErrBins[7][0] = 0.0000;
   rangeDxyErrBins[7][1] = 0.0015;
   rangeDxyErrBins[7][2] = 0.0022;
   rangeDxyErrBins[7][3] = 0.0030;
   rangeDxyErrBins[7][4] = 0.0042;
   rangeDxyErrBins[7][5] = 0.0100;
   
   rangeDxyErrBins[8][0] = 0.0000;
   rangeDxyErrBins[8][1] = 0.0020;
   rangeDxyErrBins[8][2] = 0.0040;
   rangeDxyErrBins[8][3] = 0.0070;
   rangeDxyErrBins[8][4] = 0.0110;
   rangeDxyErrBins[8][5] = 0.0250;
   
   rangeDxyErrBins[9][0] = 0.0000;
   rangeDxyErrBins[9][1] = 0.0015;
   rangeDxyErrBins[9][2] = 0.0025;
   rangeDxyErrBins[9][3] = 0.0040;
   rangeDxyErrBins[9][4] = 0.0060;
   rangeDxyErrBins[9][5] = 0.0200;

   rangeDxyBins[0][0] = 0.05;
   rangeDxyBins[0][1] = 0.12;
   rangeDxyBins[0][2] = 0.15;
   rangeDxyBins[0][3] = 0.17;
   rangeDxyBins[0][4] = 0.22;
   
   rangeDxyBins[1][0] = 0.035;
   rangeDxyBins[1][1] = 0.06;
   rangeDxyBins[1][2] = 0.08;
   rangeDxyBins[1][3] = 0.12;
   rangeDxyBins[1][4] = 0.17;
   
   rangeDxyBins[2][0] = 0.02;
   rangeDxyBins[2][1] = 0.025;
   rangeDxyBins[2][2] = 0.035;
   rangeDxyBins[2][3] = 0.05;
   rangeDxyBins[2][4] = 0.08;
   
   rangeDxyBins[3][0] = 0.025;
   rangeDxyBins[3][1] = 0.035;
   rangeDxyBins[3][2] = 0.05;
   rangeDxyBins[3][3] = 0.06;
   rangeDxyBins[3][4] = 0.08;
   
   rangeDxyBins[4][0] = 0.035;
   rangeDxyBins[4][1] = 0.05;
   rangeDxyBins[4][2] = 0.07;
   rangeDxyBins[4][3] = 0.10;
   rangeDxyBins[4][4] = 0.15;
   
   rangeDxyBins[5][0] = 0.02;
   rangeDxyBins[5][1] = 0.03;
   rangeDxyBins[5][2] = 0.06;
   rangeDxyBins[5][3] = 0.08;
   rangeDxyBins[5][4] = 0.15;
   
   rangeDxyBins[6][0] = 0.01;
   rangeDxyBins[6][1] = 0.01;
   rangeDxyBins[6][2] = 0.015;
   rangeDxyBins[6][3] = 0.02;
   rangeDxyBins[6][4] = 0.04;
   
   rangeDxyBins[7][0] = 0.01;
   rangeDxyBins[7][1] = 0.012;
   rangeDxyBins[7][2] = 0.016;
   rangeDxyBins[7][3] = 0.02;
   rangeDxyBins[7][4] = 0.04;
   
   rangeDxyBins[8][0] = 0.015;
   rangeDxyBins[8][1] = 0.02;
   rangeDxyBins[8][2] = 0.03;
   rangeDxyBins[8][3] = 0.045;
   rangeDxyBins[8][4] = 0.08;
   
   rangeDxyBins[9][0] = 0.01;
   rangeDxyBins[9][1] = 0.015;
   rangeDxyBins[9][2] = 0.025;
   rangeDxyBins[9][3] = 0.035;
   rangeDxyBins[9][4] = 0.08;
   
   nBinsDzErr = 5;
   
   rangeDzErrBins[0][0] = 0.00;
   rangeDzErrBins[0][1] = 0.01;
   rangeDzErrBins[0][2] = 0.02;
   rangeDzErrBins[0][3] = 0.04;
   rangeDzErrBins[0][4] = 0.06;
   rangeDzErrBins[0][5] = 0.15;
   
   rangeDzErrBins[1][0] = 0.000;
   rangeDzErrBins[1][1] = 0.005;
   rangeDzErrBins[1][2] = 0.010;
   rangeDzErrBins[1][3] = 0.015;
   rangeDzErrBins[1][4] = 0.022;
   rangeDzErrBins[1][5] = 0.060;
   
   rangeDzErrBins[2][0] = 0.000;
   rangeDzErrBins[2][1] = 0.004;
   rangeDzErrBins[2][2] = 0.006;
   rangeDzErrBins[2][3] = 0.008;
   rangeDzErrBins[2][4] = 0.012;
   rangeDzErrBins[2][5] = 0.020;
   
   rangeDzErrBins[3][0] = 0.000;
   rangeDzErrBins[3][1] = 0.007;
   rangeDzErrBins[3][2] = 0.010;
   rangeDzErrBins[3][3] = 0.015;
   rangeDzErrBins[3][4] = 0.020;
   rangeDzErrBins[3][5] = 0.050;
   
   rangeDzErrBins[4][0] = 0.00;
   rangeDzErrBins[4][1] = 0.02;
   rangeDzErrBins[4][2] = 0.03;
   rangeDzErrBins[4][3] = 0.04;
   rangeDzErrBins[4][4] = 0.06;
   rangeDzErrBins[4][5] = 0.15;
   
   rangeDzErrBins[5][0] = 0.000;
   rangeDzErrBins[5][1] = 0.005;
   rangeDzErrBins[5][2] = 0.010;
   rangeDzErrBins[5][3] = 0.015;
   rangeDzErrBins[5][4] = 0.025;
   rangeDzErrBins[5][5] = 0.060;
   
   rangeDzErrBins[6][0] = 0.0000;
   rangeDzErrBins[6][1] = 0.0025;
   rangeDzErrBins[6][2] = 0.0035;
   rangeDzErrBins[6][3] = 0.0045;
   rangeDzErrBins[6][4] = 0.0055;
   rangeDzErrBins[6][5] = 0.0130;
   
   rangeDzErrBins[7][0] = 0.0000;
   rangeDzErrBins[7][1] = 0.0035;
   rangeDzErrBins[7][2] = 0.0045;
   rangeDzErrBins[7][3] = 0.0060;
   rangeDzErrBins[7][4] = 0.0080;
   rangeDzErrBins[7][5] = 0.0200;
   
   rangeDzErrBins[8][0] = 0.0000;
   rangeDzErrBins[8][1] = 0.0070;
   rangeDzErrBins[8][2] = 0.0120;
   rangeDzErrBins[8][3] = 0.0170;
   rangeDzErrBins[8][4] = 0.0250;
   rangeDzErrBins[8][5] = 0.0800;
   
   rangeDzErrBins[9][0] = 0.000;
   rangeDzErrBins[9][1] = 0.003;
   rangeDzErrBins[9][2] = 0.005;
   rangeDzErrBins[9][3] = 0.008;
   rangeDzErrBins[9][4] = 0.012;
   rangeDzErrBins[9][5] = 0.040;

   rangeDzBins[0][0] = 0.07;
   rangeDzBins[0][1] = 0.14;
   rangeDzBins[0][2] = 0.20;
   rangeDzBins[0][3] = 0.25;
   rangeDzBins[0][4] = 0.40;
   
   rangeDzBins[1][0] = 0.045;
   rangeDzBins[1][1] = 0.075;
   rangeDzBins[1][2] = 0.085;
   rangeDzBins[1][3] = 0.13;
   rangeDzBins[1][4] = 0.19;
   
   rangeDzBins[2][0] = 0.025;
   rangeDzBins[2][1] = 0.030;
   rangeDzBins[2][2] = 0.040;
   rangeDzBins[2][3] = 0.06;
   rangeDzBins[2][4] = 0.065;
   
   rangeDzBins[3][0] = 0.035;
   rangeDzBins[3][1] = 0.050;
   rangeDzBins[3][2] = 0.08;
   rangeDzBins[3][3] = 0.09;
   rangeDzBins[3][4] = 0.10;
   
   rangeDzBins[4][0] = 0.07;
   rangeDzBins[4][1] = 0.12;
   rangeDzBins[4][2] = 0.15;
   rangeDzBins[4][3] = 0.25;
   rangeDzBins[4][4] = 0.35;
   
   rangeDzBins[5][0] = 0.025;
   rangeDzBins[5][1] = 0.04;
   rangeDzBins[5][2] = 0.08;
   rangeDzBins[5][3] = 0.14;
   rangeDzBins[5][4] = 0.25;
   
   rangeDzBins[6][0] = 0.015;
   rangeDzBins[6][1] = 0.020;
   rangeDzBins[6][2] = 0.025;
   rangeDzBins[6][3] = 0.022;
   rangeDzBins[6][4] = 0.035;
   
   rangeDzBins[7][0] = 0.020;
   rangeDzBins[7][1] = 0.024;
   rangeDzBins[7][2] = 0.030;
   rangeDzBins[7][3] = 0.035;
   rangeDzBins[7][4] = 0.050;
   
   rangeDzBins[8][0] = 0.030;
   rangeDzBins[8][1] = 0.040;
   rangeDzBins[8][2] = 0.060;
   rangeDzBins[8][3] = 0.080;
   rangeDzBins[8][4] = 0.120;
   
   rangeDzBins[9][0] = 0.02;
   rangeDzBins[9][1] = 0.04;
   rangeDzBins[9][2] = 0.05;
   rangeDzBins[9][3] = 0.08;
   rangeDzBins[9][4] = 0.15;
   
   if( applySmearing && isData )
     {
	for(int icat=0;icat<10;icat++)
	  {
	     for(int i=0;i<=nBinsDxyErr;i++)
	       {
		  float errNew = (i > 0) ? corDxyErr(rangeDxyErrBins[icat][i],icat) : 0.;
		  rangeDxyErrBinsCor[icat][i] = errNew;
	       }	     
	     
	     for(int i=0;i<=nBinsDzErr;i++)
	       {
		  float errNew = (i > 0) ? corDzErr(rangeDzErrBins[icat][i],icat) : 0.;
		  rangeDzErrBinsCor[icat][i] = errNew;
	       }	     
	  }	
     }
   
   std::cout << "Initialization of IPSmear done" << std::endl;
}

std::vector<double> IPSmear::smear(double chi2,double eta,int npix,double p,double pt,int nstrip,double dxy,double dz,double dist,double length,double dxyErr,double dzErr)
{
   std::vector<double> res;
   
//   res.push_back(dxy);
//   res.push_back(dxyErr);
//   res.push_back(dz);
//   res.push_back(dzErr);
   
//   bool pass = trackPass(eta,pt,chi2,npix,(npix+nstrip),
//			 dxy,dz,dist,length);
   
//   if( !pass ) return res;
   
//   res.clear();
   
   int icat = trackCategory(chi2,eta,npix,p);

   double Dxy = dxy;
   double Dz = dz;
   double DzCor = Dz;
   double DxyErr = dxyErr;
   double DzErr = dzErr;
   double DxyCor = Dxy;
   double DxyErrCor = DxyErr;
   double DzErrCor = DzErr;
      
   if( !isData && applySmearing )
     {
	// Dxy
	
	int binIdxDxy = findBinDxy(DxyErr,icat);
	
	double sigDxy1 = parDxyData[binIdxDxy][icat][1]*parDxyData[binIdxDxy][icat][1]-parDxyMC[binIdxDxy][icat][1]*parDxyData[binIdxDxy][icat][1]-parDxyMC[binIdxDxy][icat][1]*parDxyMC[binIdxDxy][icat][1];
	double sigDxy2 = parDxyData[binIdxDxy][icat][3]*parDxyData[binIdxDxy][icat][3]-parDxyMC[binIdxDxy][icat][3]*parDxyData[binIdxDxy][icat][3]-parDxyMC[binIdxDxy][icat][3]*parDxyMC[binIdxDxy][icat][3];
	double sigDxy3 = parDxyData[binIdxDxy][icat][5]*parDxyData[binIdxDxy][icat][5]-parDxyMC[binIdxDxy][icat][5]*parDxyData[binIdxDxy][icat][5]-parDxyMC[binIdxDxy][icat][5]*parDxyMC[binIdxDxy][icat][5];
	
	if( sigDxy1 < 0 ) sigDxy1 = 0.;
	if( sigDxy2 < 0 ) sigDxy2 = 0.;
	if( sigDxy3 < 0 ) sigDxy3 = 0.;
	
	sigDxy1 = sqrt(sigDxy1);
	double rDxy1 = ran->Gaus(0.0,sigDxy1);
	if( sigDxy1 <= 0 ) rDxy1 = 0;
	sigDxy2 = sqrt(sigDxy2);
	double rDxy2 = ran->Gaus(0.0,sigDxy2);
	if( sigDxy2 <= 0 ) rDxy2 = 0;
	sigDxy3 = sqrt(sigDxy3);
	double rDxy3 = ran->Gaus(0.0,sigDxy3);
	if( sigDxy3 <= 0 ) rDxy3 = 0;
	
	long double vDxy1 = gausDxyData[binIdxDxy][icat][0]->Eval(0.);
	long double vDxy2 = gausDxyData[binIdxDxy][icat][1]->Eval(0.);
	long double vDxy3 = gausDxyData[binIdxDxy][icat][2]->Eval(0.);
	long double frDxy1 = vDxy1/(vDxy1+vDxy2+vDxy3);
	long double frDxy2 = vDxy2/(vDxy1+vDxy2+vDxy3);
	long double frDxy3 = vDxy3/(vDxy1+vDxy2+vDxy3);
	
	std::vector<std::pair<int,long double> > frDxy;
	frDxy.push_back(std::make_pair(0,frDxy1));
	frDxy.push_back(std::make_pair(1,frDxy2));
	frDxy.push_back(std::make_pair(2,frDxy3));
	
	std::sort(frDxy.begin(),frDxy.end(),
		  boost::bind(&std::pair<int,long double>::second,_1) <
		  boost::bind(&std::pair<int,long double>::second,_2));
	
	double sigDxy = 0.;
	double rDxy = ran->Rndm();
	
	if( rDxy < frDxy.at(0).second )
	  {
	     if( frDxy.at(0).first == 0 ) sigDxy = rDxy1;
	     else if( frDxy.at(0).first == 1 ) sigDxy = rDxy2;
	     else sigDxy = rDxy3;
	  }
	else if( rDxy >= frDxy.at(0).second && rDxy < frDxy.at(0).second+frDxy.at(1).second )
	  {
	     if( frDxy.at(1).first == 0 ) sigDxy = rDxy1;
	     else if( frDxy.at(1).first == 1 ) sigDxy = rDxy2;
	     else sigDxy = rDxy3;
	  }
	else
	  {
	     if( frDxy.at(2).first == 0 ) sigDxy = rDxy1;
	     else if( frDxy.at(2).first == 1 ) sigDxy = rDxy2;
	     else sigDxy = rDxy3;
	  }
	
	DxyCor += sigDxy;
//	if( DxyCor > 0 ) DxyCor = -DxyCor;

	DxyErrCor = corDxyErr(DxyErr,icat);
	
	DzErrCor = corDzErr(DzErr,icat);
	
	// Dz
	
	int binIdxDz = findBinDz(DzErr,icat);
	
	double sigDz1 = parDzData[binIdxDz][icat][1]*parDzData[binIdxDz][icat][1]-parDzMC[binIdxDz][icat][1]*parDzData[binIdxDz][icat][1]-parDzMC[binIdxDz][icat][1]*parDzMC[binIdxDz][icat][1];
	double sigDz2 = parDzData[binIdxDz][icat][3]*parDzData[binIdxDz][icat][3]-parDzMC[binIdxDz][icat][3]*parDzData[binIdxDz][icat][3]-parDzMC[binIdxDz][icat][3]*parDzMC[binIdxDz][icat][3];
	double sigDz3 = parDzData[binIdxDz][icat][5]*parDzData[binIdxDz][icat][5]-parDzMC[binIdxDz][icat][5]*parDzData[binIdxDz][icat][5]-parDzMC[binIdxDz][icat][5]*parDzMC[binIdxDz][icat][5];
	double shift = parDzData[binIdxDz][icat][6]-parDzMC[binIdxDz][icat][6];
	
	if( sigDz1 < 0 ) sigDz1 = 0.;
	if( sigDz2 < 0 ) sigDz2 = 0.;
	if( sigDz3 < 0 ) sigDz3 = 0.;
	
	sigDz1 = sqrt(sigDz1);
	double rDz1 = ran->Gaus(0.0,sigDz1);
	if( sigDz1 <= 0 ) rDz1 = 0;
	sigDz2 = sqrt(sigDz2);
	double rDz2 = ran->Gaus(0.0,sigDz2);
	if( sigDz2 <= 0 ) rDz2 = 0;
	sigDz3 = sqrt(sigDz3);
	double rDz3 = ran->Gaus(0.0,sigDz3);
	if( sigDz3 <= 0 ) rDz3 = 0;
	
	long double vDz1 = gausDzData[binIdxDz][icat][0]->Eval(0.);
	long double vDz2 = gausDzData[binIdxDz][icat][1]->Eval(0.);
	long double vDz3 = gausDzData[binIdxDz][icat][2]->Eval(0.);
	long double frDz1 = vDz1/(vDz1+vDz2+vDz3);
	long double frDz2 = vDz2/(vDz1+vDz2+vDz3);
	long double frDz3 = vDz3/(vDz1+vDz2+vDz3);
	
	std::vector<std::pair<int,long double> > frDz;
	frDz.push_back(std::make_pair(0,frDz1));
	frDz.push_back(std::make_pair(1,frDz2));
	frDz.push_back(std::make_pair(2,frDz3));
	
	std::sort(frDz.begin(),frDz.end(),
		  boost::bind(&std::pair<int,long double>::second,_1) <
		  boost::bind(&std::pair<int,long double>::second,_2));
	
	double sigDz = 0.;
	double rDz = ran->Rndm();
	
	if( rDz < frDz.at(0).second )
	  {	     
	     if( frDz.at(0).first == 0 ) sigDz = rDz1;
	     else if( frDz.at(0).first == 1 ) sigDz = rDz2;
	     else sigDz = rDz3;
	  }	
	else if( rDz >= frDz.at(0).second && rDz < frDz.at(0).second+frDz.at(1).second )
	  {	     
	     if( frDz.at(1).first == 0 ) sigDz = rDz1;
	     else if( frDz.at(1).first == 1 ) sigDz = rDz2;
	     else sigDz = rDz3;
	  }	
	else
	  {	     
	     if( frDz.at(2).first == 0 ) sigDz = rDz1;
	     else if( frDz.at(2).first == 1 ) sigDz = rDz2;
	     else sigDz = rDz3;
	  }

	DzCor += sigDz;
	DzCor += shift;
     }      
   
   if( icat < 0 )
     {	
	std::cout << "ERROR: Unknown track category" << std::endl;
	std::cout << "chi2=" << chi2 << std::endl;
	std::cout << "eta=" << eta << std::endl;
	std::cout << "npix=" << npix << std::endl;
	std::cout << "p=" << p << std::endl;
	exit(1);
     }
     
   res.push_back(DxyCor);
   res.push_back(DxyErrCor);
   res.push_back(DzCor);
   res.push_back(DzErrCor);
   
   return res;
}

double IPSmear::corDxyErr(double mc,int icat)
{
   double mcCor = mc;
   
   int nBinsMC = histDxyErrMC[icat]->GetXaxis()->GetNbins();
   int nBinsData = histDxyErrData[icat]->GetXaxis()->GetNbins();
   
   int iBinMC = histDxyErrMC[icat]->FindBin(mc);
   if( mc > histDxyErrMC[icat]->GetXaxis()->GetBinUpEdge(nBinsMC) ) return mc;
   
   double binValR = histDxyErrMC[icat]->GetBinContent(iBinMC);
   double binValL = (iBinMC > 1) ? histDxyErrMC[icat]->GetBinContent(iBinMC-1) : 0.;
   double binR = histDxyErrMC[icat]->GetXaxis()->GetBinUpEdge(iBinMC);
   double binL = histDxyErrMC[icat]->GetXaxis()->GetBinLowEdge(iBinMC);
   double FuncMC = (binValL*(binR-mc)+binValR*(mc-binL))/(binR-binL);
   
   int iBinData = 0;
   
   for(int ib=1;ib<nBinsData+1;ib++)
     {
	double prob = histDxyErrData[icat]->GetBinContent(ib);
	
	if( prob >= FuncMC )
	  {
	     iBinData = ib;
	     break;
	  }	
     }   
   
   binValR = histDxyErrData[icat]->GetBinContent(iBinData);
   binValL = (iBinData > 1) ? histDxyErrData[icat]->GetBinContent(iBinData-1) : 0.;
   binR = histDxyErrData[icat]->GetXaxis()->GetBinUpEdge(iBinData);
   binL = histDxyErrData[icat]->GetXaxis()->GetBinLowEdge(iBinData);
   
   mcCor = (FuncMC*(binR-binL)+binValR*binL-binValL*binR)/(binValR-binValL);
   
   return mcCor;
}

double IPSmear::corDzErr(double mc,int icat)
{
   double mcCor = mc;
   
   int nBinsMC = histDzErrMC[icat]->GetXaxis()->GetNbins();
   int nBinsData = histDzErrData[icat]->GetXaxis()->GetNbins();
   
   int iBinMC = histDzErrMC[icat]->FindBin(mc);
   if( mc > histDzErrMC[icat]->GetXaxis()->GetBinUpEdge(nBinsMC) ) return mc;
   
   double binValR = histDzErrMC[icat]->GetBinContent(iBinMC);
   double binValL = (iBinMC > 1) ? histDzErrMC[icat]->GetBinContent(iBinMC-1) : 0.;
   double binR = histDzErrMC[icat]->GetXaxis()->GetBinUpEdge(iBinMC);
   double binL = histDzErrMC[icat]->GetXaxis()->GetBinLowEdge(iBinMC);
   double FuncMC = (binValL*(binR-mc)+binValR*(mc-binL))/(binR-binL);
   
   int iBinData = 0;
   
   for(int ib=1;ib<nBinsData+1;ib++)
     {
	double prob = histDzErrData[icat]->GetBinContent(ib);
	
	if( prob >= FuncMC )
	  {
	     iBinData = ib;
	     break;
	  }	
     }   
   
   binValR = histDzErrData[icat]->GetBinContent(iBinData);
   binValL = (iBinData > 1) ? histDzErrData[icat]->GetBinContent(iBinData-1) : 0.;
   binR = histDzErrData[icat]->GetXaxis()->GetBinUpEdge(iBinData);
   binL = histDzErrData[icat]->GetXaxis()->GetBinLowEdge(iBinData);
   
   mcCor = (FuncMC*(binR-binL)+binValR*binL-binValL*binR)/(binValR-binValL);
   
   return mcCor;
}

int IPSmear::trackCategory(double chi2,double eta,int npix,double p)
{
   int cat = -1;
   
   if( fabs(eta) >= 2.5 ) return cat;
   
   if( npix == 1 ) cat = 0;
   
   if( chi2 < 2.5 )
     {
	if( npix >= 3 && p < 8 )
	  {
	     if( fabs(eta) >= 0 && fabs(eta) < 0.8 ) cat = 2;
	     else if( fabs(eta) >= 0.8 && fabs(eta) < 1.6 ) cat = 3;
	     else if( fabs(eta) >= 1.6 && fabs(eta) < 2.5 ) cat = 4;
	  }	
	else if( npix == 2 && p < 8 ) cat = 5;
	else if( npix >= 3 && p >= 8 )
	  {
	     if( fabs(eta) >= 0 && fabs(eta) < 0.8 ) cat = 6;
	     else if( fabs(eta) >= 0.8 && fabs(eta) < 1.6 ) cat = 7;
	     else if( fabs(eta) >= 1.6 && fabs(eta) < 2.5 ) cat = 8;
	  }	
	else if( npix == 2 && p >= 8 ) cat = 9;
     }   
   else if( npix >= 2 ) cat = 1;
   
   return cat;
}

bool IPSmear::trackPass(double teta,double tpt,double chi2,int npix,int npixstrip,
			double dxy,double dz,double dist,double length)
{
   bool pass = 1;
   
/*   bool passEta = (fabs(teta) < 2.5);
   bool passPt = (tpt > 1.);
   bool passChi2 = (chi2 < 5.);
   bool passNpix = (npix >= 1);
   bool passNpixstrip = (npixstrip >= 3);
   bool passDxy = (fabs(dxy) < 0.2);
   bool passDz = (fabs(dz) < 17);
   bool passDist = (fabs(dist) < 0.07);
   bool passLength = (fabs(length) < 5);
   
   pass = (passEta && passPt && passChi2 && passNpix && passNpixstrip && passDxy && passDz && passDist && passLength);*/
   
   return pass;
}

int IPSmear::findBinDxy(double val,int icat)
{   
   int ibin = -1;
   
   for(int ib=0;ib<nBinsDxyErr;ib++)
     {	
	if( (val >= rangeDxyErrBinsCor[icat][ib] && val < rangeDxyErrBinsCor[icat][ib+1] && isData) ||
	    (val >= rangeDxyErrBins[icat][ib] && val < rangeDxyErrBins[icat][ib+1] && !isData) )
	  {	     
	     ibin = ib;
	     break;
	  }	
     }
              
   if( (val >= rangeDxyErrBinsCor[icat][nBinsDxyErr] && isData) ||
       (val >= rangeDxyErrBins[icat][nBinsDxyErr] && !isData) ) ibin = nBinsDxyErr-1;
   
   if( (val < rangeDxyErrBinsCor[icat][0] && isData) ||
       (val < rangeDxyErrBins[icat][0] && !isData) ) ibin = 0;
   
   return ibin;
}

int IPSmear::findBinDz(double val,int icat)
{   
   int ibin = -1;
   
   for(int ib=0;ib<nBinsDzErr;ib++)
     {
	if( (val >= rangeDzErrBinsCor[icat][ib] && val < rangeDzErrBinsCor[icat][ib+1] && isData) ||
	    (val >= rangeDzErrBins[icat][ib] && val < rangeDzErrBins[icat][ib+1] && !isData) )
	  {	     
	     ibin = ib;
	     break;
	  }	
     }
      
   if( (val >= rangeDzErrBinsCor[icat][nBinsDzErr] && isData) ||
       (val >= rangeDzErrBins[icat][nBinsDzErr] && !isData) ) ibin = nBinsDzErr-1;
   
   if( (val < rangeDzErrBinsCor[icat][0] && isData) ||
       (val < rangeDzErrBins[icat][0] && !isData) ) ibin = 0;
   
   return ibin;
}
