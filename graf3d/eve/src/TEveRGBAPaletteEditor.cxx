// @(#)root/eve:$Id$
// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TEveRGBAPaletteEditor.h"
#include "TEveRGBAPalette.h"
#include "TEveGValuators.h"

#include "TVirtualPad.h"
#include "TColor.h"

#include "TGLabel.h"
#include "TGButton.h"
#include "TGComboBox.h"
#include "TGColorSelect.h"
#include "TGSlider.h"
#include "TGDoubleSlider.h"

//______________________________________________________________________________
//
// Sub-editor for TEveRGBAPalette class.

ClassImp(TEveRGBAPaletteSubEditor)

//______________________________________________________________________________
TEveRGBAPaletteSubEditor::TEveRGBAPaletteSubEditor(const TGWindow* p) :
   TGVerticalFrame(p),

   fM(0),

   fUnderflowAction (0),
   fUnderColor      (0),
   fOverflowAction  (0),
   fOverColor       (0),

   fMinMax(0),

   fInterpolate(0),
   fShowDefValue(0),
   fDefaultColor(0),
   fFixColorRange(0)
{
   // Constructor.

   // Int_t labelW = 42;

   {
      TGHorizontalFrame* f = new TGHorizontalFrame(this);

      fInterpolate = new TGCheckButton(f, "Interpolate");
      f->AddFrame(fInterpolate, new TGLayoutHints(kLHintsLeft, 3, 1, 1, 0));
      fInterpolate->Connect("Toggled(Bool_t)",
                            "TEveRGBAPaletteSubEditor", this, "DoInterpolate()");

      AddFrame(f, new TGLayoutHints(kLHintsTop, 1, 1, 1, 0));
   }

   {
      TGHorizontalFrame* f = new TGHorizontalFrame(this);

      fShowDefValue = new TGCheckButton(f, "Show default value");
      f->AddFrame(fShowDefValue, new TGLayoutHints(kLHintsLeft, 3, 1, 1, 0));
      fShowDefValue->Connect("Toggled(Bool_t)",
                             "TEveRGBAPaletteSubEditor", this, "DoShowDefValue()");

      fDefaultColor = new TGColorSelect(f, 0, -1);
      f->AddFrame(fDefaultColor, new TGLayoutHints(kLHintsLeft|kLHintsTop, 0, 0, 0, 0));
      fDefaultColor->Connect("ColorSelected(Pixel_t)",
                             "TEveRGBAPaletteSubEditor", this, "DoDefaultColor(Pixel_t)");

      AddFrame(f, new TGLayoutHints(kLHintsTop, 1, 1, 2, 0));
   }

   {
      TGHorizontalFrame* f = new TGHorizontalFrame(this);

      fFixColorRange = new TGCheckButton(f, "Fix color range");
      f->AddFrame(fFixColorRange, new TGLayoutHints(kLHintsLeft, 3, 1, 0, 0));
      fFixColorRange->Connect("Toggled(Bool_t)",
                              "TEveRGBAPaletteSubEditor", this, "DoFixColorRange()");

      AddFrame(f, new TGLayoutHints(kLHintsTop, 1, 1, 0, 2));
   }

   { // Underflow
      TGHorizontalFrame* f = new TGHorizontalFrame(this);
      TGLabel* lab = new TGLabel(f, "Underflow:");
      f->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 1, 15, 1, 2));
      fUnderflowAction = new TGComboBox(f);
      fUnderflowAction->AddEntry("Cut", 0);
      fUnderflowAction->AddEntry("Mark", 1);
      fUnderflowAction->AddEntry("Clip", 2);
      fUnderflowAction->AddEntry("Wrap", 3);
      TGListBox* lb = fUnderflowAction->GetListBox();
      lb->Resize(lb->GetWidth(), 4*16);
      fUnderflowAction->Resize(59, 20);
      fUnderflowAction->Connect("Selected(Int_t)", "TEveRGBAPaletteSubEditor", this,
                                "DoUnderflowAction(Int_t)");
      f->AddFrame(fUnderflowAction, new TGLayoutHints(kLHintsLeft, 1, 2, 1, 1));

      fUnderColor = new TGColorSelect(f, 0, -1);
      f->AddFrame(fUnderColor, new TGLayoutHints(kLHintsLeft|kLHintsTop, 1, 1, 0, 2));
      fUnderColor->Connect("ColorSelected(Pixel_t)",
                           "TEveRGBAPaletteSubEditor", this, "DoUnderColor(Pixel_t)");

      AddFrame(f);
   }

   { // Overflow
      TGHorizontalFrame* f = new TGHorizontalFrame(this);
      TGLabel* lab = new TGLabel(f, "Overflow:");
      f->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 1, 20, 1, 2));
      fOverflowAction = new TGComboBox(f);
      fOverflowAction->AddEntry("Cut", 0);
      fOverflowAction->AddEntry("Mark", 1);
      fOverflowAction->AddEntry("Clip", 2);
      fOverflowAction->AddEntry("Wrap", 3);
      TGListBox* lb = fOverflowAction->GetListBox();
      lb->Resize(lb->GetWidth(), 4*16);
      fOverflowAction->Resize(59, 20);
      fOverflowAction->Connect("Selected(Int_t)", "TEveRGBAPaletteSubEditor", this,
                               "DoOverflowAction(Int_t)");
      f->AddFrame(fOverflowAction, new TGLayoutHints(kLHintsLeft, 1, 2, 1, 1));

      fOverColor = new TGColorSelect(f, 0, -1);
      f->AddFrame(fOverColor, new TGLayoutHints(kLHintsLeft|kLHintsTop, 1, 1, 0, 2));
      fOverColor->Connect("ColorSelected(Pixel_t)",
                          "TEveRGBAPaletteSubEditor", this, "DoOverColor(Pixel_t)");

      AddFrame(f);
   }

   fMinMax = new TEveGDoubleValuator(this,"Main range:", 130, 0);
   fMinMax->SetNELength(5);
   fMinMax->SetLabelWidth(74);
   fMinMax->Build();
   fMinMax->GetSlider()->SetWidth(190);
   fMinMax->SetLimits(0, 1023, TGNumberFormat::kNESInteger);
   fMinMax->Connect("ValueSet()",
                    "TEveRGBAPaletteSubEditor", this, "DoMinMax()");
   AddFrame(fMinMax, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));

}

