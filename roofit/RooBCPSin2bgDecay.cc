/*****************************************************************************
 * Project: BaBar detector at the SLAC PEP-II B-factory
 * Package: RooFitModels
 *    File: $Id: $
 * Authors:
 *   WW, Wolfgang Walkowiak, UC Santa Cruz, walkowia@slac.stanford.edu
 * History:
 *   05-Mar-2002 WW Created initial version
 *
 * Copyright (C) 2001 University of California
 *****************************************************************************/

// -- CLASS DESCRIPTION [PDF] --
// 

#include <iostream.h>
#include "RooFitCore/RooRealVar.hh"
#include "RooFitCore/RooRandom.hh"
#include "RooFitModels/RooBCPSin2bgDecay.hh"

ClassImp(RooBCPSin2bgDecay) 
;

RooBCPSin2bgDecay::RooBCPSin2bgDecay(const char *name, const char *title, 
				     RooRealVar& t, RooAbsCategory& tag,
				     RooAbsCategory& mixState,
				     RooAbsReal& tau, RooAbsReal& dm,
				     RooAbsReal& avgMistag, 
				     RooAbsReal& CPeigenval,
				     RooAbsReal& a, RooAbsReal& bPlus,
				     RooAbsReal& bMinus, 
				     RooAbsReal& delMistag,
				     const RooResolutionModel& model, 
				     DecayType type) :
    RooConvolutedPdf(name,title,model,t), 
    _absLambda("absLambda","Absolute value of lambda",this,a),
    _argLambdaPlus("argLambdaPlus","Arg(Lambda+)",this,bPlus),
    _argLambdaMinus("argLambdaPlus","Arg(Lambda-)",this,bMinus),
    _CPeigenval("CPeigenval","CP eigen value",this,CPeigenval),
    _avgMistag("avgMistag","Average mistag rate",this,avgMistag),
    _delMistag("delMistag","Delta mistag rate",this,delMistag),  
    _tag("tag","Btag state",this,tag),
    _mixState("mixState","mix state",this,mixState),
    _tau("tau","decay time",this,tau),
    _dm("dm","mixing frequency",this,dm),
    _t("t","time",this,t),
    _type(type),
    _genFlavFrac(0),
    _genMixFrac(0),
    _genFlavFracMix(0),
    _genFlavFracUnmix(0)
{
  // Constructor
  switch(type) {
  case SingleSided:
    _basisExp = declareBasis("exp(-@0/@1)",RooArgList(tau,dm)) ;
    _basisSin = declareBasis("exp(-@0/@1)*sin(@0*@2)",RooArgList(tau,dm)) ;
    _basisCos = declareBasis("exp(-@0/@1)*cos(@0*@2)",RooArgList(tau,dm)) ;
    break ;
  case Flipped:
    _basisExp = declareBasis("exp(@0)/@1)",RooArgList(tau,dm)) ;
    _basisSin = declareBasis("exp(@0/@1)*sin(@0*@2)",RooArgList(tau,dm)) ;
    _basisCos = declareBasis("exp(@0/@1)*cos(@0*@2)",RooArgList(tau,dm)) ;
    break ;
  case DoubleSided:
    _basisExp = declareBasis("exp(-abs(@0)/@1)",RooArgList(tau,dm)) ;
    _basisSin = declareBasis("exp(-abs(@0)/@1)*sin(@0*@2)",RooArgList(tau,dm)) ;
    _basisCos = declareBasis("exp(-abs(@0)/@1)*cos(@0*@2)",RooArgList(tau,dm)) ;
    break ;
  }
}


RooBCPSin2bgDecay::RooBCPSin2bgDecay(const RooBCPSin2bgDecay& other,
				     const char* name) : 
    RooConvolutedPdf(other,name), 
    _absLambda("absLambda",this,other._absLambda),
    _argLambdaPlus("argLambdaPlus",this,other._argLambdaPlus),
    _argLambdaMinus("argLambdaMinus",this,other._argLambdaMinus),
    _CPeigenval("CPeigenval",this,other._CPeigenval),
    _avgMistag("avgMistag",this,other._avgMistag),
    _delMistag("delMistag",this,other._delMistag),
    _tag("tag",this,other._tag),
    _mixState("mixState",this,other._mixState),
    _tau("tau",this,other._tau),
    _dm("dm",this,other._dm),
    _t("t",this,other._t),
    _type(other._type),
    _basisExp(other._basisExp),
    _basisSin(other._basisSin),
    _basisCos(other._basisCos),
    _genFlavFrac(other._genFlavFrac),
    _genMixFrac(other._genMixFrac),
    _genFlavFracMix(other._genFlavFracMix),
    _genFlavFracUnmix(other._genFlavFracUnmix)
{
  // Copy constructor
}



