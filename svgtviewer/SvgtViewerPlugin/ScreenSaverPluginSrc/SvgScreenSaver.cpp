/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  This file implements the SVGT Screen Saver Plugin which
*                is used by Screen Saver application to display SVGT content
*
*/


#include <eikenv.h>
#include <bautils.h>

#include <aknnotewrappers.h> 
#include <SVGTCustControl.h>
#include <coecntrl.h>
#include <SVGTUIDialog.hrh>
#include <SVGTAppObserverUtil.h>
#include "SvgScreenSaver.h"
#include "SvgRenderFrameControl.h"

_LIT( KSccSmilFitValue,"meet" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSvgScreenSaver::CSvgScreenSaver
// -----------------------------------------------------------------------------
//
CSvgScreenSaver::CSvgScreenSaver():iIsRenderFrameControl(ETrue),
                                   iLastError(KErrNone)
    {        
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::~CSvgScreenSaver
// -----------------------------------------------------------------------------
//   
CSvgScreenSaver::~CSvgScreenSaver()
    {
    iFileHandle.Close();
    delete iSvgControl;    
    }
    
// -----------------------------------------------------------------------------
// CSvgScreenSaver::SetRect
// -----------------------------------------------------------------------------
//    
void CSvgScreenSaver::SetRect(const TRect& aRect)
    {
    iRect = aRect;

    if ( iIsRenderFrameControl )
        {
        if ( iSvgControl )
            {
            TInt index = STATIC_CAST(CSvgRenderFrameControl*, iSvgControl)->
                GetCurrentIndex(); 
            delete iSvgControl;      
            iSvgControl = NULL;
            AsFile( iFileName );
            if ( iIsRenderFrameControl && iSvgControl )
                {
                STATIC_CAST(CSvgRenderFrameControl*, iSvgControl)->
                   SetCurrentIndex( index );                    
                }
            }        
        }
    else
        {
        if ( iSvgControl )
            {
            iSvgControl->SetExtentToWholeScreen();
            }
        }        
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::SendCommandL
// -----------------------------------------------------------------------------
//
void CSvgScreenSaver::SendCommandL( TInt aCommand )
    {
    if ( !iSvgControl )
        {
        User::Leave( KErrGeneral );
        }
    if ( iIsRenderFrameControl )
        {
        STATIC_CAST(CSvgRenderFrameControl*, iSvgControl)->
                ProcessViewerCommandL( aCommand );
        }
    else
        {
        STATIC_CAST(CSVGTCustControl*, iSvgControl)->
                ProcessViewerCommandL( aCommand );
        }    
    }
    
// -----------------------------------------------------------------------------
// CSvgScreenSaver::MakeVisible
// -----------------------------------------------------------------------------
//
void CSvgScreenSaver::MakeVisible( TBool aVisible )
    {
    if ( iSvgControl )
        {
        iSvgControl->MakeVisible( aVisible );        
        }    
    }
 
// -----------------------------------------------------------------------------
// CSvgScreenSaver::GetLastError
// -----------------------------------------------------------------------------
//
TInt CSvgScreenSaver::GetLastError() const
    {
    return iLastError;
    }


// -----------------------------------------------------------------------------
// CSvgScreenSaver::FetchImage
// -----------------------------------------------------------------------------
//    
TInt CSvgScreenSaver::FetchImage( const TDesC& aUri, RFs& aSession, 
                 RFile& aFileHandle )
    {
    CSVGTAppObserverUtil::TDownloadNeeded dnloadNeeded = 
                                     CSVGTAppObserverUtil::ELocalImageFile ;
    TRAPD( err, 
       dnloadNeeded = CSVGTAppObserverUtil::IsDownloadNeededL( aUri ));
    if ( err != KErrNone )
        {        
        return err;   
        }
    
           
    if ( CSVGTAppObserverUtil::ELocalImageFile == dnloadNeeded )
        {
                           
        TFileName fileName;    
        if ( CSVGTAppObserverUtil::GetLocalFile( aUri, fileName, iFileName ) )
            {
            return aFileHandle.Open( aSession, fileName, EFileShareAny );
            }
        else
            {
            return KErrNotFound;
            }
        }
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::GetSmilFitValue
// -----------------------------------------------------------------------------
//
void CSvgScreenSaver::GetSmilFitValue(TDes& aSmilValue)
    {
    aSmilValue.Copy( KSccSmilFitValue );
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::NotifyL
// -----------------------------------------------------------------------------
//    
TInt CSvgScreenSaver::NotifyL(TNotificationType /*aCallType*/, 
                                    void* /*aParam*/)
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::ConstructL
// -----------------------------------------------------------------------------
//    
void CSvgScreenSaver::ConstructL( const CCoeControl* aParent )
    { 
    iParent = aParent; 
    iEikEnv = CEikonEnv::Static();
    }
    
// -----------------------------------------------------------------------------
// CSvgScreenSaver::AsFile
// -----------------------------------------------------------------------------
//     
TInt CSvgScreenSaver::AsFile(const TDesC& aFileName)
    {
    iFileName = aFileName;
    iLastError = iFileHandle.Open( iEikEnv->FsSession(), 
                                 aFileName, 
                                 EFileShareReadersOnly );
                                 
    if ( iLastError != KErrNone )
        {
        return iLastError;
        }
    TInt aSize = 0;
    iFileHandle.Size(aSize);
        
    if ( aSize > KMaxFileSize )
       {
            iLastError = KErrGeneral;
        }
                                     
    else
        {
            TRAP( iLastError,
            iSvgControl = CSvgRenderFrameControl::NewL( iParent, iFileHandle, this ) );    
        }                                   
            
    if ( iLastError != KErrNone )
        {
        iIsRenderFrameControl = EFalse;
        delete iSvgControl;
        iSvgControl = NULL;
     
        // reset iLastError
        iLastError = KErrNone;

        // Render Frames returned error
        TRAP( iLastError, 
          iSvgControl = CSVGTCustControl::NewL( iFileHandle,
                                              this,   // App Observer
                                              NULL,   // Progress Bar Draw Impl
                                              NULL,	  // MSK label Impl
                                              NULL,   // Exit Observer
                                              EFalse, // No Progressive Render
                                              iParent,
                                              ESvgWindowOwning ));
                                               
        if ( iSvgControl )
            {
            STATIC_CAST(CSVGTCustControl*, iSvgControl)->
                    SetCurrentFullScreenStatus( ETrue );
            // trigger a size changed event
            iSvgControl->SetExtentToWholeScreen();                    
            TRAP_IGNORE(STATIC_CAST(CSVGTCustControl*, iSvgControl)->
                    ProcessViewerCommandL(ESvgtDlgStopCommand));
            }          
        else
            {
            // Control is not created.
            iLastError = KErrGeneral;
            }
        } 
      
    iFileHandle.Close();
   
    return iLastError;
    
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::LinkActivated
// -----------------------------------------------------------------------------
//  
TBool CSvgScreenSaver::LinkActivated( const TDesC& /*aUri*/ )
    {
    return EFalse;
    }

        
// -----------------------------------------------------------------------------
// CSvgScreenSaver::LinkActivatedWithShow
// -----------------------------------------------------------------------------
// 
TBool CSvgScreenSaver::LinkActivatedWithShow( const TDesC& /*aUri*/, 
                                              const TDesC& /*aShow*/ )
    {
    return EFalse;
    }
               
// -----------------------------------------------------------------------------
// CSvgScreenSaver::SetDownloadMenu
// -----------------------------------------------------------------------------
// 
void CSvgScreenSaver::SetDownloadMenu( TBool /*aAddDownloadMenu*/ )
    {
    }
        
// -----------------------------------------------------------------------------
// CSvgScreenSaver::CanShowDownload
// -----------------------------------------------------------------------------
//   
TBool CSvgScreenSaver::CanShowDownload() const
    {
    return EFalse;
    }       

// -----------------------------------------------------------------------------
// CSvgScreenSaver::DisplayDownloadMenuL
// -----------------------------------------------------------------------------
//  
void CSvgScreenSaver::DisplayDownloadMenuL()
    {
    }
// -----------------------------------------------------------------------------
// CSvgScreenSaver::CanShowSave
// -----------------------------------------------------------------------------
// 
TBool CSvgScreenSaver::CanShowSave()
    {
    return EFalse;
    }
        
// -----------------------------------------------------------------------------
// CSvgScreenSaver::IsSavingDone
// -----------------------------------------------------------------------------
// 
TBool CSvgScreenSaver::IsSavingDone()
    {
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::DoSaveL
// -----------------------------------------------------------------------------
//  
void CSvgScreenSaver::DoSaveL( TInt /*aCommandId*/ )
    {
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::NewFetchImageData
// -----------------------------------------------------------------------------
//  
TInt CSvgScreenSaver::NewFetchImageData( const TDesC& /* aUri */ )
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::AssignEmbededDataL
// -----------------------------------------------------------------------------
//  
void CSvgScreenSaver::AssignEmbededDataL( const TDesC& /*aUri*/ )
    {
    }

// -----------------------------------------------------------------------------
// CSvgScreenSaver::ExitWhenOrientationChange
// -----------------------------------------------------------------------------
//  
TBool CSvgScreenSaver::ExitWhenOrientationChange()
    {
        return EFalse;
    }

// End Of file.

