// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveWindow
#define ROOT_TEveWindow

#include "TEveElement.h"

#include "TGFrame.h"

class TEveWindow;
class TEveWindowSlot;
class TEveWindowFrame;
class TEveWindowMainFrame;
class TEveWindowPack;
class TEveWindowTab;

class TContextMenu;

class TGButton;
class TGSplitButton;
class TGTextButton;

class TGPack;
class TGTab;

//==============================================================================
// TEveCompositeFrame
//==============================================================================

class TEveCompositeFrame : public TGCompositeFrame
{
   friend class TEveWindow;

private:
   TEveCompositeFrame(const TEveCompositeFrame&);            // Not implemented
   TEveCompositeFrame& operator=(const TEveCompositeFrame&); // Not implemented

protected:
   TGCompositeFrame  *fTopFrame;
   TGTextButton      *fToggleBar;
   TGTextButton      *fTitleBar;
   TGTextButton      *fIconBar;
   TGLayoutHints     *fEveWindowLH;

   TGButton          *fMiniBar;

   TEveWindow        *fEveParentWindow;
   TEveWindow        *fEveWindow;

   static TContextMenu *fgCtxMenu;

public:
   TEveCompositeFrame(TGCompositeFrame* gui_parent, TEveWindow* eve_parent);
   virtual ~TEveCompositeFrame();

   virtual void Destroy();

   virtual void        AcquireEveWindow(TEveWindow* ew);
   virtual TEveWindow* RelinquishEveWindow();

   virtual TEveWindow* ChangeEveWindow(TEveWindow* ew);

   virtual void SetCurrent(Bool_t curr);
   virtual void SetShowTitleBar(Bool_t show);

   void ActionPressed();
   void FlipTitleBarState();
   void TitleBarClicked();

   ClassDef(TEveCompositeFrame, 0); // Composite frame containing eve-window-controls and eve-windows.
};


//==============================================================================
// TEveCompositeFrameInMainFrame
//==============================================================================

class TEveCompositeFrameInMainFrame : public TEveCompositeFrame
{
private:
   TEveCompositeFrameInMainFrame(const TEveCompositeFrameInMainFrame&);            // Not implemented
   TEveCompositeFrameInMainFrame& operator=(const TEveCompositeFrameInMainFrame&); // Not implemented

protected:
   TGMainFrame      *fMainFrame;

public:
   TEveCompositeFrameInMainFrame(TGCompositeFrame* parent, TEveWindow* eve_parent,
                                 TGMainFrame* mf);
   virtual ~TEveCompositeFrameInMainFrame();

   virtual void Destroy();

   virtual void        AcquireEveWindow(TEveWindow* ew);
   virtual TEveWindow* RelinquishEveWindow();

   void MainFrameClosed();

   ClassDef(TEveCompositeFrameInMainFrame, 0); // Eve-composite-frame that is contained in one tab of a TGTab.
};


//==============================================================================
// TEveCompositeFrameInPack
//==============================================================================

class TEveCompositeFrameInPack : public TEveCompositeFrame
{
private:
   TEveCompositeFrameInPack(const TEveCompositeFrameInPack&);            // Not implemented
   TEveCompositeFrameInPack& operator=(const TEveCompositeFrameInPack&); // Not implemented

protected:
   TGPack        *fPack;

public:
   TEveCompositeFrameInPack(TGCompositeFrame* parent, TEveWindow* eve_parent,
                            TGPack* pack);
   virtual ~TEveCompositeFrameInPack();

   virtual void Destroy();

   ClassDef(TEveCompositeFrameInPack, 0); // Eve-composite-frame that is contained in a TGPack.
};


//==============================================================================
// TEveCompositeFrameInTab
//==============================================================================

class TEveCompositeFrameInTab : public TEveCompositeFrame
{
private:
   TEveCompositeFrameInTab(const TEveCompositeFrameInTab&);            // Not implemented
   TEveCompositeFrameInTab& operator=(const TEveCompositeFrameInTab&); // Not implemented

protected:
   TGTab            *fTab;
   TGCompositeFrame *fParentInTab;

   Int_t FindTabIndex();

public:
   TEveCompositeFrameInTab(TGCompositeFrame* parent, TEveWindow* eve_parent,
                           TGTab* tab);
   virtual ~TEveCompositeFrameInTab();

   virtual void Destroy();

   virtual void        AcquireEveWindow(TEveWindow* ew);
   virtual TEveWindow* RelinquishEveWindow();

   virtual void SetCurrent(Bool_t curr);

   ClassDef(TEveCompositeFrameInTab, 0); // Eve-composite-frame that is contained in one tab of a TGTab.
};


//==============================================================================
//==============================================================================
// TEveWindow classes
//==============================================================================
//==============================================================================


//==============================================================================
// TEveWindow
//==============================================================================

class TEveWindow : public TEveElementList
{
private:
   TEveWindow(const TEveWindow&);            // Not implemented
   TEveWindow& operator=(const TEveWindow&); // Not implemented

protected:
   TEveCompositeFrame  *fEveFrame;
   Bool_t               fShowTitleBar;

