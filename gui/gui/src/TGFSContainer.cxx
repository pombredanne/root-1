// @(#)root/gui:$Id$
// Author: Fons Rademakers   19/01/98

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
/**************************************************************************

    This source is based on Xclass95, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

    Xclass95 is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

**************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TGFileIcon, TGFileEntry, TGFSContainer                               //
//                                                                      //
// Utility classes used by the file selection dialog (TGFSDialog).      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TGFSContainer.h"
#include "TGIcon.h"
#include "TGMsgBox.h"
#include "TGMimeTypes.h"
#include "TRegexp.h"
#include "TList.h"
#include "TSystem.h"
#include "TGDNDManager.h"
#include "TBufferFile.h"
#include "Riostream.h"
#include "TRemoteObject.h"
#include "snprintf.h"
#include <time.h>

ClassImp(TGFileItem)
ClassImp(TGFileContainer)

class TViewUpdateTimer : public TTimer {

private:
   TGFileContainer   *fContainer;

public:
   TViewUpdateTimer(TGFileContainer *t, Long_t ms) : TTimer(ms, kTRUE) { fContainer = t; }
   Bool_t Notify();
};



class TGFileIcon : public TGIcon {

protected:
   const TGPicture *fLpic;   // icon picture

   virtual void DoRedraw();

public:
   TGFileIcon(const TGWindow *p, const TGPicture *pic, const TGPicture *lpic,
              UInt_t options = kChildFrame, Pixel_t back = GetWhitePixel()) :
      TGIcon(p, pic, 0, 0, options, back) { fLpic = lpic; }
};



//______________________________________________________________________________
class TGFSFrameElement : public TGFrameElement {
public:
   TGFileContainer  *fContainer;   // file container

   Bool_t IsSortable() const { return kTRUE; }
   Int_t  Compare(const TObject *obj) const;
};

//______________________________________________________________________________
Int_t TGFSFrameElement::Compare(const TObject *obj) const
{
   // Sort frame elements in file selection list view container.

   Int_t type1, type2;

   TGFileItem *f1 = (TGFileItem *) fFrame;
   TGFileItem *f2 = (TGFileItem *) ((TGFrameElement *) obj)->fFrame;

   switch (fContainer->fSortType) {
      default:
      case kSortByName:
         //--- this is not exactly what I want...
         type1 = f1->GetType();
         type2 = f2->GetType();

         //--- use posix macros
         if (R_ISDIR(type1)) type1 = 1;
         else                type1 = 6;

         if (R_ISDIR(type2)) type2 = 1;
         else                type2 = 6;

         if (type1 < type2)  return -1;
         if (type1 > type2)  return  1;
         return strcmp(f1->GetItemName()->GetString(),
                       f2->GetItemName()->GetString());

      case kSortByOwner:
         if (f1->GetUid() != f2->GetUid()) {
            if (f1->GetUid() < f2->GetUid())
               return -1;
            else
               return +1;
         }

         // else sort by name
         type1 = f1->GetType();
         type2 = f2->GetType();

         //--- use posix macros
         if (R_ISDIR(type1)) type1 = 1;
         else                type1 = 6;

         if (R_ISDIR(type2)) type2 = 1;
         else                type2 = 6;

         if (type1 < type2)  return -1;
         if (type1 > type2)  return  1;
         return strcmp(f1->GetItemName()->GetString(),
                       f2->GetItemName()->GetString());

      case kSortByGroup:
         if (f1->GetGid() != f2->GetGid()) {
            if (f1->GetGid() < f2->GetGid())
               return -1;
            else
               return +1;
         }

         // else sort by name
         type1 = f1->GetType();
         type2 = f2->GetType();

         //--- use posix macros
         if (R_ISDIR(type1)) type1 = 1;
         else                type1 = 6;

         if (R_ISDIR(type2)) type2 = 1;
         else                type2 = 6;

         if (type1 < type2)  return -1;
         if (type1 > type2)  return  1;
         return strcmp(f1->GetItemName()->GetString(),
                       f2->GetItemName()->GetString());

      case kSortByType:
         //--- this is not exactly what I want...

         type1 = f1->GetType();
         type2 = f2->GetType();

         //--- use posix macros

         if (R_ISDIR(type1))         type1 = 1;
         else if (R_ISLNK(type1))    type1 = 2;
         else if (R_ISSOCK(type1))   type1 = 3;
         else if (R_ISFIFO(type1))   type1 = 4;
         else if (R_ISREG(type1) && (type1 & kS_IXUSR)) type1 = 5;
         else                        type1 = 6;

         if (R_ISDIR(type2))         type2 = 1;
         else if (R_ISLNK(type2))    type2 = 2;
         else if (R_ISSOCK(type2))   type2 = 3;
         else if (R_ISFIFO(type2))   type2 = 4;
         else if (R_ISREG(type2) && (type2 & kS_IXUSR)) type2 = 5;
         else                        type2 = 6;

         if (type1 < type2) return -1;
         if (type1 > type2) return 1;
         return strcmp(f1->GetItemName()->GetString(),
                       f2->GetItemName()->GetString());

      case kSortBySize:
         if (f1->GetSize() < f2->GetSize()) return -1;
         if (f1->GetSize() > f2->GetSize()) return 1;
         return strcmp(f1->GetItemName()->GetString(),
                       f2->GetItemName()->GetString());

      case kSortByDate:
         time_t loctimeF1 = (time_t) f1->GetModTime();
         struct tm tmF1 = *localtime(&loctimeF1);

         time_t loctimeF2 = (time_t) f2->GetModTime();
         struct tm tmF2 = *localtime(&loctimeF2);

         if ( tmF1.tm_year != tmF2.tm_year )
            return (tmF1.tm_year < tmF2.tm_year) ? +1 : -1;
         else if ( tmF1.tm_mon != tmF2.tm_mon )
            return (tmF1.tm_mon < tmF2.tm_mon) ? +1 : -1;
         else if ( tmF1.tm_mday != tmF2.tm_mday )
            return (tmF1.tm_mday < tmF2.tm_mday) ? +1 : -1;
         else if ( tmF1.tm_hour != tmF2.tm_hour )
            return (tmF1.tm_hour < tmF2.tm_hour) ? +1 : -1;
         else if ( tmF1.tm_min != tmF2.tm_min )
            return (tmF1.tm_min < tmF2.tm_min) ? +1 : -1;
         else if ( tmF1.tm_sec != tmF2.tm_sec )
            return (tmF1.tm_sec < tmF2.tm_sec) ? +1 : -1;
         else
            return 0;
   }
}


//______________________________________________________________________________
Bool_t TViewUpdateTimer::Notify()
{
   // Reset the timer.

   fContainer->HandleTimer(0);
   Reset();
   return kFALSE;
}


//______________________________________________________________________________
void TGFileIcon::DoRedraw()
{
   // Draw icon.

   TGIcon::DoRedraw();
   if (fLpic) fLpic->Draw(fId, GetBckgndGC()(), 0, 0);
}


//______________________________________________________________________________
TGFileItem::TGFileItem(const TGWindow *p,
                       const TGPicture *bpic, const TGPicture *blpic,
                       const TGPicture *spic, const TGPicture *slpic,
                       TGString *name, Int_t type, Long64_t size, Int_t uid,
                       Int_t gid, Long_t modtime, EListViewMode viewMode,
                       UInt_t options, ULong_t back) :
   TGLVEntry(p, bpic, spic, name, 0, viewMode, options, back)
{
   // Create a list view item.

   char tmp[256];
   Long64_t fsize, bsize;

   fBuf = 0;
   fDNDData.fData = 0;
   fDNDData.fDataLength = 0;
   fDNDData.fDataType = 0;
   fLcurrent =
   fBlpic = blpic;
   fSlpic = slpic;

   fViewMode = (EListViewMode) -1;
   SetViewMode(viewMode);

   fType = type;
   fSize = size;
   fUid  = uid;
   fGid  = gid;
   fModTime = modtime;

   // FIXME: hack...
   fIsLink = (blpic != 0);

   fSubnames = new TGString* [6];

   // file type
   snprintf(tmp, 255, "%c%c%c%c%c%c%c%c%c%c",
                (fIsLink ?
                 'l' :
                 R_ISREG(type) ?
                  '-' :
                  (R_ISDIR(type) ?
                   'd' :
                    (R_ISCHR(type) ?
                     'c' :
                     (R_ISBLK(type) ?
                      'b' :
                      (R_ISFIFO(type) ?
                       'p' :
                       (R_ISSOCK(type) ?
                        's' : '?' )))))),
                 ((type & kS_IRUSR) ? 'r' : '-'),
                 ((type & kS_IWUSR) ? 'w' : '-'),
                 ((type & kS_ISUID) ? 's' : ((type & kS_IXUSR) ? 'x' : '-')),
                 ((type & kS_IRGRP) ? 'r' : '-'),
                 ((type & kS_IWGRP) ? 'w' : '-'),
                 ((type & kS_ISGID) ? 's' : ((type & kS_IXGRP) ? 'x' : '-')),
                 ((type & kS_IROTH) ? 'r' : '-'),
                 ((type & kS_IWOTH) ? 'w' : '-'),
                 ((type & kS_ISVTX) ? 't' : ((type & kS_IXOTH) ? 'x' : '-')));
   fSubnames[0] = new TGString(tmp);

   // file size
   fsize = bsize = fSize;
   if (fsize > 1024) {
      fsize /= 1024;
      if (fsize > 1024) {
         // 3.7MB is more informative than just 3MB
         snprintf(tmp, 255, "%lld.%lldM", fsize/1024, (fsize%1024)/103);
      } else {
         snprintf(tmp, 255, "%lld.%lldK", bsize/1024, (bsize%1024)/103);
      }
   } else {
      snprintf(tmp, 255, "%lld", bsize);
   }
   fSubnames[1] = new TGString(tmp);

   {
      struct UserGroup_t *user_group;

      user_group = gSystem->GetUserInfo(fUid);
      if (user_group) {
         fSubnames[2] = new TGString(user_group->fUser);
         fSubnames[3] = new TGString(user_group->fGroup);
         delete user_group;
      } else {
         fSubnames[2] = new TGString(TString::Format("%d", fUid));
         fSubnames[3] = new TGString(TString::Format("%d", fGid));
      }
   }

   struct tm *newtime;
   time_t loctime = (time_t) fModTime;
   newtime = localtime(&loctime);
   snprintf(tmp, 255, "%d-%02d-%02d %02d:%02d", newtime->tm_year + 1900,
           newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour,
           newtime->tm_min);
   fSubnames[4] = new TGString(tmp);

   fSubnames[5] = 0;

   int i;
   for (i = 0; fSubnames[i] != 0; ++i)
      ;
   fCtw = new int[i+1];
   fCtw[i] = 0;
   for (i = 0; fSubnames[i] != 0; ++i)
      fCtw[i] = gVirtualX->TextWidth(fFontStruct, fSubnames[i]->GetString(),
                                     fSubnames[i]->GetLength());

   SetWindowName();
}

//______________________________________________________________________________
TGFileItem::~TGFileItem()
{
   // Destructor.

   delete fBuf;
}

//______________________________________________________________________________
void TGFileItem::SetViewMode(EListViewMode viewMode)
{
   // Set container item view mode.

   TGLVEntry::SetViewMode(viewMode);

   if (viewMode == kLVLargeIcons)
      fLcurrent = fBlpic;
   else
      fLcurrent = fSlpic;

   if (fClient) fClient->NeedRedraw(this);
}

//______________________________________________________________________________
void TGFileItem::DoRedraw()
{
   // Draw list view container item.

   int ix, iy;

   TGLVEntry::DoRedraw();
   if (!fLcurrent) return;

   if (fViewMode == kLVLargeIcons) {
      ix = (fWidth - fLcurrent->GetWidth()) >> 1;
      iy = 0;
   } else {
      ix = 0;
      iy = (fHeight - fLcurrent->GetHeight()) >> 1;
   }

   fLcurrent->Draw(fId, fNormGC, ix, iy);
}


//______________________________________________________________________________
TGFileContainer::TGFileContainer(const TGWindow *p, UInt_t w, UInt_t h,
                                 UInt_t options, ULong_t back) :
   TGLVContainer(p, w, h, options, back)
{
   // Create a list view container which will hold the contents of
   // the current directory.

   fSortType  = kSortByName;
   fFilter    = 0;
   fDirectory = gSystem->WorkingDirectory();
   fRefresh   = new TViewUpdateTimer(this, 1000);
   gSystem->AddTimer(fRefresh);
   fCachePictures = kTRUE;
   fDisplayStat   = kTRUE;

   fFolder_s = fClient->GetPicture("folder_s.xpm");
   fFolder_t = fClient->GetPicture("folder_t.xpm");
   fApp_s    = fClient->GetPicture("app_s.xpm");
   fApp_t    = fClient->GetPicture("app_t.xpm");
   fDoc_s    = fClient->GetPicture("doc_s.xpm");
   fDoc_t    = fClient->GetPicture("doc_t.xpm");
   fSlink_s  = fClient->GetPicture("slink_s.xpm");
   fSlink_t  = fClient->GetPicture("slink_t.xpm");

   if (!fFolder_s || !fFolder_t ||
       !fApp_s    || !fApp_t    ||
       !fDoc_s    || !fDoc_t    ||
       !fSlink_s  || !fSlink_t)
      Error("TGFileContainer", "required pixmap(s) missing\n");

   SetWindowName();
}

//______________________________________________________________________________
TGFileContainer::TGFileContainer(TGCanvas *p, UInt_t options, ULong_t back) :
   TGLVContainer(p,options, back)
{
   // Create a list view container which will hold the contents of
   // the current directory.

   fSortType  = kSortByName;
   fFilter    = 0;
   fDirectory = gSystem->WorkingDirectory();
   fRefresh   = new TViewUpdateTimer(this, 1000);
   gSystem->AddTimer(fRefresh);
   fCachePictures = kTRUE;
   fDisplayStat   = kTRUE;

   fFolder_s = fClient->GetPicture("folder_s.xpm");
   fFolder_t = fClient->GetPicture("folder_t.xpm");
   fApp_s    = fClient->GetPicture("app_s.xpm");
   fApp_t    = fClient->GetPicture("app_t.xpm");
   fDoc_s    = fClient->GetPicture("doc_s.xpm");
   fDoc_t    = fClient->GetPicture("doc_t.xpm");
   fSlink_s  = fClient->GetPicture("slink_s.xpm");
   fSlink_t  = fClient->GetPicture("slink_t.xpm");

   if (!fFolder_s || !fFolder_t ||
       !fApp_s    || !fApp_t    ||
       !fDoc_s    || !fDoc_t    ||
       !fSlink_s  || !fSlink_t)
      Error("TGFileContainer", "required pixmap(s) missing\n");

   SetWindowName();
}

//______________________________________________________________________________
TGFileContainer::~TGFileContainer()
{
   // Delete list view file container.

   if (fRefresh) delete fRefresh;
   if (fFilter)  delete fFilter;
   fClient->FreePicture(fFolder_s);
   fClient->FreePicture(fFolder_t);
   fClient->FreePicture(fApp_s);
   fClient->FreePicture(fApp_t);
   fClient->FreePicture(fDoc_s);
   fClient->FreePicture(fDoc_t);
   fClient->FreePicture(fSlink_s);
   fClient->FreePicture(fSlink_t);
}

//______________________________________________________________________________
void TGFileContainer::AddFrame(TGFrame *f, TGLayoutHints *l)
{
   // Add frame to the composite frame.

   TGFSFrameElement *nw;

   nw = new TGFSFrameElement;
   nw->fFrame     = f;
   nw->fLayout    = l ? l : fgDefaultHints;
   nw->fState     = 1;
   nw->fContainer = this;
   fList->Add(nw);
}

//______________________________________________________________________________
Bool_t TGFileContainer::HandleTimer(TTimer *)
{
   // Refresh container contents. Check every 5 seconds to see if the
   // directory modification date has changed.

   FileStat_t sbuf;

   if (gSystem->GetPathInfo(fDirectory, sbuf) == 0)
      if (fMtime != (ULong_t)sbuf.fMtime) DisplayDirectory();

   return kTRUE;
}

//______________________________________________________________________________
void TGFileContainer::SetFilter(const char *filter)
{
   // Set file selection filter.

   if (fFilter) delete fFilter;
   fFilter = new TRegexp(filter, kTRUE);
}

//______________________________________________________________________________
void TGFileContainer::Sort(EFSSortMode sortType)
{
   // Sort file system list view container according to sortType.

   fSortType = sortType;

   fList->Sort();

   TGCanvas *canvas = (TGCanvas *) this->GetParent()->GetParent();
   canvas->Layout();
}

//______________________________________________________________________________
void TGFileContainer::GetFilePictures(const TGPicture **pic,
             const TGPicture **lpic, Int_t file_type, Bool_t is_link,
             const char *name, Bool_t /*small*/)
{
   // Determine the file picture for the given file type.

   static TString cached_ext;
   static const TGPicture *cached_spic = 0;
   static const TGPicture *cached_lpic = 0;
   const char *ext = name ? strrchr(name, '.') : 0;
   *pic = 0;
   *lpic = 0;

   if (fCachePictures && ext && cached_spic && cached_lpic && (cached_ext == ext)) {
      *pic = cached_spic;
      *lpic = cached_lpic;
      return;
   }

   if (R_ISREG(file_type)) {
      *pic = fClient->GetMimeTypeList()->GetIcon(name, kTRUE);
      *lpic = fClient->GetMimeTypeList()->GetIcon(name, kFALSE);

      if (*pic) {
         if (!*lpic) *lpic = *pic;
         if (ext) {
            cached_ext = ext;
            cached_spic = *pic;
            cached_lpic = *lpic;
            return;
         }
      }
   } else {
      *pic = 0;
   }

   if (*pic == 0) {
      *pic = fDoc_t;
      *lpic = fDoc_s;

      if (R_ISREG(file_type) && (file_type) & kS_IXUSR) {
         *pic = fApp_t;
         *lpic = fApp_s;
      }
      if (R_ISDIR(file_type)) {
         *pic = fFolder_t;
         *lpic = fFolder_s;
      }
   }
   if (is_link) {
      *pic = fSlink_t;
      *lpic = fSlink_s;
   }

   cached_lpic = 0;
   cached_spic = 0;
   cached_ext = "";
}

