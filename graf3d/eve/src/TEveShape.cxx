// @(#)root/eve:$Id$
// Author: Matevz Tadel, 2010

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveShape.h"
#include "Riostream.h"

//______________________________________________________________________________
//
// Abstract base-class for 2D/3D shapes.
// It provides:
// - fill color / transparency, accessible via Get/SetMainColor/Transparency;
// - frame line color / width;
// - flag if frame should be drawn;
// - flag specifying whether frame or whole shape should be emphasised for
//   highlight.

ClassImp(TEveShape);

//______________________________________________________________________________
TEveShape::TEveShape(const char* n, const char* t) :
   TEveElementList(n, t),
   fFillColor(5),
   fLineColor(3),
   fLineWidth(1),
   fDrawFrame(kTRUE),
   fHighlightFrame(kTRUE)
{
   // Constructor.

   SetMainColorPtr(&fFillColor);
}

//______________________________________________________________________________
TEveShape::~TEveShape()
{
   // Destructor.
}

//______________________________________________________________________________
void TEveShape::SetMainColor(Color_t color)
{
   // Set main color.
   // Override so that line-color can also be changed if it is equal
   // to fill color (which is treated as main color).

   if (fFillColor == fLineColor) {
      fLineColor = color;
      StampObjProps();
   }
   TEveElementList::SetMainColor(color);
}

//______________________________________________________________________________
void TEveShape::CopyVizParams(const TEveElement* el)
{
   // Copy visualization parameters from element el.

   const TEveShape* m = dynamic_cast<const TEveShape*>(el);
   if (m)
   {
      fFillColor = m->fFillColor;
      fLineColor = m->fLineColor;
      fLineWidth = m->fLineWidth;
      fDrawFrame      = m->fDrawFrame;
      fHighlightFrame = m->fHighlightFrame;
   }

   TEveElementList::CopyVizParams(el);
}

//______________________________________________________________________________
void TEveShape::WriteVizParams(ostream& out, const TString& var)
{
   // Write visualization parameters.

   TEveElementList::WriteVizParams(out, var);

   TString t = "   " + var + "->";
   out << t << "SetFillColor(" << fFillColor << ");\n";
   out << t << "SetLineColor(" << fLineColor << ");\n";
   out << t << "SetLineWidth(" << fLineWidth << ");\n";
   out << t << "SetDrawFrame("      << ToString(fDrawFrame) << ");\n";
   out << t << "SetHighlightFrame(" << ToString(fHighlightFrame) << ");\n";
}

//______________________________________________________________________________
void TEveShape::Paint(Option_t*)
{
   // Paint this object. Only direct rendering is supported.

   PaintStandard(this);
}

//==============================================================================

//______________________________________________________________________________
Int_t TEveShape::FindConvexHull(const vPoint_t& pin, vPoint_t& pout, TEveElement* caller)
{
   // Determines the convex-hull of points in pin.
   //
   // Adds the hull points to pout and returns the number of added points.
   // If size of pout is less then 3 then either the number of input points
   // was too low or they were degenerate so that the hull is actually a line
   // segment or even a point.

   Int_t N = pin.size();

   // Find the minimum (bottom-left) point.
   Int_t min_point = 0;
   for (Int_t i = 1; i < N; ++i)
   {
      if (pin[i].fY < pin[min_point].fY || (pin[i].fY == pin[min_point].fY && pin[i].fX < pin[min_point].fX))
         min_point = i;
   }

   // Calculate angles and sort.
   std::vector<Float_t> angles(N);
   for (Int_t i = 0; i < N; ++i)
   {
      angles[i] = (pin[i] - pin[min_point]).Phi();
   }
   std::vector<Int_t> idcs(N);
   TMath::Sort(N, &angles[0], &idcs[0], kFALSE);

   // Weed out points with the same angle -- keep the furthest only.
   // The first point must stay.
   if (N > 2)
   {
      std::vector<Int_t> new_idcs;
      new_idcs.push_back(idcs[0]);
      std::vector<Int_t>::iterator a, b;
      a = idcs.begin(); ++a;
      b = a; ++b;
      while (b != idcs.end())
      {
         if (TMath::Abs(angles[*a] - angles[*b]) < 1e-5f)
         {
            if (pin[idcs[0]].SquareDistance(pin[*a]) < pin[idcs[0]].SquareDistance(pin[*b]))
               a = b;
         }
         else
         {
            new_idcs.push_back(*a);
            a = b;
         }
         ++b;
      }
      new_idcs.push_back(*a);
      idcs.swap(new_idcs);
   }

   N = idcs.size();

   // Find hull.
   std::vector<Int_t> hull;
   if (N > 2)
   {
      hull.push_back(idcs[0]);
      hull.push_back(idcs[1]);
      hull.push_back(idcs[2]);
      {
         Int_t i = 3;
         while (i < N)
         {
            Int_t n = hull.size() - 1;
            if ((pin[hull[n]] - pin[hull[n-1]]).Cross(pin[idcs[i]] - pin[hull[n]]) > 0)
            {
               hull.push_back(idcs[i]);
               ++i;
            }
            else
            {
               hull.pop_back();
            }
         }
      }
   }
   else
   {
      ::Warning("TEveShape::FindConvexHull()", "Polygon reduced to %d points. for '%s'.",
              N, caller ? caller->GetElementName() : "unknown");
      hull.swap(idcs);
   }

   // Add hull points into the output vector.
   N = hull.size();
   Int_t Nold = pout.size();
   pout.resize(Nold + N);
   for (Int_t i = 0; i < N; ++i)
   {
      pout[Nold + i] = pin[hull[i]];
   }

   // Print the hull.
   // for (Int_t i = 0; i < N; ++i)
   // {
   //    const TEvePoint &p = pin[hull[i]];
   //    printf("%d [%d] (%5.1f, %5.1f) %f\n", i, hull[i], p.fX, p.fY, angles[hull[i]]);
   // }

   return N;
}