RooBCPSin2bgDecay::~RooBCPSin2bgDecay()
{
  // Destructor
}


Double_t RooBCPSin2bgDecay::coefficient(Int_t basisIndex) const 
{
    // B0      : _tag      = +1 
    // B0bar   : _tag      = -1 
    // unmixed : _mixState = +1
    // mixed   : _mixState = -1

    if (basisIndex==_basisExp) {
	//exp term: (1 -/+ dw)(1+a^2)/4 
	return (1 - _tag*_delMistag)*(1+_absLambda*_absLambda)/4 ;
	// =    1 + _tag*deltaDil/4
    }
    
    if (basisIndex==_basisSin) {
	//sin term: +/- (1-2w)*ImLambda(= -etaCP * |l| * sin(2b+g+/-d)/2 )
	if ( _tag*_mixState < 0 ) { 
	    return _tag*(1-2*_avgMistag)*_CPeigenval*_absLambda
		*_argLambdaMinus/2 ;
	} else {
	    return _tag*(1-2*_avgMistag)*_CPeigenval*_absLambda
		*_argLambdaPlus/2 ;
	}
	// =   _tag*avgDil * .../2
    }
  
    if (basisIndex==_basisCos) {
	//cos term: +/- (1-2w)*(1-a^2)/4
	return _mixState*(1-2*_avgMistag)*(1-_absLambda*_absLambda)/4 ;
	// =   mixState*avgDil * .../4
    } 
    
    return 0 ;
}



Int_t RooBCPSin2bgDecay::getCoefAnalyticalIntegral(RooArgSet& allVars, 
						    RooArgSet& analVars) const 
{
  if (matchArgs(allVars,analVars,_tag,_mixState)) return 3 ;
  if (matchArgs(allVars,analVars,_mixState)     ) return 2 ;
  if (matchArgs(allVars,analVars,_tag)          ) return 1 ;
  return 0 ;
}



Double_t RooBCPSin2bgDecay::coefAnalyticalIntegral(Int_t basisIndex, 
						    Int_t code) const 
{
    switch(code) {
	// No integration
	case 0: return coefficient(basisIndex) ;
	    
	    // integration over 'tag' and 'mixState'
	case 3: 
	    if (basisIndex==_basisExp) {
		return (1+_absLambda*_absLambda) ;
	    }
	    if (basisIndex==_basisSin || basisIndex==_basisCos) {
		return 0 ;
	    }
	    
	case 2:
	    // integration over 'mixState' 
	    if (basisIndex==_basisExp) {
		return 2*coefficient(basisIndex) ;
	    }
	    if (basisIndex==_basisSin) {
		return _tag*(1-2*_avgMistag)*_CPeigenval*_absLambda
		*(_argLambdaPlus+_argLambdaMinus)/2 ;
	    }
	    if (basisIndex==_basisCos) {
		return 0 ;
	    }

	    // Integration over 'tag'
	case 1:
	    if (basisIndex==_basisExp) {
		return (1+_absLambda*_absLambda)/2 ;
	    }
	    if (basisIndex==_basisSin ) {
		return _mixState*(1-2*_avgMistag)*_CPeigenval*_absLambda
		*(_argLambdaPlus-_argLambdaMinus)/2 ;
	    }
	    if (basisIndex==_basisCos) {
		return 2*coefficient(basisIndex) ;
	    }
	    
	default:
	    assert(0) ;
    }
    
    return 0 ;
}



Int_t RooBCPSin2bgDecay::getGenerator(const RooArgSet& directVars, 
				       RooArgSet &generateVars) const
{
    if (matchArgs(directVars,generateVars,_t,_tag,_mixState)) return 4 ;  
    if (matchArgs(directVars,generateVars,_t,_mixState)     ) return 3 ;  
    if (matchArgs(directVars,generateVars,_t,_tag)          ) return 2 ;  
    if (matchArgs(directVars,generateVars,_t)               ) return 1 ;  
    return 0 ;
}