/******************************************************************************/

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::SetModel(TEveRGBAPalette* p)
{
   // Set model object.

   fM = p;

   fMinMax->SetValues(fM->fMinVal, fM->fMaxVal);
   fMinMax->SetLimits(fM->fLowLimit, fM->fHighLimit);

   fInterpolate  ->SetState(fM->fInterpolate ? kButtonDown : kButtonUp);
   fShowDefValue ->SetState(fM->fShowDefValue ? kButtonDown : kButtonUp);
   fDefaultColor ->SetColor(TColor::Number2Pixel(fM->GetDefaultColor()), kFALSE);
   fFixColorRange->SetState(fM->fFixColorRange ? kButtonDown : kButtonUp);

   fUnderColor->SetColor(TColor::Number2Pixel(fM->GetUnderColor()), kFALSE);
   fOverColor ->SetColor(TColor::Number2Pixel(fM->GetOverColor()), kFALSE);

   fUnderflowAction->Select(fM->fUnderflowAction, kFALSE);
   fOverflowAction ->Select(fM->fOverflowAction, kFALSE);
}

/******************************************************************************/

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::Changed()
{
   // Emit "Changed()" signal.

   Emit("Changed()");
}

/******************************************************************************/

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoMinMax()
{
   // Slot for MinMax.

   fM->SetMinMax((Int_t) fMinMax->GetMin(), (Int_t) fMinMax->GetMax());
   Changed();
}

/******************************************************************************/

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoInterpolate()
{
   // Slot for Interpolate.

   fM->SetInterpolate(fInterpolate->IsOn());
   Changed();
}

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoShowDefValue()
{
   // Slot for ShowDefValue.

   fM->SetShowDefValue(fShowDefValue->IsOn());
   Changed();
}

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoDefaultColor(Pixel_t color)
{
   // Slot for DefaultColor.

   fM->SetDefaultColorPixel(color);
   Changed();
}

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoFixColorRange()
{
   // Slot for FixColorRange.

   fM->SetFixColorRange(fFixColorRange->IsOn());
   Changed();
}

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoUnderColor(Pixel_t color)
{
   // Slot for UnderColor.

   fM->SetUnderColorPixel(color);
   Changed();
}

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoOverColor(Pixel_t color)
{
   // Slot for OverColor.

   fM->SetOverColorPixel(color);
   Changed();
}

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoUnderflowAction(Int_t mode)
{
   // Slot for UnderflowAction.

   fM->SetUnderflowAction(mode);
   Changed();
}

//______________________________________________________________________________
void TEveRGBAPaletteSubEditor::DoOverflowAction(Int_t mode)
{
   // Slot for OverflowAction.

   fM->SetOverflowAction(mode);
   Changed();
}


//______________________________________________________________________________
//
// Editor for TEveRGBAPalette class.

ClassImp(TEveRGBAPaletteEditor)

//______________________________________________________________________________
TEveRGBAPaletteEditor::TEveRGBAPaletteEditor(const TGWindow *p, Int_t width, Int_t height,
                                             UInt_t options, Pixel_t back) :
   TGedFrame(p, width, height, options | kVerticalFrame, back),
   fM (0),
   fSE(0)
{
   // Constructor.

   MakeTitle("TEveRGBAPalette");

   fSE = new TEveRGBAPaletteSubEditor(this);
   AddFrame(fSE, new TGLayoutHints(kLHintsTop, 2, 0, 2, 2));
   fSE->Connect("Changed()", "TEveRGBAPaletteEditor", this, "Update()");
}

/******************************************************************************/

//______________________________________________________________________________
void TEveRGBAPaletteEditor::SetModel(TObject* obj)
{
   // Set model object.

   fM = dynamic_cast<TEveRGBAPalette*>(obj);
   fSE->SetModel(fM);
}