//______________________________________________________________________________
void TGFileContainer::ChangeDirectory(const char *path)
{
   // Change current directory.

   TString savdir = gSystem->WorkingDirectory();
   gSystem->ChangeDirectory(fDirectory.Data());   // so path of ".." will work
   if (gSystem->ChangeDirectory(path)) {
      fDirectory = gSystem->WorkingDirectory();
      gSystem->ChangeDirectory(savdir.Data());
      DisplayDirectory();
   }
}

//______________________________________________________________________________
void TGFileContainer::DisplayDirectory()
{
   // Display the contents of the current directory in the container.
   // This can be used to refresh the contents of the window.

   RemoveAll();
   CreateFileList();

   // This automatically calls layout
   Sort(fSortType);

   // Make TGExplorerMainFrame display total objects in status bar
   SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED),
               fTotal, fSelected);

   MapSubwindows();
}

//______________________________________________________________________________
void TGFileContainer::CreateFileList()
{
   // This function creates the file list from current dir.

   TString savdir = gSystem->WorkingDirectory();
   if (!gSystem->ChangeDirectory(fDirectory.Data())) return;

   FileStat_t sbuf;
   if (gSystem->GetPathInfo(".", sbuf) == 0)
      fMtime = sbuf.fMtime;

   void *dirp;
   if ((dirp = gSystem->OpenDirectory(".")) == 0) {
      gSystem->ChangeDirectory(savdir.Data());
      return;
   }

   const char *name;
   while ((name = gSystem->GetDirEntry(dirp)) != 0 && fDisplayStat) {
      if (strcmp(name, ".") && strcmp(name, ".."))
         AddFile(name);
      gSystem->ProcessEvents();
   }
   gSystem->FreeDirectory(dirp);

   gSystem->ChangeDirectory(savdir.Data());
}

