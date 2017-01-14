#include <cmath>

#include "S8Solver.h"
#include "S8AnalyticSolver.h"
#include "System8Solver.h"
#include "S8FitSolver.h"
#include "System8Solver.h"

#include "TH1.h"
#include "TF1.h"
#include "TAxis.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TMultiGraph.h"
#include "TArrayD.h"
#include "TLegend.h"

#include <iomanip>
#include <iostream>
#include <iterator>
#include <fstream>
#include <memory>
#include <sstream>

ClassImp(S8Solver)

using std::auto_ptr;
using std::cout;
using std::cerr;
using std::endl;
using std::fixed;
using std::left;
using std::make_pair;
using std::pair;
using std::right;
using std::setw;
using std::setfill;
using std::setprecision;

S8Solver::S8Solver():
    _doBinnedSolution(true),
    _doAverageSolution(true),
    _firstBin(-1),
    _lastBin(-1)
{
    fAlphaConst = true;
    fBetaConst = false;
    fcategory = "pT";
    fminPtrel = 0.8;
    fMaxPtrel = 3.0;//-1;
    fVerbose = false;
    fmethod = "numeric";
    frebin = false;
    fnbins = -1;
    frecalculateFactors = true;
    fthename = "";
    fAlphaf = 1.;
    fBetaf = 1.;
    fKappabf = 1.;
    fKappaclf = 1.;
    fDeltaf = 1.;
    fGammaf = 1.;
    fKappabConst = false;
    fKappaclConst = true;
    fisCorrFile = true;
    fDeltaConst = false;
    fGammaConst = true;
    fusemctrue = false;
    fMCtruthAwayTag = false;

    for(int i = 0; 8 > i; ++i)
        *(_averageResults + i) = 0;
}

S8Solver::~S8Solver()
{
    Clear();
}

void S8Solver::Clear()
{
    if (fh_alpha) delete fh_alpha;
    if (fh_beta) delete fh_beta;
    if (fh_kb) delete fh_kb;
    if (fh_kcl) delete fh_kcl;
    if (fh_delta) delete fh_delta;
    if (fh_gamma) delete fh_gamma;
    if (feffTag_b) delete feffTag_b;
    if (feffTag_cl) delete feffTag_cl;
    if (feffmu_b) delete feffmu_b;
    if (feffmu_cl) delete feffmu_cl;
}