   static TEveWindow   *fgCurrentWindow;

public:
   TEveWindow(const Text_t* n="TEveWindow", const Text_t* t="");
   virtual ~TEveWindow();

   virtual TGFrame*        GetGUIFrame() = 0;

   virtual Bool_t          CanMakeNewSlots() const { return kFALSE; }
   virtual TEveWindowSlot* NewSlot() { return 0; }

   void PopulateSlot(TEveCompositeFrame* ef); 

   void SwapWindow(TEveWindow* w);
   void SwapWindowWithCurrent();        // *MENU*

   virtual void DestroyWindow();        // *MENU*
   virtual void DestroyWindowAndSlot(); // *MENU*

   TEveCompositeFrame* GetEveFrame()  { return fEveFrame; }
   void                ClearEveFrame();

   void   FlipShowTitleBar()      { SetShowTitleBar(!fShowTitleBar); }
   Bool_t GetShowTitleBar() const { return fShowTitleBar; }
   void   SetShowTitleBar(Bool_t x);

   Bool_t       IsCurrent() const { return fgCurrentWindow == this; }
   virtual void SetCurrent(Bool_t curr);

   void TitleBarClicked();

   // Static helper functions for common window management scenarios.

   static TEveWindowSlot* CreateDefaultWindowSlot();
   static TEveWindowSlot* CreateWindowMainFrame(TEveWindow* eve_parent=0);
   static TEveWindowSlot* CreateWindowInTab(TGTab* tab, TEveWindow* eve_parent=0);

   static UInt_t  fgMainFrameDefWidth;
   static UInt_t  fgMainFrameDefHeight;

   static Pixel_t fgCurrentBackgroundColor;
   static Pixel_t fgMiniBarBackgroundColor;

   ClassDef(TEveWindow, 0); // Abstract base-class for eve-windows.
};


//==============================================================================
// TEveWindowSlot
//==============================================================================

class TEveWindowSlot : public TEveWindow
{
private:
   TEveWindowSlot(const TEveWindowSlot&);            // Not implemented
   TEveWindowSlot& operator=(const TEveWindowSlot&); // Not implemented

protected:
   TGTextButton      *fEmptyButt;
   TGCompositeFrame  *fEmbedBuffer;

public:
   TEveWindowSlot(const Text_t* n="TEveWindowSlot", const Text_t* t="");
   virtual ~TEveWindowSlot();

   virtual TGFrame* GetGUIFrame();

   virtual void SetCurrent(Bool_t curr);

   TEveWindowPack* MakePack(); // *MENU*
   TEveWindowTab*  MakeTab();  // *MENU*

   void             StartEmbedding();
   TEveWindowFrame* StopEmbedding();

   ClassDef(TEveWindowSlot, 0); // An unoccupied eve-window slot.
};


//==============================================================================
// TEveWindowFrame
//==============================================================================

class TEveWindowFrame : public TEveWindow
{
private:
   TEveWindowFrame(const TEveWindowFrame&);            // Not implemented
   TEveWindowFrame& operator=(const TEveWindowFrame&); // Not implemented

protected:
   TGFrame         *fGUIFrame;

public:
   TEveWindowFrame(TGFrame* f, const Text_t* n="TEveWindowFrame", const Text_t* t="");
   virtual ~TEveWindowFrame();

   virtual TGFrame* GetGUIFrame() { return fGUIFrame; }

   ClassDef(TEveWindowFrame, 0); // Eve-window containing any TGFrame.
};


//==============================================================================
// TEveWindowPack
//==============================================================================

class TEveWindowPack : public TEveWindow
{
private:
   TEveWindowPack(const TEveWindowPack&);            // Not implemented
   TEveWindowPack& operator=(const TEveWindowPack&); // Not implemented

protected:
   TGPack          *fPack;

public:
   TEveWindowPack(TGPack* p, const Text_t* n="TEveWindowPack", const Text_t* t="");
   virtual ~TEveWindowPack();

   virtual TGFrame*        GetGUIFrame();

   virtual Bool_t          CanMakeNewSlots() const { return kTRUE; }
   virtual TEveWindowSlot* NewSlot(); // *MENU*

   void FlipOrientation(); // *MENU*

   TGPack* GetPack() const { return fPack; }

   ClassDef(TEveWindowPack, 0); // Eve-window containing a TGPack.
};


//==============================================================================
// TEveWindowTab
//==============================================================================

class TEveWindowTab : public TEveWindow
{
private:
   TEveWindowTab(const TEveWindowTab&);            // Not implemented
   TEveWindowTab& operator=(const TEveWindowTab&); // Not implemented

protected:
   TGTab           *fTab;

public:
   TEveWindowTab(TGTab* tab, const Text_t* n="TEveWindowTab", const Text_t* t="");
   virtual ~TEveWindowTab();

   virtual TGFrame*        GetGUIFrame();

   virtual Bool_t          CanMakeNewSlots() const { return kTRUE; }
   virtual TEveWindowSlot* NewSlot(); // *MENU*

   TGTab* GetTab() const { return fTab; }

   ClassDef(TEveWindowTab, 0); // Eve-window containing a TGTab.
};

#endif