//______________________________________________________________________________
TGFileItem *TGFileContainer::AddFile(const char *name,  const TGPicture *ipic,
                                     const TGPicture *ilpic)
{
   // Add file in container.

   Bool_t      is_link;
   Int_t       type, uid, gid;
   Long_t      modtime;
   Long64_t    size;
   TString     filename;
   TGFileItem *item = 0;
   const TGPicture *spic, *slpic;
   TGPicture *pic, *lpic;

   FileStat_t sbuf;

   type    = 0;
   size    = 0;
   uid     = 0;
   gid     = 0;
   modtime = 0;
   is_link = kFALSE;

   if (gSystem->GetPathInfo(name, sbuf) == 0) {
      is_link = sbuf.fIsLink;
      type    = sbuf.fMode;
      size    = sbuf.fSize;
      uid     = sbuf.fUid;
      gid     = sbuf.fGid;
      modtime = sbuf.fMtime;
   } else {
      if (sbuf.fIsLink) {
         Info("AddFile", "Broken symlink of %s.", name);
      } else {
         TString msg;
         msg.Form("Can't read file attributes of \"%s\": %s.",
                  name, gSystem->GetError());
         new TGMsgBox(fClient->GetDefaultRoot(), GetMainFrame(),
                      "Error", msg.Data(), kMBIconStop, kMBOk);
      }
      return item;
   }

   filename = name;
   if (R_ISDIR(type) || fFilter == 0 ||
       (fFilter && filename.Index(*fFilter) != kNPOS)) {

      if (ipic && ilpic) { // dynamic icons
         spic = ipic;
         slpic = ilpic;
      } else {
         GetFilePictures(&spic, &slpic, type, is_link, name, kTRUE);
      }

      pic = (TGPicture*)spic; pic->AddReference();
      lpic = (TGPicture*)slpic; lpic->AddReference();

      item = new TGFileItem(this,lpic, slpic, spic, pic, 
                            new TGString(gSystem->BaseName(name)),
                            type, size, uid, gid, modtime, fViewMode);
      AddItem(item);
   }

   return item;
}