void S8Solver::LoadHistos()
{
    // Load Solver Input plots
    //
    finputFile->cd();

    //  for analyzer
    fnHistoBase = (TH2F*) gDirectory->Get("lepton_in_jet/n_"+fcategory);
    fpHistoBase = (TH2F*) gDirectory->Get("lepton_in_jet/p_"+fcategory);
    fnSvxHistoBase = (TH2F*) gDirectory->Get("lepton_in_jet/ntag_"+fcategory);
    fpSvxHistoBase = (TH2F*) gDirectory->Get("lepton_in_jet/ptag_"+fcategory);

    // get bin for pTrel cut
    int ith_ptrel_bin = (int) fnHistoBase->GetYaxis()->FindBin(fminPtrel);
    int ith_max_bin = -1;
    if (fMaxPtrel != -1) ith_max_bin = (int) fnHistoBase->GetYaxis()->FindBin(fMaxPtrel);
    std::cout << " Max. ptrel= " << fMaxPtrel << ", bin = " << ith_max_bin << std::endl;

    fnHisto = (TH1D*) fnHistoBase->ProjectionX("fnHisto",-1,ith_max_bin,"e");
    fpHisto = (TH1D*) fpHistoBase->ProjectionX("fpHisto",-1,ith_max_bin,"e");
    fnHistoMu = (TH1D*) fnHistoBase->ProjectionX("fnHistoMu",ith_ptrel_bin,ith_max_bin,"e");
    fpHistoMu = (TH1D*) fpHistoBase->ProjectionX("fpHistoMu",ith_ptrel_bin,ith_max_bin,"e");
    fnHistoSvx = (TH1D*) fnSvxHistoBase->ProjectionX("fnHistoSvx", -1, ith_max_bin, "e");
    fpHistoSvx = (TH1D*) fpSvxHistoBase->ProjectionX("fpHistoSvx", -1, ith_max_bin, "e");
    fnHistoAll = (TH1D*) fnSvxHistoBase->ProjectionX("fnHistoAll",ith_ptrel_bin, ith_max_bin,"e");
    fpHistoAll = (TH1D*) fpSvxHistoBase->ProjectionX("fpHistoAll",ith_ptrel_bin, ith_max_bin,"e");

    // Save input plots in the SolverInput
    //
    /*
    _solverInput.n.all = fnHisto;
    _solverInput.n.mu = fnHistoMu;
    _solverInput.n.tag = fnHistoSvx;
    _solverInput.n.muTag = fnHistoAll;

    _solverInput.p.all = fpHisto;
    _solverInput.p.mu = fpHistoMu;
    _solverInput.p.tag = fpHistoSvx;
    _solverInput.p.muTag = fpHistoAll;
    */
    std::cout << " got projections" << std::endl;

    // rebin correlation factors
    const int ncorrptarray = 3;
    const int ncorretaarray = 3;
    //Double_t corrptbins[ncorrptarray] = {30., 80.,230.};
    Double_t corretabins[ncorrptarray] = {0.,1.5,2.5};

    // recalculate correlation factors
    //
    if (frecalculateFactors)
    {
        std::cout << " recalculate correlation factors " << std::endl;

        std::map< TString, TH1*> h1;
        std::map< TString, TH2*> h2;

        if (fisCorrFile)
            finputCorrFile->cd();
        else
            finputFile->cd();

        // for analyzer
        //
        h2["b_npT"] = (TH2F*) gDirectory->Get("MCTruth/n_"+fcategory+"_b");
        h2["cl_npT"] = (TH2F*) gDirectory->Get("MCTruth/n_"+fcategory+"_cl");
        h2["b_ppT"] = (TH2F*) gDirectory->Get("MCTruth/p_"+fcategory+"_b");
        h2["cl_ppT"] = (TH2F*) gDirectory->Get("MCTruth/p_"+fcategory+"_cl");
        h2["b_ncmbpT"] = (TH2F*) gDirectory->Get("MCTruth/ntag_"+fcategory+"_b");
        h2["cl_ncmbpT"] = (TH2F*) gDirectory->Get("MCTruth/ntag_"+fcategory+"_cl");
        h2["b_pcmbpT"] = (TH2F*) gDirectory->Get("MCTruth/ptag_"+fcategory+"_b");
        h2["cl_pcmbpT"] = (TH2F*) gDirectory->Get("MCTruth/ptag_"+fcategory+"_cl");

        std::cout << " got initial truth dist." << std::endl;

        halljets_b           = h2["b_npT"]->ProjectionX("halljets_b", -1 , ith_max_bin,"e");
        halljets_cl          = h2["cl_npT"]->ProjectionX("halljets_cl", -1 , ith_max_bin,"e");
        htagjets_b           = h2["b_ncmbpT"]->ProjectionX("htagjets_b", -1 , ith_max_bin,"e");
        htagjets_cl          = h2["cl_ncmbpT"]->ProjectionX("htagjets_cl", -1 , ith_max_bin,"e");
        halljets_b_ptrel     = h2["b_npT"]->ProjectionX("halljets_b_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        halljets_cl_ptrel    = h2["cl_npT"]->ProjectionX("halljets_cl_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        htagjets_b_ptrel     = h2["b_ncmbpT"]->ProjectionX("htagjets_b_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        htagjets_cl_ptrel    = h2["cl_ncmbpT"]->ProjectionX("htagjets_cl_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        halloppjets_b        = h2["b_ppT"]->ProjectionX("halloppjets_b", -1 , ith_max_bin,"e");
        halloppjets_cl       = h2["cl_ppT"]->ProjectionX("halloppjets_cl", -1 , ith_max_bin,"e");
        htagoppjets_b        = h2["b_pcmbpT"]->ProjectionX("htagoppjets_b", -1 , ith_max_bin,"e");
        htagoppjets_cl       = h2["cl_pcmbpT"]->ProjectionX("htagoppjets_cl", -1 , ith_max_bin,"e");
        halloppjets_b_ptrel  = h2["b_ppT"]->ProjectionX("halloppjets_b_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        halloppjets_cl_ptrel = h2["cl_ppT"]->ProjectionX("halloppjets_cl_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        htagoppjets_b_ptrel  = h2["b_pcmbpT"]->ProjectionX("htagoppjets_b_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        htagoppjets_cl_ptrel = h2["cl_pcmbpT"]->ProjectionX("htagoppjets_cl_ptrel", ith_ptrel_bin , ith_max_bin,"e");

        b_halljets_ptrel     = h2["b_npT"]->ProjectionX("b_halljets_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        cl_halljets_ptrel    = h2["cl_npT"]->ProjectionX("cl_halljets_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        b_halljets_tagged    = h2["b_ncmbpT"]->ProjectionX("b_halljets_tagged", -1 , ith_max_bin,"e");
        cl_halljets_tagged   = h2["cl_ncmbpT"]->ProjectionX("cl_halljets_tagged", -1 , ith_max_bin,"e");
        b_halljets_ptreltagged  = h2["b_ncmbpT"]->ProjectionX("b_halljets_ptreltagged", ith_ptrel_bin , ith_max_bin,"e");
        cl_halljets_ptreltagged = h2["cl_ncmbpT"]->ProjectionX("cl_halljets_ptreltagged", ith_ptrel_bin , ith_max_bin,"e");
        b_halloppjets_tagged    = h2["b_pcmbpT"]->ProjectionX("b_halloppjets_tagged", -1 , ith_max_bin,"e");
        cl_halloppjets_tagged   = h2["cl_pcmbpT"]->ProjectionX("cl_halloppjets_tagged", -1 , ith_max_bin,"e");
        b_halloppjets_ptrel  = h2["b_ppT"]->ProjectionX("b_halloppjets_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        cl_halloppjets_ptrel  = h2["cl_ppT"]->ProjectionX("cl_halloppjets_ptrel", ith_ptrel_bin , ith_max_bin,"e");
        b_halloppjets_ptreltagged  = h2["b_pcmbpT"]->ProjectionX("b_halloppjets_ptreltagged", ith_ptrel_bin , ith_max_bin,"e");
        cl_halloppjets_ptreltagged = h2["cl_pcmbpT"]->ProjectionX("cl_halloppjets_ptreltagged", ith_ptrel_bin , ith_max_bin,"e");
	/*
        _flavouredInput.n.b = halljets_b;
        _flavouredInput.n.tag.b = htagjets_b;
        _flavouredInput.n.mu.b = halljets_b_ptrel;
        _flavouredInput.n.muTag.b = htagjets_b_ptrel;

        _flavouredInput.n.cl = halljets_cl;
        _flavouredInput.n.tag.cl = htagjets_cl;
        _flavouredInput.n.mu.cl = halljets_cl_ptrel;
        _flavouredInput.n.muTag.cl = htagjets_cl_ptrel;

        _flavouredInput.p.b = halloppjets_b;
        _flavouredInput.p.tag.b = htagoppjets_b;
        _flavouredInput.p.mu.b = halloppjets_b_ptrel;
        _flavouredInput.p.muTag.b = htagoppjets_b_ptrel;

        _flavouredInput.p.cl = halloppjets_cl;
        _flavouredInput.p.tag.cl = htagoppjets_cl;
        _flavouredInput.p.mu.cl = halloppjets_cl_ptrel;
        _flavouredInput.p.muTag.cl = htagoppjets_cl_ptrel;
	*/
        std::cout << " got projections" << std::endl;

        if (frebin) {
            std::cout << " Rebin distributions" << std::endl;

            TH1D* tmphalljets_b           = (TH1D*) halljets_b->Rebin(fnbins,"tmphalljets_b",fxbins);
            TH1D* tmphalljets_cl          = (TH1D*) halljets_cl->Rebin(fnbins,"tmphalljets_cl",fxbins);
            TH1D* tmphtagjets_b           = (TH1D*) htagjets_b->Rebin(fnbins,"tmphtagjets_b",fxbins);
            TH1D* tmphtagjets_cl          = (TH1D*) htagjets_cl->Rebin(fnbins,"tmphtagjets_cl",fxbins);
            TH1D* tmphalljets_b_ptrel     = (TH1D*) halljets_b_ptrel->Rebin(fnbins,"tmphalljets_b_ptrel",fxbins);
            TH1D* tmphalljets_cl_ptrel    = (TH1D*) halljets_cl_ptrel->Rebin(fnbins,"tmphalljets_cl_ptrel",fxbins);
            TH1D* tmphtagjets_b_ptrel     = (TH1D*) htagjets_b_ptrel->Rebin(fnbins,"tmphtagjets_b_ptrel",fxbins);
            TH1D* tmphtagjets_cl_ptrel    = (TH1D*) htagjets_cl_ptrel->Rebin(fnbins,"tmphtagjets_cl_ptrel",fxbins);
            TH1D* tmphalloppjets_b        = (TH1D*) halloppjets_b->Rebin(fnbins,"tmphalloppjets_b",fxbins);
            TH1D* tmphalloppjets_cl       = (TH1D*) halloppjets_cl->Rebin(fnbins,"tmp halloppjets_cl",fxbins);
            TH1D* tmphtagoppjets_b        = (TH1D*) htagoppjets_b->Rebin(fnbins,"tmphtagoppjets_b",fxbins);
            TH1D* tmphtagoppjets_cl       = (TH1D*) htagoppjets_cl->Rebin(fnbins,"tmphtagoppjets_cl",fxbins);
            TH1D* tmphalloppjets_b_ptrel  = (TH1D*) halloppjets_b_ptrel->Rebin(fnbins,"tmphalloppjets_b_ptrel",fxbins);
            TH1D* tmphalloppjets_cl_ptrel = (TH1D*) halloppjets_cl_ptrel->Rebin(fnbins,"tmphalloppjets_cl_ptrel",fxbins);
            TH1D* tmphtagoppjets_b_ptrel  = (TH1D*) htagoppjets_b_ptrel->Rebin(fnbins,"tmphtagoppjets_b_ptrel",fxbins);
            TH1D* tmphtagoppjets_cl_ptrel = (TH1D*) htagoppjets_cl_ptrel->Rebin(fnbins,"tmphtagoppjets_cl_ptrel",fxbins);


            TH1D* tmpb_halljets_ptrel        = (TH1D*) b_halljets_ptrel->Rebin(fnbins,"tmpb_halljets_ptrel",fxbins);
            TH1D* tmpcl_halljets_ptrel       = (TH1D*) cl_halljets_ptrel->Rebin(fnbins,"tmpcl_halljets_ptrel",fxbins);
            TH1D* tmpb_halljets_tagged       = (TH1D*) b_halljets_tagged->Rebin(fnbins,"tmpb_halljets_tagged",fxbins);
            TH1D* tmpcl_halljets_tagged      = (TH1D*) cl_halljets_tagged->Rebin(fnbins,"tmpcl_halljets_tagged",fxbins);
            TH1D* tmpb_halljets_ptreltagged  = (TH1D*) b_halljets_ptreltagged->Rebin(fnbins,"tmpb_halljets_ptreltagged",fxbins);
            TH1D* tmpcl_halljets_ptreltagged = (TH1D*) cl_halljets_ptreltagged->Rebin(fnbins,"tmpcl_halljets_ptreltagged",fxbins);
            TH1D* tmpb_halloppjets_tagged    = (TH1D*) b_halloppjets_tagged->Rebin(fnbins,"tmpb_halloppjets_tagged",fxbins);
            TH1D* tmpcl_halloppjets_tagged   = (TH1D*) cl_halloppjets_tagged->Rebin(fnbins,"tmpcl_halloppjets_tagged",fxbins);
            TH1D* tmpb_halloppjets_ptrel     = (TH1D*) b_halloppjets_ptrel->Rebin(fnbins,"tmpb_halloppjets_ptrel",fxbins);
            TH1D* tmpcl_halloppjets_ptrel    = (TH1D*) cl_halloppjets_ptrel->Rebin(fnbins,"tmpcl_halloppjets_ptrel",fxbins);


            delete halljets_b;
            delete halljets_cl;
            delete htagjets_b;
            delete htagjets_cl;
            delete halljets_b_ptrel;
            delete halljets_cl_ptrel;
            delete htagjets_b_ptrel;
            delete htagjets_cl_ptrel;
            delete halloppjets_b;
            delete halloppjets_cl;
            delete htagoppjets_b;
            delete htagoppjets_cl;
            delete halloppjets_b_ptrel;
            delete halloppjets_cl_ptrel;
            delete htagoppjets_b_ptrel;
            delete htagoppjets_cl_ptrel;


            delete b_halljets_ptrel;
            delete cl_halljets_ptrel;
            delete b_halljets_tagged;
            delete cl_halljets_tagged;
            delete b_halljets_ptreltagged;
            delete cl_halljets_ptreltagged;
            delete b_halloppjets_tagged;
            delete cl_halloppjets_tagged;
            delete b_halloppjets_ptrel;
            delete cl_halloppjets_ptrel;


            halljets_b           =  (TH1D*) tmphalljets_b->Clone("halljets_b");
            halljets_cl          =  (TH1D*) tmphalljets_cl->Clone("halljets_cl");
            htagjets_b           =  (TH1D*) tmphtagjets_b->Clone("htagjets_b");
            htagjets_cl          =  (TH1D*) tmphtagjets_cl->Clone("htagjets_cl");
            halljets_b_ptrel     =  (TH1D*) tmphalljets_b_ptrel->Clone("halljets_b_ptrel");
            halljets_cl_ptrel    =  (TH1D*) tmphalljets_cl_ptrel->Clone("halljets_cl_ptrel");
            htagjets_b_ptrel     =  (TH1D*) tmphtagjets_b_ptrel->Clone("htagjets_b_ptrel");
            htagjets_cl_ptrel    =  (TH1D*) tmphtagjets_cl_ptrel->Clone("htagjets_cl_ptrel");
            halloppjets_b        =  (TH1D*) tmphalloppjets_b->Clone("halloppjets_b");
            halloppjets_cl       =  (TH1D*) tmphalloppjets_cl->Clone("halloppjets_cl");
            htagoppjets_b        =  (TH1D*) tmphtagoppjets_b->Clone("htagoppjets_b ");
            htagoppjets_cl       =  (TH1D*) tmphtagoppjets_cl->Clone("htagoppjets_cl");
            halloppjets_b_ptrel  =  (TH1D*) tmphalloppjets_b_ptrel->Clone("halloppjets_b_ptrel");
            halloppjets_cl_ptrel =  (TH1D*) tmphalloppjets_cl_ptrel->Clone("halloppjets_cl_ptrel");
            htagoppjets_b_ptrel  =  (TH1D*) tmphtagoppjets_b_ptrel->Clone("htagoppjets_b_ptrel");
            htagoppjets_cl_ptrel =  (TH1D*) tmphtagoppjets_cl_ptrel->Clone("htagoppjets_cl_ptrel");


            b_halljets_ptrel        = (TH1D*) tmpb_halljets_ptrel->Clone("b_halljets_ptrel");
            cl_halljets_ptrel       = (TH1D*) tmpcl_halljets_ptrel->Clone("cl_halljets_ptrel");
            b_halljets_tagged       = (TH1D*) tmpb_halljets_tagged->Clone("b_halljets_tagged");
            cl_halljets_tagged      = (TH1D*) tmpcl_halljets_tagged->Clone("cl_halljets_tagged");
            b_halljets_ptreltagged  = (TH1D*) tmpb_halljets_ptreltagged->Clone("b_halljets_ptreltagged ");
            cl_halljets_ptreltagged = (TH1D*) tmpcl_halljets_ptreltagged->Clone("cl_halljets_ptreltagged");
            b_halloppjets_tagged    = (TH1D*) tmpb_halloppjets_tagged->Clone("b_halloppjets_tagged");
            cl_halloppjets_tagged   = (TH1D*) tmpcl_halloppjets_tagged->Clone("cl_halloppjets_tagged");
            b_halloppjets_ptrel     = (TH1D*) tmpb_halloppjets_ptrel->Clone("b_halloppjets_ptrel");
            cl_halloppjets_ptrel    = (TH1D*) tmpcl_halloppjets_ptrel->Clone("cl_halloppjets_ptrel");


            TH1D* tmpfnHisto = (TH1D*) fnHisto->Rebin(fnbins,"tmpfnHisto",fxbins);
            TH1D* tmpfpHisto = (TH1D*) fpHisto->Rebin(fnbins,"tmpfpHisto",fxbins);
            TH1D* tmpfnHistoMu = (TH1D*) fnHistoMu->Rebin(fnbins,"tmpfnHistoMu",fxbins);
            TH1D* tmpfpHistoMu = (TH1D*) fpHistoMu->Rebin(fnbins,"tmpfpHistoMu",fxbins);
            TH1D* tmpfnHistoSvx = (TH1D*) fnHistoSvx->Rebin(fnbins,"tmpfnHistoSvx",fxbins);
            TH1D* tmpfpHistoSvx = (TH1D*) fpHistoSvx->Rebin(fnbins,"tmpfpHistoSvx",fxbins);
            TH1D* tmpfnHistoAll = (TH1D*) fnHistoAll->Rebin(fnbins,"tmpfnHistoAll",fxbins);
            TH1D* tmpfpHistoAll = (TH1D*) fpHistoAll->Rebin(fnbins,"tmpfpHistoAll",fxbins);

            delete fnHisto;
            delete fpHisto;
            delete fnHistoMu;
            delete fpHistoMu;
            delete fnHistoSvx;
            delete fpHistoSvx;
            delete fnHistoAll;
            delete fpHistoAll;


            fnHisto = (TH1D*) tmpfnHisto->Clone("fnHisto");
            fpHisto = (TH1D*) tmpfpHisto->Clone("fpHisto");
            fnHistoMu = (TH1D*) tmpfnHistoMu->Clone("fnHistoMu");
            fpHistoMu = (TH1D*) tmpfpHistoMu->Clone("fpHistoMu");
            fnHistoSvx = (TH1D*) tmpfnHistoSvx->Clone("fnHistoSvx");
            fpHistoSvx = (TH1D*) tmpfpHistoSvx->Clone("fpHistoSvx");
            fnHistoAll = (TH1D*) tmpfnHistoAll->Clone("fnHistoAll");
            fpHistoAll = (TH1D*) tmpfpHistoAll->Clone("fpHistoAll");

        }

	// Save input plots in the SolverInput
	_solverInput.n.all = fnHisto;
	_solverInput.n.mu = fnHistoMu;
	_solverInput.n.tag = fnHistoSvx;
	_solverInput.n.muTag = fnHistoAll;
	_solverInput.p.all = fpHisto;
	_solverInput.p.mu = fpHistoMu;
	_solverInput.p.tag = fpHistoSvx;
	_solverInput.p.muTag = fpHistoAll;

	_flavouredInput.n.b = halljets_b;
        _flavouredInput.n.tag.b = htagjets_b;
        _flavouredInput.n.mu.b = halljets_b_ptrel;
        _flavouredInput.n.muTag.b = htagjets_b_ptrel;

        _flavouredInput.n.cl = halljets_cl;
        _flavouredInput.n.tag.cl = htagjets_cl;
        _flavouredInput.n.mu.cl = halljets_cl_ptrel;
        _flavouredInput.n.muTag.cl = htagjets_cl_ptrel;

        _flavouredInput.p.b = halloppjets_b;
        _flavouredInput.p.tag.b = htagoppjets_b;
        _flavouredInput.p.mu.b = halloppjets_b_ptrel;
        _flavouredInput.p.muTag.b = htagoppjets_b_ptrel;

        _flavouredInput.p.cl = halloppjets_cl;
        _flavouredInput.p.tag.cl = htagoppjets_cl;
        _flavouredInput.p.mu.cl = halloppjets_cl_ptrel;
        _flavouredInput.p.muTag.cl = htagoppjets_cl_ptrel;

        h1["eff_pTrel_b"] = (TH1D*) fnHisto->Clone("eff_pTrel_b");
        h1["eff_pTrel_cl"] = (TH1D*) fnHisto->Clone("eff_pTrel_cl");
        h1["eff_pTrel_TaggedJet_b"] = (TH1D*) fnHisto->Clone("eff_pTrel_TaggedJet_b");
        h1["eff_pTrel_TaggedJet_cl"] = (TH1D*) fnHisto->Clone("eff_pTrel_TaggedJet_cl");
        h1["eff_TaggedJet_b"] = (TH1D*) fnHisto->Clone("eff_TaggedJet_b");
        h1["eff_TaggedJet_cl"] = (TH1D*) fnHisto->Clone("eff_TaggedJet_cl");
        h1["eff_TaggedBothJets_b"] = (TH1D*) fnHisto->Clone("eff_TaggedBothJets_b");
        h1["eff_TaggedBothJets_cl"] = (TH1D*) fnHisto->Clone("eff_TaggedBothJets_cl");

        h1["eff_mu_taggedaway_b"] = (TH1D*) fnHisto->Clone("eff_mu_taggedaway_b");
        h1["eff_mu_taggedaway_cl"] = (TH1D*) fnHisto->Clone("eff_mu_taggedaway_cl");

        h1["eff_pTrel_b"]->Reset();
        h1["eff_pTrel_cl"]->Reset();
        h1["eff_pTrel_TaggedJet_b"]->Reset();
        h1["eff_pTrel_TaggedJet_cl"]->Reset();
        h1["eff_TaggedJet_b"]->Reset();
        h1["eff_TaggedJet_cl"]->Reset();
        h1["eff_TaggedBothJets_b"]->Reset();
        h1["eff_TaggedBothJets_cl"]->Reset();
        h1["eff_mu_taggedaway_b"]->Reset();
        h1["eff_mu_taggedaway_cl"]->Reset();

        std::cout << " get MC truth input: " << std::endl;
        halljets_b->Print("all");
        halljets_cl->Print("all");
        halloppjets_b->Print("all");
        halloppjets_cl->Print("all");


        h1["eff_pTrel_b"]->Divide(b_halljets_ptrel , halljets_b ,1.,1.,"B");
        h1["eff_pTrel_cl"]->Divide(cl_halljets_ptrel, halljets_cl ,1.,1.,"B");
        h1["eff_TaggedJet_b"]->Divide(b_halljets_tagged , halljets_b ,1.,1.,"B");
        h1["eff_TaggedJet_cl"]->Divide(cl_halljets_tagged , halljets_cl ,1.,1.,"B");
        h1["eff_pTrel_TaggedJet_b"]->Divide(b_halljets_ptreltagged, halljets_b ,1.,1.,"B");
        h1["eff_pTrel_TaggedJet_cl"]->Divide(cl_halljets_ptreltagged, halljets_cl ,1.,1.,"B");
        h1["eff_TaggedBothJets_b"]->Divide( b_halloppjets_tagged , halloppjets_b ,1.,1.,"B");
        h1["eff_TaggedBothJets_cl"]->Divide( cl_halloppjets_tagged , halloppjets_cl ,1.,1.,"B");
        h1["eff_mu_taggedaway_b"]->Divide(b_halloppjets_ptrel , halloppjets_b, 1.,1.,"B");
        h1["eff_mu_taggedaway_cl"]->Divide( cl_halloppjets_ptrel, halloppjets_cl, 1.,1.,"B");

	if (! fMCtruthAwayTag )
	  {
	    feffTag_b = (TH1D*) h1["eff_TaggedJet_b"]->Clone("feffTag_b");
	    feffTag_cl = (TH1D*) h1["eff_TaggedJet_cl"]->Clone("feffTag_cl");
	  }
	else
	  {
	    feffTag_b = (TH1D*) h1["eff_TaggedBothJets_b"]->Clone("feffTag_b");
            feffTag_cl = (TH1D*) h1["eff_TaggedBothJets_cl"]->Clone("feffTag_cl");
	  }

        feffmu_b = (TH1D*) h1["eff_pTrel_b"]->Clone("feffmu_b");
        feffmu_cl = (TH1D*) h1["eff_pTrel_cl"]->Clone("feffmu_cl");


        fh_alpha = (TH1D*) fnHisto->Clone("fh_alpha");
        fh_beta = (TH1D*) fnHisto->Clone("fh_beta");
        fh_kcl = (TH1D*) fnHisto->Clone("fh_kcl");
        fh_kb = (TH1D*) fnHisto->Clone("fh_kb");
        fh_delta = (TH1D*) fnHisto->Clone("fh_delta");
        fh_gamma = (TH1D*) fnHisto->Clone("fh_gamma");
        fh_alpha->Reset();
        fh_beta->Reset();
        fh_kcl->Reset();
        fh_kb->Reset();
        fh_delta->Reset();
        fh_gamma->Reset();

        fh_alpha->Divide( h1["eff_TaggedBothJets_cl"], h1["eff_TaggedJet_cl"]);
        fh_beta->Divide( h1["eff_TaggedBothJets_b"], h1["eff_TaggedJet_b"]);

        fh_kb->Divide( h1["eff_pTrel_TaggedJet_b"], h1["eff_pTrel_b"]);
        fh_kb->Divide( h1["eff_TaggedJet_b"]);

        fh_kcl->Divide( h1["eff_pTrel_TaggedJet_cl"], h1["eff_pTrel_cl"] );
        fh_kcl->Divide(  h1["eff_TaggedJet_cl"] );

        fh_delta->Divide( h1["eff_mu_taggedaway_b"], h1["eff_pTrel_b"] );
        fh_gamma->Divide( h1["eff_mu_taggedaway_cl"], h1["eff_pTrel_cl"] );


        // fit to pol0
        fh_alpha->Fit("pol0","0");
        std::cout << "Fit to pol0 fh_alpha: Chi2 = " << fh_alpha->GetFunction("pol0")->GetChisquare() << std::endl;
        fh_beta->Fit("pol0","0");
        std::cout << "Fit to pol0 fh_beta: Chi2 = " << fh_beta->GetFunction("pol0")->GetChisquare() << std::endl;
        fh_kb->Fit("pol0","0");
        std::cout << "Fit to pol0 fh_kb: Chi2 = " << fh_kb->GetFunction("pol0")->GetChisquare() << std::endl;
        fh_kcl->Fit("pol0","0");
        std::cout << "Fit to pol0 fh_kcl: Chi2 = " << fh_kcl->GetFunction("pol0")->GetChisquare() << std::endl;
        fh_delta->Fit("pol0","0");
        std::cout << "Fit to pol0 fh_delta: Chi2 = " << fh_delta->GetFunction("pol0")->GetChisquare() << std::endl;
        fh_gamma->Fit("pol0","0");
        std::cout << "Fit to pol0 fh_gamma: Chi2 = " << fh_gamma->GetFunction("pol0")->GetChisquare() << std::endl;


        // fit to pol1
        fh_alpha->Fit("pol1","0+");
        std::cout << "Fit to pol1 fh_alpha: Chi2 = " << fh_alpha->GetFunction("pol1")->GetChisquare() << std::endl;
        fh_beta->Fit("pol1","0+");
        std::cout << "Fit to pol1 fh_beta: Chi2 = " << fh_beta->GetFunction("pol1")->GetChisquare() << std::endl;
        fh_kb->Fit("pol1","0+");
        std::cout << "Fit to pol1 fh_kb: Chi2 = " << fh_kb->GetFunction("pol1")->GetChisquare() << std::endl;
        fh_kcl->Fit("pol1","0+");
        std::cout << "Fit to pol1 fh_kcl: Chi2 = " << fh_kcl->GetFunction("pol1")->GetChisquare() << std::endl;
        fh_delta->Fit("pol1","0+");
        std::cout << "Fit to pol1 fh_delta: Chi2 = " << fh_delta->GetFunction("pol1")->GetChisquare() << std::endl;
        fh_gamma->Fit("pol1","0+");
        std::cout << "Fit to pol1 fh_gamma: Chi2 = " << fh_gamma->GetFunction("pol1")->GetChisquare() << std::endl;


        // fit to pol2
        fh_alpha->Fit("pol2","0+");
        std::cout << "Fit to pol2 fh_alpha: Chi2 = " << fh_alpha->GetFunction("pol2")->GetChisquare() << std::endl;
        fh_beta->Fit("pol2","0+");
        std::cout << "Fit to pol2 fh_beta: Chi2 = " << fh_beta->GetFunction("pol2")->GetChisquare() << std::endl;
        fh_kb->Fit("pol2","0+");
        std::cout << "Fit to pol2 fh_kb: Chi2 = " << fh_kb->GetFunction("pol2")->GetChisquare() << std::endl;
        fh_kcl->Fit("pol2","0+");
        std::cout << "Fit to pol2 fh_kcl: Chi2 = " << fh_kcl->GetFunction("pol2")->GetChisquare() << std::endl;
        fh_delta->Fit("pol2","0+");
        std::cout << "Fit to pol0 fh_delta: Chi2 = " << fh_delta->GetFunction("pol2")->GetChisquare() << std::endl;
        fh_gamma->Fit("pol2","0+");
        std::cout << "Fit to pol2 fh_gamma: Chi2 = " << fh_gamma->GetFunction("pol2")->GetChisquare() << std::endl;

    }
    // should I remove the following loop?
    else
    {
        // true efficiency
      if (! fMCtruthAwayTag )
	{
	  feffTag_b = (TH1D*) gDirectory->Get("eff_TaggedBothJets_b");
	  feffTag_cl = (TH1D*) gDirectory->Get("eff_TaggedBothJets_cl");
	}
      else
	{
	  feffTag_b = (TH1D*) gDirectory->Get("eff_TaggedBothJets_b");
          feffTag_cl = (TH1D*) gDirectory->Get("eff_TaggedBothJets_cl");
	}

        feffmu_b = (TH1D*) gDirectory->Get("eff_pTrel_b");
        feffmu_cl = (TH1D*) gDirectory->Get("eff_pTrel_cl");
        feffTagmu_b = (TH1D*) gDirectory->Get("eff_pTrel_TaggedJet_b");
        feffTagmu_cl = (TH1D*) gDirectory->Get("eff_pTrel_TaggedJet_cl");

        if (fisCorrFile) finputCorrFile->cd();

        if (fcategory=="pT")
        {
            if (frebin)
            {

                TH1D* tmpfh_alpha = (TH1D*) gDirectory->Get("alpha");
                TH1D* tmpfh_beta = (TH1D*) gDirectory->Get("beta");
                TH1D* tmpfh_kb = (TH1D*) gDirectory->Get("kappa_b");
                TH1D* tmpfh_kcl = (TH1D*) gDirectory->Get("kappa_cl");

                fh_alpha = (TH1D*) tmpfh_alpha->Rebin(fnbins,"fh_alpha",fxbins);
                fh_beta = (TH1D*)tmpfh_beta->Rebin(fnbins,"fh_beta",fxbins);
                fh_kb = (TH1D*)tmpfh_kb->Rebin(fnbins,"fh_kb",fxbins);
                fh_kcl = (TH1D*)tmpfh_kcl->Rebin(fnbins,"fh_kcl",fxbins);
                // fit to pol0
                fh_alpha->Fit("pol0","0");
                fh_beta->Fit("pol0","0");
                fh_kb->Fit("pol0","0");
                fh_kcl->Fit("pol0","0");
            } else {
                fh_alpha = (TH1D*) gDirectory->Get("alpha");
                fh_beta = (TH1D*) gDirectory->Get("beta");
                fh_kb = (TH1D*) gDirectory->Get("kappa_b");
                fh_kcl = (TH1D*) gDirectory->Get("kappa_cl");
            }

        }
        else
        {
            if (frebin)
            {
                TH1D* tmpfh_alpha = (TH1D*) gDirectory->Get("alpha_eta");
                TH1D* tmpfh_beta = (TH1D*) gDirectory->Get("beta_eta");
                TH1D* tmpfh_kb = (TH1D*) gDirectory->Get("kappa_eta_b");
                TH1D* tmpfh_kcl = (TH1D*) gDirectory->Get("kappa_eta_cl");

                tmpfh_alpha->Rebin(ncorretaarray-1,"fh_alpha_eta",corretabins);
                tmpfh_beta->Rebin(ncorretaarray-1,"fh_beta_eta",corretabins);
                tmpfh_kb->Rebin(ncorretaarray-1,"fh_kb_eta",corretabins);
                tmpfh_kcl->Rebin(ncorretaarray-1,"fh_kcl_eta",corretabins);
                // fit to pol0
                fh_alpha->Fit("pol0","0");
                fh_beta->Fit("pol0","0");
                fh_kb->Fit("pol0","0");
                fh_kcl->Fit("pol0","0");

            } else {
                fh_alpha = (TH1D*) gDirectory->Get("alpha_eta");
                fh_beta = (TH1D*) gDirectory->Get("beta_eta");
                fh_kb = (TH1D*) gDirectory->Get("kappa_b_eta");
                fh_kcl = (TH1D*) gDirectory->Get("kappa_cl_eta");
            }
        }
    }

    std::cout << " got correlations" << std::endl;
}

void S8Solver::GetInput()
{
    LoadHistos();

    // integrated input
    if (fusemctrue)
    {
        TotalInput["n"] = halljets_b->Integral(1,halljets_b->GetNbinsX()+1) + halljets_cl->Integral(1,halljets_cl->GetNbinsX()+1);
        TotalInput["nMu"] = halljets_b_ptrel->Integral(1,halljets_b_ptrel->GetNbinsX()+1) + halljets_cl_ptrel->Integral(1,halljets_cl_ptrel->GetNbinsX()+1);
        TotalInput["p"] = halloppjets_b->Integral(1,halloppjets_b->GetNbinsX()+1) + halloppjets_cl->Integral(1,halloppjets_cl->GetNbinsX()+1) ;
        TotalInput["pMu"] = halloppjets_b_ptrel->Integral(1,halloppjets_b_ptrel->GetNbinsX()+1) + halloppjets_cl_ptrel->Integral(1,halloppjets_cl_ptrel->GetNbinsX()+1);
        TotalInput["nTag"] = htagjets_b->Integral(1,htagjets_b->GetNbinsX()+1) + htagjets_cl->Integral(1,htagjets_cl->GetNbinsX()+1);
        TotalInput["nMuTag"] =  htagjets_b_ptrel->Integral(1,htagjets_b_ptrel->GetNbinsX()+1) + htagjets_cl_ptrel->Integral(1,htagjets_cl_ptrel->GetNbinsX()+1);
        TotalInput["pTag"] = htagoppjets_b->Integral(1,htagoppjets_b->GetNbinsX()+1) + htagoppjets_cl->Integral(1,htagoppjets_cl->GetNbinsX()+1);
        TotalInput["pMuTag"] =  htagoppjets_b_ptrel->Integral(1,htagoppjets_b_ptrel->GetNbinsX()+1) + htagoppjets_cl_ptrel->Integral(1,htagoppjets_cl_ptrel->GetNbinsX()+1);
    }
    else
    {
        TotalInput["n"] = fnHisto->Integral(1, fnHisto->GetNbinsX()+1);
        TotalInput["nMu"] = fnHistoMu->Integral(1,fnHistoMu->GetNbinsX()+1);
        TotalInput["p"] = fpHisto->Integral(1,fpHisto->GetNbinsX()+1);
        TotalInput["pMu"] = fpHistoMu->Integral(1,fpHistoMu->GetNbinsX()+1);
        TotalInput["nTag"] = fnHistoSvx->Integral(1,fnHistoSvx->GetNbinsX()+1);
        TotalInput["nMuTag"] = fnHistoAll->Integral(1,fnHistoAll->GetNbinsX()+1);
        TotalInput["pTag"] = fpHistoSvx->Integral(1,fpHistoSvx->GetNbinsX()+1);
        TotalInput["pMuTag"] = fpHistoAll->Integral(1,fpHistoAll->GetNbinsX()+1);
    }

    // Get Total Input
    //
    std::cout << " get total input " << endl;
    _totalInput = inputGroup(_solverInput, _flavouredInput);
    std::cout << " done with total input" << endl;

    // binned input base in the n samples
    //
    TF1 *F_kb = fh_kb->GetFunction("pol1");
    TF1 *F_kcl = fh_kcl->GetFunction("pol1");
    TF1 *F_alpha = fh_alpha->GetFunction("pol1");
    TF1 *F_beta = fh_beta->GetFunction("pol1");
    TF1 *F_delta = fh_delta->GetFunction("pol1");
    TF1 *F_gamma = fh_gamma->GetFunction("pol1");

    std::vector< TH1* > HistoList;
    HistoList.push_back(fnHisto);
    HistoList.push_back(fnHistoMu);
    HistoList.push_back(fpHisto);
    HistoList.push_back(fpHistoMu);
    HistoList.push_back(fnHistoSvx);
    HistoList.push_back(fnHistoAll);
    HistoList.push_back(fpHistoSvx);
    HistoList.push_back(fpHistoAll);
    HistoList.push_back(fh_kb);
    HistoList.push_back(fh_kcl);
    HistoList.push_back(fh_alpha);
    HistoList.push_back(fh_beta);
    HistoList.push_back(fh_delta);
    HistoList.push_back(fh_gamma);

    std::vector< TString> name;
    name.push_back("n");
    name.push_back("nMu");
    name.push_back("p");
    name.push_back("pMu");
    name.push_back("nTag");
    name.push_back("nMuTag");
    name.push_back("pTag");
    name.push_back("pMuTag");
    name.push_back("kappa_b");
    name.push_back("kappa_cl");
    name.push_back("alpha");
    name.push_back("beta");
    name.push_back("delta");
    name.push_back("gamma");

    // Binned input
    //
    for(int ibin = 1, bins = fnHisto->GetNbinsX();
         bins >= ibin;
         ++ibin)
    {
        // Calculate each bin input explicitly
        //
        NumericInputGroup group = inputGroup(
            _solverInput,
            _flavouredInput,
            ibin);

        if (bins == ibin)
        {
            // Last bin
            //
            add(group, _solverInput, _flavouredInput, ibin);
        }

        _binnedInput.push_back(group);

        // Done binned input preparation
        //

        std::map<TString, double> tmpmap;

        double pt = fnHisto->GetXaxis()->GetBinCenter(ibin);

        for ( size_t ihisto=0; ihisto!=HistoList.size(); ++ihisto)
        {
            TH1D *htemp = (TH1D*) HistoList[ihisto];

            if (name[ihisto]=="kappa_b"||name[ihisto]=="kappa_cl"||
                name[ihisto]=="alpha"||name[ihisto]=="beta"||
                name[ihisto]=="delta"||name[ihisto]=="gamma")
            {
                if(name[ihisto]=="beta")
                {
                    if (!fBetaConst) {
                        tmpmap[name[ihisto]] = fBetaf * F_beta->Eval(pt,0,0);
                    } else {
                        tmpmap[name[ihisto]] = TotalInput["beta"];
                    }
                }
                else if(name[ihisto]=="alpha") {
                    if (!fAlphaConst) {
                        tmpmap[name[ihisto]] = fAlphaf * F_alpha->Eval(pt,0,0);
                    } else {
                        tmpmap[name[ihisto]] = TotalInput["alpha"];
                    }
                }
                else if(name[ihisto]=="kappa_b") {
                  if (!fKappabConst) {
                    tmpmap[name[ihisto]] = fKappabf * F_kb->Eval(pt,0,0);
                  } else {
                    tmpmap[name[ihisto]] = TotalInput["kappa_b"];
                  }
                }
                else if(name[ihisto]=="kappa_cl") {
                  if (!fKappaclConst) {
                    tmpmap[name[ihisto]] = fKappaclf * F_kcl->Eval(pt,0,0);
                  } else {
                    tmpmap[name[ihisto]] = TotalInput["kappa_cl"];
                  }
                }
                else if(name[ihisto]=="delta") {
                  if (!fDeltaConst) {
                    tmpmap[name[ihisto]] = fDeltaf * F_delta->Eval(pt,0,0);
                  }  else {
                    tmpmap[name[ihisto]] = TotalInput["delta"];
                  }
                }
                else if(name[ihisto]=="gamma") {
                  if (!fGammaConst) {
                    tmpmap[name[ihisto]] = fGammaf * F_gamma->Eval(pt,0,0);
                  } else {
                    tmpmap[name[ihisto]] = TotalInput["gamma"];
                  }
                }

            } else {
                tmpmap[name[ihisto]] = htemp->GetBinContent(ibin);
            }
        }

        BinnedInput[ibin] = tmpmap;
    }

    cout << _binnedInput.size() << " binned inputs stored" << endl;
}

void S8Solver::Solve()
{
    GetInput();

    if (fmethod=="analytic" || fmethod=="fit" )
    {
        S8AnalyticSolver sol;

        sol.Solve(TotalInput);

        fTotalSolution = sol.GetSolution();
        fTotalSolutionErr = sol.GetSolutionErr();

        // now fit
        //
        if (fmethod=="fit")
        {
            S8FitSolver s8fit;
            s8fit.Init(fTotalSolution);
            s8fit.Solve(TotalInput);
            fTotalSolution = s8fit.GetSolution();
            fTotalSolutionErr = s8fit.GetSolutionErr();
        }

        // binned solution
        //
        for(BinnedInputMap::const_iterator ibin = BinnedInput.begin();
            ibin != BinnedInput.end();
            ++ibin)
        {
            sol.Solve(ibin->second);
            fBinnedSolution[ibin->first] = sol.GetSolution();
            fBinnedSolutionErr[ibin->first] = sol.GetSolutionErr();

            // now fit
            //
            if (fmethod=="fit") {
                S8FitSolver as8fit;
                as8fit.Init(fBinnedSolution[ibin->first]);
                as8fit.Solve(ibin->second);
                fBinnedSolution[ibin->first] = as8fit.GetSolution();
                fBinnedSolutionErr[ibin->first] = as8fit.GetSolutionErr();
            }
        }
    }
    else if (fmethod=="numeric")
    {
        doAverageSolution();

        doBinnedSolution();
    }
}

void S8Solver::doAverageSolution()
{
    if (!_doAverageSolution)
        return;

    cout << endl;
    cout << "AVERAGE SOLUTION" << endl;
    cout << endl;

    System8Solver sol;
    sol.setInput(_totalInput);

    sol.SetError(System8Solver::STAT, 1000);
    sol.SetInitialOrder(1, 1);

    // Force solution manually if requested
    //
    /*
    for(std::map<int,int>::const_iterator ipick = fPickSolutionMap.begin();
        ipick!= fPickSolutionMap.end();
        ++ipick)
    {
        if (0 == ipick->first)
        {
            std::cout << "> Force average solution # " << ipick->second << std::endl;
            sol.SetSolution(ipick->second);

            break;
        }
    }
    */

    sol.Solve();

    // Save Fit Results in output file
    //
    {
        TFile *out = TFile::Open("out_avg.root", "recreate");
        for (int i = 0; 8 > i; ++i)
        {
            sol.result(i)->Write();
            *(_averageResults + i) = (TH1 *) sol.result(i)->Clone();
        }
        out->Close();
    }

    saveSolution(_totalSolution, sol, _totalInput);
}

void S8Solver::doBinnedSolution()
{
    if (!_doBinnedSolution)
        return;

    _binnedSolution.clear();

    // binned solutions
    //
    unsigned int bin = (-1 == _firstBin ? 1 : _firstBin);
    const unsigned int maxBin = (-1 == _lastBin ? 10000 : _lastBin);
    if (1 > bin ||
        _binnedInput.size() < bin)
    {
        cerr << "first bin value " << bin << " is not supported" << endl;

        return;
    }

    BinnedNumericInputGroup::const_iterator inputGroup = _binnedInput.begin();
    advance(inputGroup, bin - 1);
    for(;
        _binnedInput.end() != inputGroup &&
            maxBin >= bin;
        ++inputGroup, ++bin)
    {
        cout << endl;
        cout << "BINED SOLUTION... BIN " << bin << endl;
        cout << endl;

        System8Solver solu;
        solu.setInput(*inputGroup);

        solu.SetError(System8Solver::STAT, 1000);
        solu.SetInitialOrder(1, 1);

        // pick solution manually if requested
        /*
        for(std::map<int, int>::const_iterator ipick = fPickSolutionMap.begin();
            ipick != fPickSolutionMap.end();
            ++ipick)
        {
            if (static_cast<unsigned int>(ipick->first) == bin)
            {
                cout << "> Force binned solution # " << ipick->second
                    << endl;
                solu.SetSolution( ipick->second );

                break;
            }
        }
        */

        if (!solu.Solve())
            continue;

        // Save Fits
        //
        {
            std::ostringstream name;
            name << "out_bin_" << bin << ".root";
            TFile *binout = TFile::Open(name.str().c_str(), "recreate");
            for (int i = 0; 8 > i; ++i)
            {
                solu.result(i)->Write();
            }
            binout->Close();
        }

        SolutionInBin solutionInBin;
        saveSolution(solutionInBin.solution, solu, *inputGroup);
        solutionInBin.bin = inputGroup->bin;
        solutionInBin.binID = bin;
        _binnedSolution.push_back(solutionInBin);
    }
}

// bin 0 corresponds to average solution
//
void S8Solver::SetSolution(const int &bin, const int &solution)
{
    fPickSolutionMap[bin] = solution;
}

void S8Solver::PrintData(TString option)
{
    // Setup
    //
    std::cout << " Name: " << fthename << std::endl;
    std::cout << " Method: " << fmethod << std::endl;
    std::cout << " Category: " << fcategory << std::endl;
    std::cout << " alpha constant: " << fAlphaConst << std::endl;
    std::cout << " beta constant: " << fBetaConst << std::endl;
    std::cout << " k_b constant: " << fKappabConst << std::endl;
    std::cout << " k_cl constant: " << fKappaclConst << std::endl;
    std::cout << " max Ptrel: " << fMaxPtrel << std::endl;
    std::cout << " rebin correlation histograms: " << frebin << std::endl;
    std::cout << " recalculate correlation histograms: " << frecalculateFactors << std::endl;
    std::cout << " alpha scale factor: " << fAlphaf << std::endl;
    std::cout << " beta scale factor: " << fBetaf << std::endl;
    std::cout << " k_b scale factor: " << fKappabf << std::endl;
    std::cout << " k_cl scale factor: " << fKappaclf << std::endl;

    if (_doAverageSolution)
    {
        cout << " [Average]" << endl
            << _totalInput << endl;

        cout << " SOLUTION" << endl;
        cout << _totalSolution << endl
            << endl;
    }

    if (!_doBinnedSolution)
        return;

    // Binned
    //
    cout << " [Binned]" << endl;
    for(BinnedSolution::const_iterator solution = _binnedSolution.begin();
        _binnedSolution.end() != solution;
        ++solution)
    {
        cout << setw(40) << setfill('-') << ' '
            << " Bin " << solution->binID
            << " ---" << setfill(' ') << endl
            << endl;

        BinnedNumericInputGroup::const_iterator input = _binnedInput.begin();
        advance(input, (solution->binID - 1));
        cout << *input << endl;

        cout << " SOLUTION" << endl;
        cout << solution->solution << endl;

        cout << setw(50) << setfill('-') << ' ' << setfill(' ')
            << endl
            << endl;
    }
}

void S8Solver::Save(TString filename)
{
    if (!_doBinnedSolution)
    {
        cout << "Binned solution is not run. Nothing to save" << endl;

        return;
    }

    auto_ptr<TFile> ofile(new TFile(filename,"RECREATE"));
    if (!ofile->IsOpen())
    {
        cerr << "Failed to open output file" << endl;

        return;
    }

    generateGraphs();

    // Save Graphs
    //
    _graphs->save(ofile.get());
}

void S8Solver::DumpTable(std::string filename)
{
    Int_t nxbins = fBinnedSolution.size();
    // separator
    std::string sp = ",";

    std::ofstream ff;
    ff.open(filename.c_str());

    // header
    ff <<  "ptMin,ptMax,etaMin,etaMax,bTagEff,bTagEffErr,clTagEff,clTagEfferr,bPtrelEff,bPtrelEffErr,clPtrelEff,clPtrelEfferr"<< sp << fthename << std::endl;

    // MC truth
    for (int ibin = 1; ibin<= nxbins; ++ibin) {

        double ptcenter = fnHisto->GetXaxis()->GetBinCenter(ibin);
        double ptdelta = 0.5 * fnHisto->GetXaxis()->GetBinWidth(ibin);

        double etamin = 0.;
        double etamax = 2.5;

        double beff = feffTag_b->GetBinContent(ibin);
        double befferr = feffTag_b->GetBinError(ibin);

        double cleff = feffTag_cl->GetBinContent(ibin);
        double clefferr = feffTag_cl->GetBinError(ibin);

        ff << ptcenter - ptdelta << sp << ptcenter + ptdelta << sp << etamin << sp << etamax << sp << beff << sp << befferr << sp << cleff << sp << clefferr << std::endl;

    }

    int bbin = 1;
    for( std::map<int,std::map<TString,double> >::const_iterator ibin = fBinnedSolution.begin(); ibin!=fBinnedSolution.end(); ++ibin) {

        if (fcategory == "pT") {

            double ptcenter = fnHisto->GetXaxis()->GetBinCenter(bbin);
            double ptdelta = 0.5 * (fnHisto->GetXaxis())->GetBinWidth(bbin);

            double etamin = 0.;
            double etamax = 2.5;

            double beff = 0.;
            double befferr = 0.;

            double cleff = 0.;
            double clefferr = 0.;

            double bPtreleff = 0.;
            double bPtrelefferr = 0.;

            double clPtreleff = 0.;
            double clPtrelefferr = 0.;

            std::map<TString, double> tmpmap = ibin->second;
            std::map<TString, double> tmpmaperr = fBinnedSolutionErr[ibin->first];
            for( std::map<TString,double>::const_iterator i = tmpmap.begin(); i!=tmpmap.end(); ++i) {

                if (i->first == "effTag_b") { beff = i->second; befferr = tmpmaperr[i->first]; }
                if (i->first == "effTag_cl") { cleff = i->second; clefferr = tmpmaperr[i->first]; }
                if (i->first == "effMu_b") { bPtreleff = i->second; bPtrelefferr = tmpmaperr[i->first]; }
                if (i->first == "effMu_cl") { clPtreleff = i->second; clPtrelefferr = tmpmaperr[i->first]; }

            }
            ff << ptcenter - ptdelta << sp << ptcenter + ptdelta << sp << etamin << sp << etamax << sp
               << beff << sp << befferr << sp << cleff << sp << clefferr <<sp
               << bPtreleff << sp << bPtrelefferr << sp << clPtreleff << sp << clPtrelefferr
               << std::endl;
            bbin++;
        }
        if (fcategory == "eta") {

            double etacenter = fnHisto->GetXaxis()->GetBinCenter(bbin);
            double etadelta = 0.5 * fnHisto->GetXaxis()->GetBinWidth(bbin);

            double ptmin = 30.;
            double ptmax = 230.;

            double beff = 0.;
            double befferr = 0.;

            double cleff = 0.;
            double clefferr = 0.;

            double bPtreleff = 0.;
            double bPtrelefferr = 0.;

            double clPtreleff = 0.;
            double clPtrelefferr = 0.;

            std::map<TString, double> tmpmap = ibin->second;
            std::map<TString, double> tmpmaperr = fBinnedSolutionErr[ibin->first];
            for( std::map<TString,double>::const_iterator i = tmpmap.begin(); i!=tmpmap.end(); ++i) {

                if (i->first == "effTag_b") { beff = i->second; befferr = tmpmaperr[i->first]; }
                if (i->first == "effTag_cl") { cleff = i->second; clefferr = tmpmaperr[i->first]; }
                if (i->first == "effMu_b") { bPtreleff = i->second; bPtrelefferr = tmpmaperr[i->first]; }
                if (i->first == "effMu_cl") { clPtreleff = i->second; clPtrelefferr = tmpmaperr[i->first]; }


            }
            ff << ptmin << sp << ptmax << sp << etacenter - etadelta << sp << etacenter + etadelta << sp << beff << sp
               << befferr << sp << cleff << sp << clefferr << sp
               << bPtreleff << sp << bPtrelefferr << sp << clPtreleff << sp << clPtrelefferr
               << std::endl;
            bbin++;
        }
    }
}

void S8Solver::Draw(int maxNbins)
{
    if (!_doBinnedSolution)
    {
        cout << "Binned solution is not run. Nothing to Draw" << endl;

        return;
    }

    generateGraphs();

    // Draw Graphs
    //
    _graphs->draw();
}



// Generage graphs for binned solutions only
//
void S8Solver::generateGraphs()
{
    if (_graphs.get())
        return;

    cout << "Generate Group" << endl;

    if ("eta" == fcategory)
        _graphs.reset(new GraphGroup(_binnedInput, _binnedSolution,
                                     Graph::ETA));
    else if ("phi" == fcategory)
        _graphs.reset(new GraphGroup(_binnedInput, _binnedSolution,
                                     Graph::PHI));
    else
        _graphs.reset(new GraphGroup(_binnedInput, _binnedSolution));
}



// Helpers
//
void saveSolution(Solution &solution,
                  System8Solver &solver,
                  const NumericInputGroup &inputGroup)
{
    // Remove any previously saved solutions
    //
    solution.clear();

    // n_b
    //
    //const double n_b = solver.GetResult(0) * inputGroup.input.n.all.first;
    const double n_b = solver.getCentralValue(0) * inputGroup.input.n.all.first;
    solution["n_b"] =
        make_pair(n_b,
                  pow(solver.getError(0) * inputGroup.input.n.all.first, 2));

    // n_cl
    //
    //const double n_cl = solver.GetResult(1) * inputGroup.input.n.all.first;
    const double n_cl = solver.getCentralValue(1) * inputGroup.input.n.all.first;
    solution["n_cl"] =
        make_pair(n_cl,
                  pow(solver.getError(1) * inputGroup.input.n.all.first, 2));

    // Eff_mu_b
    //
    solution["eff_mu_b"] =
        //make_pair(solver.GetResult(2),
        make_pair(solver.getCentralValue(2),
                  pow(solver.getError(2), 2));

    // Eff_mu_cl
    //
    solution["eff_mu_cl"] =
        //make_pair(solver.GetResult(3),
        make_pair(solver.getCentralValue(3),
                  pow(solver.getError(3), 2));

    // Eff_tag_b
    //
    solution["eff_tag_b"] =
        //make_pair(solver.GetResult(4),
        make_pair(solver.getCentralValue(4),
                  pow(solver.getError(4), 2));

    // Eff_tag_cl
    //
    solution["eff_tag_cl"] =
        //make_pair(solver.GetResult(5),
        make_pair(solver.getCentralValue(5),
                  pow(solver.getError(5), 2));

    // p_b
    //
    solution["p_b"] =
        //make_pair(solver.GetResult(6) * n_b,
        make_pair(solver.getCentralValue(6) * n_b,
                  pow(solver.getError(6) * n_b, 2));

    // p_cl
    //
    solution["p_cl"] =
        //make_pair(solver.GetResult(7) * n_cl,
        make_pair(solver.getCentralValue(7) * n_cl,
                  pow(solver.getError(7) * n_cl, 2));
}

std::ostream &operator <<(std::ostream &out, const Solution &solution)
{
    for(Solution::const_iterator measurement = solution.begin();
        solution.end() != measurement;
        ++measurement)
    {
        out << " [+] " << setw(15) << left << measurement->first
            << measurement->second << endl;
    }

    return out;
}

void inputGroup(numeric::InputGroup &inputGroup,
                const solver::PlotGroup &plotGroup,
                const int &bin)
{
    inputGroup.all += measurement(plotGroup.all, bin);
    inputGroup.mu += measurement(plotGroup.mu, bin);
    inputGroup.tag += measurement(plotGroup.tag, bin);
    inputGroup.muTag += measurement(plotGroup.muTag, bin);
}

void inputGroup(numeric::InputGroup &inputGroup,
                const solver::PlotGroup &plotGroup)
{
    inputGroup.all += measurement(plotGroup.all);
    inputGroup.mu += measurement(plotGroup.mu);
    inputGroup.tag += measurement(plotGroup.tag);
    inputGroup.muTag += measurement(plotGroup.muTag);
}

void flavouredInput(numeric::FlavouredInput &input,
                    const solver::FlavouredPlot &plot,
                    const int &bin)
{
    input.b += measurement(plot.b, bin);
    input.cl += measurement(plot.cl, bin);
}

void flavouredInput(numeric::FlavouredInput &input,
                    const solver::FlavouredPlot &plot)
{
    input.b += measurement(plot.b);
    input.cl += measurement(plot.cl);
}

NumericInputGroup inputGroup(const SolverInput &input,
                             const FlavouredSolverInput &flavouredInput,
                             const int &bin)
{
    NumericInputGroup group;

    group.bin =
            make_pair(input.n.all->GetBinCenter(bin),
                      pow(input.n.all->GetBinWidth(bin) / 2, 2));
;

    // S8 Input
    //
    fill(group.input, input, bin);

    // MC Input
    //
    fill(group.flavouredInput, flavouredInput, bin);

    // Measurements
    //
    inputGroup(group, group.flavouredInput.n, group.flavouredInput.p);

    return group;
}

NumericInputGroup inputGroup(const SolverInput &input,
                             const FlavouredSolverInput &flavouredInput)
{
    NumericInputGroup group;

    // S8 Input
    //
    fill(group.input, input);

    // MC Input
    //
    fill(group.flavouredInput, flavouredInput);

    // Measurements
    //
    inputGroup(group, group.flavouredInput.n, group.flavouredInput.p);

    return group;
}

void add(NumericInputGroup &group,
         const SolverInput &input,
         const FlavouredSolverInput &flavouredInput,
         const int &bin)
{
    // S8 Input
    //
    fill(group.input, input, bin);

    // MC Input
    //
    fill(group.flavouredInput, flavouredInput, bin);

    // Measurements
    //
    inputGroup(group, group.flavouredInput.n, group.flavouredInput.p);
}

void inputGroup(NumericInputGroup &group,
                const numeric::FlavouredInputGroup &n,
                const numeric::FlavouredInputGroup &p)
{
    // Efficiencies
    //
    FlavouredEffGroup &eff = group.efficiency;
    eff.tag.b = n.tag.b % n.b;
    eff.tag.cl = n.tag.cl % n.cl;

    eff.mu.b = n.mu.b % n.b;
    eff.mu.cl = n.mu.cl % n.cl;

    // Coefficients
    //
    Coefficients &coef = group.coefficients;
    coef.alpha = p.tag.cl % p.cl / eff.tag.cl;
    coef.beta = p.tag.b % p.b / eff.tag.b;

    coef.gamma = p.mu.cl % p.cl / eff.mu.cl;
    coef.delta = p.mu.b % p.b / eff.mu.b;

    coef.kappaCL = n.muTag.cl % n.cl / eff.mu.cl / eff.tag.cl;
    coef.kappaB = n.muTag.b % n.b / eff.mu.b / eff.tag.b;

    coef.kappaCL123 = p.muTag.cl % p.cl / eff.mu.cl / eff.tag.cl;
    coef.kappaB123 = p.muTag.b % p.b / eff.mu.b / eff.tag.b;
}

void fill(NumericInput &numericInput, const SolverInput &input, const int &bin)
{
    inputGroup(numericInput.n, input.n, bin);
    inputGroup(numericInput.p, input.p, bin);
}

void fill(NumericInput &numericInput, const SolverInput &input)
{
    inputGroup(numericInput.n, input.n);
    inputGroup(numericInput.p, input.p);
}

void fill(FlavouredNumericInput &numericInput,
          const FlavouredSolverInput &input,
          const int &bin)
{
    fill(numericInput.n, input.n, bin);
    fill(numericInput.p, input.p, bin);
}

void fill(FlavouredNumericInput &numericInput,
          const FlavouredSolverInput &input)
{
    fill(numericInput.n, input.n);
    fill(numericInput.p, input.p);
}

void fill(numeric::FlavouredInputGroup &inputGroup,
          const solver::FlavouredPlotGroup &plotGroup,
          const int &bin)
{
    flavouredInput(inputGroup, plotGroup, bin);
    flavouredInput(inputGroup.mu, plotGroup.mu, bin);
    flavouredInput(inputGroup.tag, plotGroup.tag, bin);
    flavouredInput(inputGroup.muTag, plotGroup.muTag, bin);
}

void fill(numeric::FlavouredInputGroup &inputGroup,
          const solver::FlavouredPlotGroup &plotGroup)
{
    flavouredInput(inputGroup, plotGroup);
    flavouredInput(inputGroup.mu, plotGroup.mu);
    flavouredInput(inputGroup.tag, plotGroup.tag);
    flavouredInput(inputGroup.muTag, plotGroup.muTag);
}

// Get Y with the error from histogram bin
//
Measurement measurement(const TH1 *hist, const int &bin)
{
    return make_pair(hist->GetBinContent(bin),
                     pow(hist->GetBinError(bin), 2));
}

// Get average Y value with error from the histogram
//
Measurement measurement(const TH1 *hist)
{
    Measurement newMeasurement;
    for(int bin = 1, bins = hist->GetNbinsX() + 1; bins >= bin; ++bin)
    {
        newMeasurement += measurement(hist, bin);
    }

    return newMeasurement;
}