void RooBCPSin2bgDecay::initGenerator(Int_t code)
{
    switch (code) {
	case 2:
	{
	    // calculate the fraction of B0-tagged events to generate
	    Double_t sumInt = 
		RooRealIntegral("sumInt","sum integral",*this, 
				RooArgSet(_t.arg(),_tag.arg())).getVal();
	    _tag = 1 ; // B0
	    Double_t flavInt = 
		RooRealIntegral("flavInt","flavor integral",*this, 
				RooArgSet(_t.arg())).getVal();
	    _genFlavFrac = flavInt/sumInt;
	    break;
	}
	case 3:
	{
	    // calculate the fraction of mixed events to generate
	    Double_t sumInt = 
		RooRealIntegral("sumInt","sum integral",*this, 
				RooArgSet(_t.arg(),_mixState.arg())).getVal();
	    _mixState = -1 ; // mixed
	    Double_t mixInt = 
		RooRealIntegral("mixInt","mix integral",*this, 
				RooArgSet(_t.arg())).getVal();
	    _genMixFrac = mixInt/sumInt;
	    break;
	}
	case 4:
	{
	    // calculate the fraction of mixed events to generate
	    Double_t sumInt = 
		RooRealIntegral("sumInt","sum integral",*this, 
				RooArgSet(_t.arg(),_mixState.arg(),
					  _tag.arg())).getVal();
	    _mixState = -1 ; // mixed
	    Double_t mixInt = 
		RooRealIntegral("mixInt","mix integral",*this, 
				RooArgSet(_t.arg(),_tag.arg())).getVal();
	    _genMixFrac = mixInt/sumInt;

	    // calculate the fraction of B0 tagged for mixed and unmixed
	    // events to generate
	    RooRealIntegral dtInt("dtInt","dtintegral",*this, 
				  RooArgSet(_t.arg())) ;
	    _mixState = -1 ; // mixed
	    _tag      =  1 ; // B0
	    _genFlavFracMix   = dtInt.getVal() / mixInt ;
	    _mixState =  1 ; // unmixed
	    _tag      =  1 ; // B0
	    _genFlavFracUnmix = dtInt.getVal() / (sumInt - mixInt) ;
	    break;
	}
    }
}


void RooBCPSin2bgDecay::generateEvent(Int_t code)
{
    // Generate mix-state and/or tag-state dependent
    switch (code) { 
	case 2:
	{
	    Double_t rand = RooRandom::uniform() ;
	    _tag = (Int_t) ((rand<=_genFlavFrac) ? 1 : -1 );
	    break;
	}
	case 3:
	{
	    Double_t rand = RooRandom::uniform() ;
	    _mixState = (Int_t)( (rand<=_genMixFrac) ? -1 : 1) ;
	break;
	}
	case 4:
	{
	    Double_t rand = RooRandom::uniform() ;
	    _mixState =(Int_t) ( (rand<=_genMixFrac) ? -1 : 1) ;
		    
	    rand = RooRandom::uniform() ;
	    Double_t genFlavFrac = (_mixState==-1) ? _genFlavFracMix 
		: _genFlavFracUnmix ;
	    _tag = (Int_t) ((rand<=genFlavFrac) ? 1 : -1) ;
	    break;
	}
    }
    
    // Generate delta-t dependent
    while(1) {
	Double_t rand = RooRandom::uniform() ;
	Double_t tval(0) ;
	
	switch(_type) {
	    case SingleSided:
		tval = -_tau*log(rand);
		break ;
	    case Flipped:
		tval= +_tau*log(rand);
		break ;
	    case DoubleSided:
		tval = 
		    (rand<=0.5) ? -_tau*log(2*rand) : +_tau*log(2*(rand-0.5)) ;
		break ;
	}

	// Accept event if T is in generated range
	Double_t dil = 1-2*_avgMistag ;
	Double_t al2 = _absLambda*_absLambda ;
	Double_t maxArgLambda = 
	    ( fabs(_argLambdaPlus)>fabs(_argLambdaMinus) ) ? 
	    fabs(_argLambdaPlus) : fabs(_argLambdaMinus) ;
	
	Double_t argLambda = 
	    (_mixState*_tag > 0) ? _argLambdaPlus : _argLambdaMinus ;
	
	Double_t maxAcceptProb = 
	    (1+fabs(_delMistag))*(1+al2) 
	    + fabs(dil*(1-al2)) 
	    + fabs(2*_CPeigenval*_absLambda*maxArgLambda) ;

	Double_t acceptProb =   
	    (1-_tag*_delMistag)*(1+al2) 
	    + _mixState*dil*(1-al2)*cos(_dm*tval)
	    + _tag*2*dil*_CPeigenval*_absLambda*argLambda*sin(_dm*tval) ;

	// paranoid check
	if ( acceptProb > maxAcceptProb ) {
	    cout << "RooBCPSin2bgDecay::generateEvent: "
		 << "acceptProb > maxAcceptProb !!!" << endl;
	    assert(0);
	}
	
	Bool_t accept = 
	    maxAcceptProb*RooRandom::uniform() < acceptProb ? kTRUE : kFALSE ;
	
	if (tval<_t.max() && tval>_t.min() && accept) {
	    _t = tval ;
	    break ;
	}
    }
}