//______________________________________________________________________________
TGFileItem *TGFileContainer::AddRemoteFile(TObject *obj, const TGPicture *ipic,
                                           const TGPicture *ilpic)
{
   // Add remote file in container.

   Bool_t      is_link;
   Int_t       type, uid, gid;
   Long_t      modtime;
   Long64_t    size;
   TString     filename;
   TGFileItem *item = 0;
   const TGPicture *spic, *slpic;
   TGPicture *pic, *lpic;

   FileStat_t sbuf;

   type    = 0;
   size    = 0;
   uid     = 0;
   gid     = 0;
   modtime = 0;
   is_link = kFALSE;
   
   TRemoteObject *robj = (TRemoteObject *)obj;

   robj->GetFileStat(&sbuf);
   is_link = sbuf.fIsLink;
   type    = sbuf.fMode;
   size    = sbuf.fSize;
   uid     = sbuf.fUid;
   gid     = sbuf.fGid;
   modtime = sbuf.fMtime;
   filename = robj->GetName();
   if (R_ISDIR(type) || fFilter == 0 ||
       (fFilter && filename.Index(*fFilter) != kNPOS)) {

      if (ipic && ilpic) { // dynamic icons
         spic = ipic;
         slpic = ilpic;
      } else {
         GetFilePictures(&spic, &slpic, type, is_link, filename, kTRUE);
      }

      pic = (TGPicture*)spic; pic->AddReference();
      lpic = (TGPicture*)slpic; lpic->AddReference();

      item = new TGFileItem(this, lpic, slpic, spic, pic, new TGString(filename),
                            type, size, uid, gid, modtime, fViewMode);
      AddItem(item);
   }
   return item;
}

