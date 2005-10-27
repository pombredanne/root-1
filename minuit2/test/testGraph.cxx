// @(#)root/minuit2:$Name:  $:$Id: testGraph.cxx,v 1.1 2005/10/27 14:11:07 brun Exp $
// Author: L. Moneta    10/2005  

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2005 ROOT Foundation,  CERN/PH-SFT                   *
 *                                                                    *
 **********************************************************************/

#include "TApplication.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom3.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TPaveLabel.h"
#include "TStopwatch.h"
#include "TVirtualFitter.h"

#include <vector>

double fitFunc( double *x , double * p) { 

  double A = p[0];
  double B = p[1];
  double C = p[2];
  return    A*TMath::Sin(C*x[0]) + B*TMath::Sin(2*C*x[0]);
}



void makePoints(Int_t n, std::vector<double> &  x, std::vector<double> & y, std::vector<double> & e)
{
  Int_t i;
  TRandom3 r;

  double A = 1; 
  double B = 2; 
  double C = 1; 

  for (i=0; i<n; i++) {
    x[i] = r.Uniform(-2, 2);
    y[i]=A*TMath::Sin(C*x[i]) + B*TMath::Sin(2*C*x[i]) + r.Gaus()*0.3;
    e[i] = 0.1;
  }

}


void doFit(int n,const char * fitter) 
{ 


   std::vector<double> x(n); 
   std::vector<double> y(n); 
   std::vector<double> e(n); 

   double initPar[3] = { 1, 1, 2 };

   //Generate points along a sin(x)+sin(2x) function
   makePoints(n, x , y, e);

   TGraphErrors *gre3 = new TGraphErrors(n, &x.front(), &y.front(), 0, &e.front());
   gre3->SetMarkerStyle(24);
   gre3->SetMarkerSize(0.3);
   gre3->Draw("ap");


   //Fit the graph with the predefined "pol3" function
   TF1 *f = new TF1("f2",fitFunc, -2, 2, 3);

   printf("fitting with %s",fitter);
   TVirtualFitter::SetDefaultFitter(fitter);

   int npass = 100; 
   TStopwatch timer;
   timer.Start();
   for (int i = 0; i < npass; ++i) { 
     f->SetParameters(initPar);
     
     gre3->Fit(f,"q");
   }
   timer.Stop();
   printf("%s,: RT=%7.3f s, Cpu=%7.3f s\n",fitter,timer.RealTime(),timer.CpuTime());

   TPaveLabel *pl = new TPaveLabel(0.5,0.7,0.85,0.8,Form("%s CPU= %g s",fitter,timer.CpuTime()),"brNDC");
   pl->Draw();
   //Access the fit resuts
   TF1 *f3 = gre3->GetFunction("f2");
   if (f3) { 
     f3->SetLineWidth(1);
     f3->SetLineColor(kRed);
   }

   TLegend *leg = new TLegend(0.1, 0.8, 0.35, 0.9);
   leg->AddEntry(gre3, "sin(x) + sin(2*x)", "p");
   leg->Draw();
   leg->SetFillColor(42);

}

void fitGraph(int n = 500) { 
//   TCanvas *myc = new TCanvas("myc", "Fitting 3 TGraphErrors with linear functions");
//   myc->Divide(1,2);
//   myc->SetFillColor(42);
//   myc->SetGrid();

//   myc->cd(1); 
//  doFit(n,"Minuit");

//  myc->cd(2); 
  doFit(n,"Minuit2");

}

#ifndef __CINT__
int main(int argc, char **argv)
{
   TApplication theApp("App", &argc, argv);
   fitGraph(500);
   theApp.Run();
   return 0;
}
#endif
/*
int main() { 
  fitGraph(500); 
}
*/