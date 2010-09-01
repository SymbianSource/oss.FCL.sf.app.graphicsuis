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
* Description:  This file implements a control which displays svg files using 
*                renderframes engine api, used by Screen Saver application 
*
*/


#include <eikenv.h>
#include <eikappui.h>
#include <gdi.h>
#include <AknUtils.h>
#include <SVGEngineInterfaceImpl.h>
#include <SVGTUIDialog.hrh>
#include <SVGTAppObserver.h>
#include <flogger.h>

#include "SvgRenderFrameControl.h"
#include "SvgRenderFrameTimer.h"

const TUint KConvertToMicroSecond = 1000;
const TInt  KMilliSecondsBetweenFrame = 50;

// Font Ids for use with Engine-Init
const TInt KApacFontId = EApacPlain16;
const TInt KLatintFontId = ELatinBold12;

// Duration const returned by engine when indefinite animations are present
const TUint KIndefiniteDur = 0xffffffff;

#ifdef _DEBUG
_LIT( KFileLoggingDir, "ScreenSaver" );
_LIT( KFileLog, "FromSvgPlugin.txt" );
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::NewL
// -----------------------------------------------------------------------------
//
CSvgRenderFrameControl* CSvgRenderFrameControl::NewL( 
                                             const CCoeControl* aParent, 
                                             RFile& aFileHandle ,
                                             MSvgtAppObserver* aObserver )
    {
    CSvgRenderFrameControl* self = CSvgRenderFrameControl::NewLC( aParent,
                                                                  aFileHandle,
                                                                  aObserver );   
    CleanupStack::Pop( self );
    return self;
    }
    
// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::NewL
// -----------------------------------------------------------------------------
//
CSvgRenderFrameControl* CSvgRenderFrameControl::NewLC( 
                                             const CCoeControl* aParent, 
                                             RFile& aFileHandle ,
                                             MSvgtAppObserver* aObserver )
    {
    CSvgRenderFrameControl* self = new (ELeave) CSvgRenderFrameControl(
                                   aFileHandle, aObserver );
    CleanupStack::PushL( self );
    self->ConstructL( aParent );    
    return self;
    }    

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::~CSvgRenderFrameControl
// -----------------------------------------------------------------------------
//   
CSvgRenderFrameControl::~CSvgRenderFrameControl()
    {     
    DeleteEngine();  
    delete iBmap;  
    delete iRenderFrameTimer;          
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::SizeChanged
// -----------------------------------------------------------------------------
// 
void CSvgRenderFrameControl::SizeChanged()
    {
    iRect = iParent->Rect();
    
    }
// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::UpdateScreen
// -----------------------------------------------------------------------------
//
void CSvgRenderFrameControl::UpdateScreen()
    {
    }
    
// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::ScriptCall
// -----------------------------------------------------------------------------
//    
TBool CSvgRenderFrameControl::ScriptCall( const TDesC& /*aScript*/,
                                          CSvgElementImpl* /*aCallerElement*/ )
    {    
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::FetchImage
// -----------------------------------------------------------------------------
//    
TInt CSvgRenderFrameControl::FetchImage( const TDesC& aUri, RFs& aSession, 
                                         RFile& aFileHandle )
    {
    if ( iAppObserver )
        {
        return iAppObserver->FetchImage( aUri, aSession, aFileHandle );        
        }
    else
        {
        return KErrNotFound;
        }
    
    }

TInt CSvgRenderFrameControl::FetchFont( const TDesC& /* aUri */, 
                                    RFs& /* aSession */, RFile& /* aFileHandle */ )
    {
    return KErrNotSupported;
    }
// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::GetSmilFitValue
// -----------------------------------------------------------------------------
//
void CSvgRenderFrameControl::GetSmilFitValue(TDes& /*aSmilValue*/)
    {
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::UpdatePresentation
// -----------------------------------------------------------------------------
//    
void CSvgRenderFrameControl::UpdatePresentation( const TInt32&  
                                                 /*aNoOfAnimation*/ )
    {
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::Draw
// -----------------------------------------------------------------------------
// 
void CSvgRenderFrameControl::Draw(const TRect& /*aRect*/) const
    {       
       
    // Calculate rectangle for the whole bitmap
    TRect bmpPieceRect(TPoint(0,0), iRect.Size() );    
    
    // Get the standard graphics context
    CWindowGc& gc = SystemGc();    
    
    gc.SetBrushColor(CGraphicsContext::ENullBrush );
    gc.SetPenColor( CGraphicsContext::ENullPen );
    
    gc.BitBlt( TPoint(0,0),iBitmapFrames[iCurrentFrame],
              bmpPieceRect );       
    }
    
// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::SetTimer
// -----------------------------------------------------------------------------
// 
void CSvgRenderFrameControl::SetTimer()    
    {    
    if ( !iRenderFrameTimer )
        {        
        TRAP_IGNORE( iRenderFrameTimer = 
                    CSvgRenderFrameTimer::NewL( this ) );
        }
    if ( iCanPlay )               
        {
        // Since the number of delay intervals is one less than 
        // the number of bitmaps need to have a bound check
        iRenderFrameTimer->SetTimeOut( iDelayIntervals[
           iCurrentFrame >= iDelayIntervals.Count() ? 0 : iCurrentFrame]
           * KConvertToMicroSecond );       
        }  
    }
 
// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::CancelTimer
// -----------------------------------------------------------------------------
//      
void CSvgRenderFrameControl::CancelTimer()
    {
    if (iRenderFrameTimer)
        {
        iRenderFrameTimer->Cancel();
        delete iRenderFrameTimer;
        iRenderFrameTimer = NULL;
        }
    }
    
// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::ProcessViewerCommandL
// -----------------------------------------------------------------------------
//    
void CSvgRenderFrameControl::ProcessViewerCommandL( TInt aCommandId )
    {
    if ( aCommandId == ESvgtDlgPlayCommand )
        {
         _LIT(msg1, "start event " );    
        PrintDebugMsg( msg1 );    
        iCanPlay = ETrue;
        SetTimer();
        }
    if ( aCommandId == ESvgtDlgPauseCommand )
        {
         _LIT(msg1, "end event " );    
        PrintDebugMsg( msg1 );    
        iCanPlay = EFalse;
        CancelTimer();
        }        
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::GetCurrentIndex
// -----------------------------------------------------------------------------
//
TInt CSvgRenderFrameControl::GetCurrentIndex() const
    {
    return iCurrentFrame;
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::SetCurrentIndex
// -----------------------------------------------------------------------------
//        
void CSvgRenderFrameControl::SetCurrentIndex( TInt aIndex )
    {
    iCurrentFrame = aIndex;
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::AnimationTimeOut
// -----------------------------------------------------------------------------
//     
void CSvgRenderFrameControl::AnimationTimeOut()
    {   
    DrawNow();
    SetTimer();
    MoveIndex();    
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::InitializeEngineL
// -----------------------------------------------------------------------------
//  
void CSvgRenderFrameControl::InitializeEngineL( TBool aCheckFile,
                                     MSvgRequestObserver* aObserver )
    {    
    
    iBmap =  new ( ELeave ) CFbsBitmap();    
    TSize dummyScreenSize( 10,10  );    
    iBmap->Create( dummyScreenSize, EColor16MU );    
           
    if ( !iSvgEngine )
        {
        TInt fontId = KLatintFontId;
    
        switch ( AknLayoutUtils::Variant() )
            {
            case EApacVariant:
                {
                fontId = KApacFontId;
                }
                break;
            case EEuropeanVariant:
            default:
                break;
            }
        const CFont* font = AknLayoutUtils::FontFromId( fontId );
        TFontSpec spec = font->FontSpecInTwips();

        iSvgEngine = CSvgEngineInterfaceImpl::NewL( iBmap,
                                                    aObserver,
                                                    spec );                    
                                                        
        if ( iSvgEngine && !iSvgDom  )
            {       
            if ( aCheckFile )
                {
                /* Set Thumbnail Mode if check file mode
                 * so that the rights wont be consumed 
                 */
                iSvgEngine->SetThumbNailMode( ETrue );
                }
                
            MSvgError* parseResult = iSvgEngine->PrepareDom( *iFileHandle, 
                                                             iSvgDom );
            
            if ( aCheckFile )
                {
                /* Reset Thumbnail Mode */                
                iSvgEngine->SetThumbNailMode( EFalse );
                }                                               
                                                               
            if ( parseResult && 
                 parseResult->HasError() && 
                 !parseResult->IsWarning() )
                {   
                if ( parseResult->HasError() == ESvgNoMemory )
                    {
                    User::Leave( KErrNoMemory );        
                    }
                else
                    {
                    User::Leave( KErrGeneral );
                    }
                }        
            if ( !aCheckFile )
                {
                MSvgError* pResult = iSvgEngine->UseDom( iSvgDom, iBmap );
                if ( pResult->HasError() && !pResult->IsWarning() )
                    {                
                    User::Leave( KErrGeneral );
                    }               
                }
            }
        else
            {
            User::Leave( KErrGeneral );
            }
        }
    else
        {
        User::Leave( KErrGeneral );
        }
    return;
    
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::DeleteEngine
// -----------------------------------------------------------------------------
//  
void CSvgRenderFrameControl::DeleteEngine()
    {       
    for ( TInt i = 0; i < iBitmapFrames.Count(); i++ )
        {
        delete iBitmapFrames[i];
        }        

    for ( TInt i = 0; i < iMaskFrames.Count(); i++ )
        {
        delete iMaskFrames[i];
        }        

    iBitmapFrames.Reset();
    iMaskFrames.Reset();
    iDelayIntervals.Reset();
    
    iBitmapFrames.Close();
    iMaskFrames.Close();
    iDelayIntervals.Close();
    
    iCurrentFrame = 0; 
    iDuration = 0;
    
    if ( iSvgDom )
        {
        iSvgEngine->DeleteDom( iSvgDom );
        iSvgDom = 0;
        }
    delete iSvgEngine;    
    iSvgEngine = NULL;
    
    }   

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::RenderMyFrames
// -----------------------------------------------------------------------------
//  
TInt CSvgRenderFrameControl::RenderMyFrames()
    {
    
    TUint interval = iSvgEngine->Duration();   
   
    if ( interval == KIndefiniteDur || interval > KMaxTimeDuration )
        {
        // Infinite content
        // Dont try to handle using Render Frames
        // return Error
        return KErrGeneral;     
        }

    
    
    MSvgError* result = iSvgEngine->RenderFrames(
                                       iSvgDom,
                                       iRect.Size(),
                                       0, 
                                       interval,
                                       KMilliSecondsBetweenFrame,
                                       EColor64K, EGray2,
                                       iBitmapFrames,
                                       iMaskFrames,
                                       iDelayIntervals,
                                       ETrue /*Remove redundant frames*/ );
                                          
                                       
    if ( result->HasError() && !result->IsWarning() )
        {
        return KErrGeneral;
        } 
    else
        {
        return KErrNone;
        }
    }
   

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::MoveIndex
// -----------------------------------------------------------------------------
//     
void CSvgRenderFrameControl::MoveIndex()
    {
    
    TBuf<64> buffer1;
    _LIT(msg1, "Index is : %d" );
    buffer1.Format( msg1, iCurrentFrame );
    PrintDebugMsg( buffer1 );    
    
    iCurrentFrame++;    
    if ( iCurrentFrame == iBitmapFrames.Count() )
        {
        iCurrentFrame = 0;
        }
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::CSvgRenderFrameControl
// -----------------------------------------------------------------------------
//
CSvgRenderFrameControl::CSvgRenderFrameControl( RFile& aFileHandle,
                                                MSvgtAppObserver* aObserver )
    {
    iCurrentFrame = 0;
    iDuration = 0; 
    iSvgEngine = NULL;
    iSvgDom = 0;    
    iFileHandle = &aFileHandle;
    iCanPlay = EFalse;  
    iParent = NULL; 
    iAppObserver = aObserver; 
    }
 
// -----------------------------------------------------------------------------
// CSvgRenderFrameControl::ConstructL
// -----------------------------------------------------------------------------
//    
void CSvgRenderFrameControl::ConstructL( const CCoeControl* aParent )
    {       
    
    if ( aParent )
        {
        CreateWindowL(aParent); 
        iParent = aParent;
        iRect = aParent->Rect();
        }    
     
    InitializeEngineL( EFalse /*Not checkFile mode*/  , this );    
       
    User::LeaveIfError(RenderMyFrames()); 
    
    SetRect( iRect );
    
    ActivateL();
    
    }
    
// -----------------------------------------------------------------------------
// CScreenSaverAnimPlugin::PrintDebugMsg
// -----------------------------------------------------------------------------
//
#ifndef _DEBUG        
void CSvgRenderFrameControl::PrintDebugMsg( const TDesC&  /* aMsg */ ) const
#else
void CSvgRenderFrameControl::PrintDebugMsg( const TDesC&  aMsg ) const
#endif
    {
#ifdef _DEBUG        
    RFileLogger::Write( KFileLoggingDir, KFileLog, 
                        EFileLoggingModeAppend, aMsg );
#endif    
    }  
          
// End Of file.