//______________________________________________________________________________
void TGFileContainer::StopRefreshTimer()
{
   // stop refresh  timer

   if (fRefresh) delete fRefresh;
   fRefresh = 0;
}

//______________________________________________________________________________
void TGFileContainer::StartRefreshTimer(ULong_t msec)
{
   // start refreshing

   fRefresh = new TViewUpdateTimer(this, msec);
   gSystem->AddTimer(fRefresh);
}

//______________________________________________________________________________
void TGFileContainer::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a file container widget as a C++ statement(s) on output stream out.

   if (fBackground != GetDefaultFrameBackground()) SaveUserColor(out, option);

   char quote = '"';
   out << endl << "   // container frame" << endl;
   out << "   TGFileContainer *";

   if ((fParent->GetParent())->InheritsFrom(TGCanvas::Class())) {
      out << GetName() << " = new TGFileContainer(" << GetCanvas()->GetName();
   } else {
      out << GetName() << " = new TGFileContainer(" << fParent->GetName();
      out << "," << GetWidth() << "," << GetHeight();
   }

   if (fBackground == GetDefaultFrameBackground()) {
      if (GetOptions() == kSunkenFrame) {
         out <<");" << endl;
      } else {
         out << "," << GetOptionString() <<");" << endl;
      }
   } else {
      out << "," << GetOptionString() << ",ucolor);" << endl;
   }
   out << "   " << GetCanvas()->GetName() << "->SetContainer("
                << GetName() << ");" << endl;
   out << "   " << GetName() << "->DisplayDirectory();" << endl;
   out << "   " << GetName() << "->AddFile("<< quote << ".." << quote << ");" << endl;
   out << "   " << GetName() << "->StopRefreshTimer();" << endl;
}
