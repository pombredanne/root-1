/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 * @(#)root/roofitcore:$Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// 
// Class RooXYChi2Var implements a simple chi^2 calculation from a unbinned
// dataset with values x,y with errors on y (and optionally on x) and a function.
// The function can be either a RooAbsReal, or an extended RooAbsPdf where
// the function value is calculated as the probability density times the
// expected number of events
// The chi^2 is calculated as 
//
//              / (Data[y]-) - func \+2
//  Sum[point] |  ------------------ |
//              \     Data[ErrY]    /
//
//

#include "RooFit.h"

#include "RooXYChi2Var.h"
#include "RooDataSet.h"
#include "RooAbsReal.h"

#include "Riostream.h"

#include "RooRealVar.h"

#include "RooGaussKronrodIntegrator1D.h"
#include "RooRealBinding.h"
#include "RooNumIntFactory.h"

ClassImp(RooXYChi2Var)
;


//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var() 
{
  // coverity[UNINIT_CTOR]
  _funcInt = 0 ;
  _rrvIter = _rrvArgs.createIterator() ;
}


//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var(const char *name, const char* title, RooAbsReal& func, RooDataSet& xydata, Bool_t integrate) :
  RooAbsOptTestStatistic(name,title,func,xydata,RooArgSet(),0,0,1,1,0,0), 
  _extended(kFALSE),
  _integrate(integrate),
  _intConfig(*defaultIntegratorConfig()),
  _funcInt(0)
{
  //
  //  RooXYChi2Var constructor with function and X-Y values dataset
  //
  // An X-Y dataset is a weighted dataset with one or more observables X where the weight is interpreted
  // as the Y value and the weight error is interpreted as the Y value error. The weight must have an
  // non-zero error defined at each point for the chi^2 calculation to be meaningful.
  //
  // To store errors associated with the x and y values in a RooDataSet, call RooRealVar::setAttribute("StoreError")
  // on each X-type observable for which the error should be stored and add datapoints to the dataset as follows
  //
  // RooDataSet::add(xset,yval,yerr) where xset is the RooArgSet of x observables (with or without errors) and yval and yerr
  //                                 are the Double_t values that correspond to the Y and its error
  //
  _extended = kFALSE ;
  _yvar = 0 ;

  initialize() ;
}


//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var(const char *name, const char* title, RooAbsReal& func, RooDataSet& xydata, RooRealVar& yvar, Bool_t integrate) :
  RooAbsOptTestStatistic(name,title,func,xydata,RooArgSet(),0,0,1,1,0,0), 
  _extended(kFALSE),
  _integrate(integrate),
  _intConfig(*defaultIntegratorConfig()),
  _funcInt(0)  
{
  //
  //  RooXYChi2Var constructor with function and X-Y values dataset
  //
  // An X-Y dataset is a weighted dataset with one or more observables X where given yvar is interpreted
  // as the Y value. The Y variable must have a non-zero error defined at each point for the chi^2 calculation to be meaningful.
  //
  // To store errors associated with the x and y values in a RooDataSet, call RooRealVar::setAttribute("StoreError")
  // on each X-type observable for which the error should be stored and add datapoints to the dataset as follows
  //
  // RooDataSet::add(xset,yval,yerr) where xset is the RooArgSet of x observables (with or without errors) and yval and yerr
  //                                 are the Double_t values that correspond to the Y and its error
  //
  _extended = kFALSE ;
  _yvar = (RooRealVar*) _dataClone->get()->find(yvar.GetName()) ;

  initialize() ;
}


//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var(const char *name, const char* title, RooAbsPdf& extPdf, RooDataSet& xydata, Bool_t integrate) :
  RooAbsOptTestStatistic(name,title,extPdf,xydata,RooArgSet(),0,0,1,1,0,0), 
  _extended(kTRUE),
  _integrate(integrate),
  _intConfig(*defaultIntegratorConfig()),
  _funcInt(0)
{
  //
  // RooXYChi2Var constructor with an extended p.d.f. and X-Y values dataset
  // The value of the function that defines the chi^2 in this form is takes as
  // the p.d.f. times the expected number of events
  //
  // An X-Y dataset is a weighted dataset with one or more observables X where the weight is interpreted
  // as the Y value and the weight error is interpreted as the Y value error. The weight must have an
  // non-zero error defined at each point for the chi^2 calculation to be meaningful.
  //
  // To store errors associated with the x and y values in a RooDataSet, call RooRealVar::setAttribute("StoreError")
  // on each X-type observable for which the error should be stored and add datapoints to the dataset as follows
  //
  // RooDataSet::add(xset,yval,yerr) where xset is the RooArgSet of x observables (with or without errors) and yval and yerr
  //                                 are the Double_t values that correspond to the Y and its error
  //
  if (!extPdf.canBeExtended()) {
    throw(string(Form("RooXYChi2Var::ctor(%s) ERROR: Input p.d.f. must be an extendible",GetName()))) ;
  }
  _yvar = 0 ;
  initialize() ;
}




//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var(const char *name, const char* title, RooAbsPdf& extPdf, RooDataSet& xydata, RooRealVar& yvar, Bool_t integrate) :
  RooAbsOptTestStatistic(name,title,extPdf,xydata,RooArgSet(),0,0,1,1,0,0), 
  _extended(kTRUE),
  _integrate(integrate),
  _intConfig(*defaultIntegratorConfig()),
  _funcInt(0)
{
  //
  // RooXYChi2Var constructor with an extended p.d.f. and X-Y values dataset
  // The value of the function that defines the chi^2 in this form is takes as
  // the p.d.f. times the expected number of events
  //
  // An X-Y dataset is a weighted dataset with one or more observables X where the weight is interpreted
  // as the Y value and the weight error is interpreted as the Y value error. The weight must have an
  // non-zero error defined at each point for the chi^2 calculation to be meaningful.
  //
  // To store errors associated with the x and y values in a RooDataSet, call RooRealVar::setAttribute("StoreError")
  // on each X-type observable for which the error should be stored and add datapoints to the dataset as follows
  //
  // RooDataSet::add(xset,yval,yerr) where xset is the RooArgSet of x observables (with or without errors) and yval and yerr
  //                                 are the Double_t values that correspond to the Y and its error
  //
  if (!extPdf.canBeExtended()) {
    throw(string(Form("RooXYChi2Var::ctor(%s) ERROR: Input p.d.f. must be an extendible",GetName()))) ;
  }
  _yvar = (RooRealVar*) _dataClone->get()->find(yvar.GetName()) ;
  initialize() ;
}




//_____________________________________________________________________________
RooXYChi2Var::RooXYChi2Var(const RooXYChi2Var& other, const char* name) : 
  RooAbsOptTestStatistic(other,name),
  _extended(other._extended),
  _integrate(other._integrate),
  _intConfig(other._intConfig),
  _funcInt(0)
{
  // Copy constructor

  _yvar = other._yvar ? (RooRealVar*) _dataClone->get()->find(other._yvar->GetName()) : 0 ;
  initialize() ;
  
}




//_____________________________________________________________________________
void RooXYChi2Var::initialize()
{
  // Common constructor initialization

  TIterator* iter = _dataClone->get()->createIterator() ;
  RooAbsArg* arg ;
  while((arg=(RooAbsArg*)iter->Next())) {
    RooRealVar* var = dynamic_cast<RooRealVar*>(arg) ;
    if (var) {
      _rrvArgs.add(*var) ;
    }
  }
  delete iter ;
  _rrvIter = _rrvArgs.createIterator() ;

  // Define alternate numeric integrator configuration for bin integration
  // We expect bin contents to very only very slowly so a non-adaptive
  // Gauss-Kronrod integrator is expected to perform well.
  _intConfig.setEpsRel(1e-7) ;
  _intConfig.setEpsAbs(1e-7) ;
  _intConfig.method1D().setLabel("RooGaussKronrodIntegrator1D") ;
  _intConfig.methodND().setLabel("RooAdaptiveIntegratorND") ;

  initIntegrator() ;
  
}



//_____________________________________________________________________________
void RooXYChi2Var::initIntegrator()
{
  // Initialize bin content integrator

  if (!_funcInt) {
    _funcInt = _funcClone->createIntegral(_rrvArgs,_rrvArgs,_intConfig,"bin") ;
    _rrvIter->Reset() ;
    RooRealVar* x ;
    while((x=(RooRealVar*)_rrvIter->Next())) {
      _binList.push_back(&x->getBinning("bin",kFALSE,kTRUE)) ;
    }
  }
  
}



//_____________________________________________________________________________
RooXYChi2Var::~RooXYChi2Var()
{
  // Destructor

  delete _rrvIter ;
  if (_funcInt) delete _funcInt ;
}




//_____________________________________________________________________________
Double_t RooXYChi2Var::xErrorContribution(Double_t ydata) const
{
  // Calculate contribution to internal error due to error on 'x' coordinates
  // at point i

  RooRealVar* var ;
  Double_t ret(0) ;
  _rrvIter->Reset() ;
  while((var=(RooRealVar*)_rrvIter->Next())) {

    if (var->hasAsymError()) {
      
      // Get value at central X
      Double_t cxval = var->getVal() ;
      Double_t xerrLo = -var->getAsymErrorLo() ;
      Double_t xerrHi = var->getAsymErrorHi() ;
      Double_t xerr = (xerrLo+xerrHi)/2 ;
      
      // Get value at X-eps
      var->setVal(cxval - xerr/100) ;
      Double_t fxmin = fy() ;

      // Get value at X+eps
      var->setVal(cxval + xerr/100) ;
      Double_t fxmax = fy() ;
      
      // Calculate slope
      Double_t slope = (fxmax-fxmin)/(2*xerr/100.) ;
      
      // Asymmetric X error, decide which one to use
      if ((ydata>cxval && fxmax>fxmin) || (ydata<=cxval && fxmax<=fxmin)) {
	// Use right X error
	ret += pow(xerrHi*slope,2) ;
      } else {
	// Use left X error
	ret += pow(xerrLo*slope,2) ;
      }
      
    } else if (var->hasError()) {

      // Get value at central X
      Double_t cxval = var->getVal() ;
      Double_t xerr = var->getError() ;

      // Get value at X-eps
      var->setVal(cxval - xerr/100) ;
      Double_t fxmin = fy() ;

      // Get value at X+eps
      var->setVal(cxval + xerr/100) ;
      Double_t fxmax = fy() ;
      
      // Calculate slope
      Double_t slope = (fxmax-fxmin)/(2*xerr/100.) ;
            
      // Symmetric X error
      ret += pow(xerr*slope,2) ;
    }    
  }
  return ret ;
}




//_____________________________________________________________________________
Double_t RooXYChi2Var::fy() const
{
  // Return function value requested bu present configuration
  //
  // If integration is required, the function value integrated
  // over the bin volume divided by the bin volume is returned,
  // otherwise the value at the bin center is returned. 
  // The bin volume is defined by the error on the 'X' coordinates
  //
  // If an extended p.d.f. is used as function, its value is
  // also multiplied by the expected number of events here

  // Get function value
  Double_t yfunc ;
  if (!_integrate) {
    yfunc = _funcClone->getVal(_dataClone->get()) ;
  } else {
    Double_t volume(1) ;
    _rrvIter->Reset() ;
    for (list<RooAbsBinning*>::const_iterator iter = _binList.begin() ; iter != _binList.end() ; iter++) {
      RooRealVar* x = (RooRealVar*) _rrvIter->Next() ;
      Double_t xmin = x->getVal() + x->getErrorLo() ;
      Double_t xmax = x->getVal() + x->getErrorHi() ;
      (*iter)->setRange(xmin,xmax) ;
      x->setShapeDirty() ;
      volume *= (xmax - xmin) ;
    }
    Double_t ret = _funcInt->getVal() ;
    return ret / volume ;
  }
  if (_extended) {      
    RooAbsPdf* pdf = (RooAbsPdf*) _funcClone ;
    // Multiply with expected number of events
    yfunc *= pdf->expectedEvents(_dataClone->get()) ;
  }      
  return yfunc ;
}



//_____________________________________________________________________________
Double_t RooXYChi2Var::evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const 
{
  // Calculate chi^2 in partition from firstEvent to lastEvent using given stepSize

  Double_t result(0) ;

  // Loop over bins of dataset
  RooDataSet* xydata = (RooDataSet*) _dataClone ;

  for (Int_t i=firstEvent ; i<lastEvent ; i+=stepSize) {
    
    // get the data values for this event
    xydata->get(i);
    
    if (!xydata->valid()) {
      continue ;
    }

    // Get function value
    Double_t yfunc = fy() ;

    // Get data value and error
    Double_t ydata ;
    Double_t eylo,eyhi ;
    if (_yvar) {
      ydata = _yvar->getVal() ;
      eylo = -1*_yvar->getErrorLo() ;
      eyhi = _yvar->getErrorHi() ;
    } else {
      ydata = xydata->weight() ;
      xydata->weightError(eylo,eyhi) ;
    }

    // Calculate external error
    Double_t eExt = yfunc-ydata ;

    // Pick upper or lower error bar depending on sign of external error
    Double_t eInt = (eExt>0) ? eyhi : eylo ;

    // Add contributions due to error in x coordinates    
    Double_t eIntX2 = _integrate ? 0 : xErrorContribution(ydata) ;

    // Return 0 if eInt=0, special handling in MINUIT will follow
    if (eInt==0.) {
      coutE(Eval) << "RooXYChi2Var::RooXYChi2Var(" << GetName() << ") INFINITY ERROR: data point " << i 
		  << " has zero error, but function is not zero (f=" << yfunc << ")" << endl ;
      return 0 ;
    }

    // Add chi2 term
    result += eExt*eExt/(eInt*eInt+ eIntX2) ;
  }

  return result ;
}



RooArgSet RooXYChi2Var::requiredExtraObservables() const 
{
  // Inform base class that observable yvar cannot be optimized away from the dataset
  if (_yvar) return RooArgSet(*_yvar) ;
  return RooArgSet() ;
}



