/*
* Copyright (c) 2004,2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This file implements the SVGT Custom Control which
*                is used to display SVGT content
*
*/



// INCLUDE FILES
#include <aknnotewrappers.h>
#include <AknWaitNoteWrapper.h>
#include <AknIconUtils.h>
#include <aknlayoutscalable_apps.cdl.h>
#include <AknsConstants.h>
#include <AknsSkinInstance.h>
#include <AknsUtils.h>
#include <AknWaitDialog.h>
#include <bitdev.h>
#include <StringLoader.h>
#include <DRMHelper.h>
#include <eikenv.h> //CEikonEnv
#include <e32math.h>
#include <uri16.h>
#include <ctsydomainpskeys.h> // for call handling
#include <data_caging_path_literals.hrh>
#include <gdi.h>
#include <svgtuicontrol.rsg>
#include <svgtuicontrol.mbg>
// User Includes
#include "SVGTUIControlDbgFlags.hrh"
#include "SVGTAppObserver.h"
#include "SVGTCustControl.h"
#include "SVGTUIDialog.hrh"

#include "SvgtController.h"
#include "SvgtApplicationExitObserver.h"
#include "SvgtEventHandlerAO.h"
#include "SvgtEvent.h"
#include <svgtfileviewdetails.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <avkon.mbg>

#ifdef RD_SVGT_AUDIO_SUPPORT
#include <AknVolumePopup.h>
#include<remconinterfaceselector.h>     //for CRemConInterfaceSelector
#include<remconcoreapitarget.h>         //for CRemConCoreApiTarget

#endif //RD_SVGT_AUDIO_SUPPORT
// Constants

// Panning Constants
// KSccPanNumPixelsX - Amount of Pixels to Pan in X Direction when Pan is
//                     performed once.
// KSccPanNumPixelsY - Amount of Pixels to Pan in Y Direction when Pan is
//                     performed once.
const TInt KSccPanNumPixelsX = 30;
const TInt KSccPanNumPixelsY = 30;

// Pointer Constants
// When pointer is moved in a particular direction, the movement also
// acceleration. The below constants define the delta for the acceleration.
// KSccCursorMinDeltaX - For X Direciton
// KSccCursorMinDeltaY - For Y Direciton
const TInt KSccCursorMinDeltaX = 5;
const TInt KSccCursorMinDeltaY = 5;

// Zoom Constants
// KSccZoomInFactor - Indicates zoom factor to use while zooming in.
//                    This Value should be > 1.0
// KSccZoomOutFactor - Indicates zoom factor to use while zooming out.
//                     This value should be > 0 and < 1.0
const TReal32 KSccZoomInFactor = 2.0; // 2X Zoom In
const TReal32 KSccZoomOutFactor = 0.5;// 2X Zoom Out

// Key Constants
const TInt KSccZoomInKey = ENumberKey5;//'5';
const TInt KSccZoomOutKey =ENumberKey0; //'0';

// Number of milli seconds per second
const TUint32 KSccNumMSecPerSec = 1000;



// Time in microsecs after which Pointer Inactivity should be declared
const TUint KSccPointerHideTimerStartAfter = 15000000;

// Time in microsecs per second
const TInt KSccOneSecond = 1000000;

// General Constants
const TInt KSccConstMinusOne = -1;
const TInt KSccConstZero = 0;
const TInt KSccConstOne = 1;
const TInt KSccConstTwo = 2;

// set the name of the multi-bitmap file containing the bitmaps
_LIT( KSccIconFile,"Z:SVGTUIControl.mif" );

// SMIL Fit Value
_LIT( KSccSmilFitValue,"meet" );

// Font Ids for use with Engine-Init
const TInt KApacFontId = EApacPlain16;
const TInt KLatintFontId = ELatinBold12;

// Duration const returned by engine when indefinite animations are present
const TUint KSccIndefiniteDur = 0xffffffff;


// Interval between two consecutive timer ticks in microseconds.
const TUint32 KSccProgIndTimerYieldDur = 100000;

//constant for converting radians to degrees
const TReal32 KDegreesToRadiansFactor = 0.01745329252;

// Maximum length of text in a text element
const TInt KSccMaxTextLength = 250;
const TInt KBackLightTimeInterval = 9;
const TInt KMaxEditorTextLength = 5120; 
const TInt KEmbededImageTagLength = 5;
const TInt KSchemaLength = 4;

#ifdef RD_SCALABLE_UI_V2
const TInt KInitRepeatInterval = 10000;
#endif // RD_SCALABLE_UI_V2

// Background colour which is used by engine before rendering as a base colour
const TUint32 KSccArgbWhite = 0xffffffff;

//TOUCH
#ifdef RD_SVGT_AUDIO_SUPPORT
const TInt KDefaultVolumeLevel = 3;   
const TInt KMinVolume = 0;
const TInt KMaxVolume = 10;
#endif // RD_SVGT_AUDIO_SUPPORT
//TOUCH
#ifdef _DEBUG
// Backlight Strings
_LIT( KSccBacklightOnStr, "Backlight On" );
_LIT( KSccBacklightOffStr, "Backlight Off" );
#endif

//Loading thread name
_LIT(KSvgThread,"SvgThread");

_LIT(KJpg,   ".jpg" );
_LIT(KJpeg,  ".jpeg" );
_LIT(KPng,   ".png" );
_LIT(KBmp,   ".bmp" );
_LIT(KSvg,   ".svg" );
_LIT(KSvgz,  ".svgz" );

_LIT( KWww, "www" );
_LIT( KHttp, "http://");
// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTCustControl::CSVGTCustControl
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CSVGTCustControl::CSVGTCustControl(
    RFile& aFileHandle, // File Handle of SVGT content
    TBool aProgressiveRendering,
    TInt aFlag ) :
    // Animation State
    iTotalNumerOfAnimation( KSccConstMinusOne ),
    iAnimationStatus( ESvgNoAnimationsPresent ),
    iAnimState( ESvgAnimStoppedState ),
    iContentFileHandle( aFileHandle ),
    // SVG Engine
    iIsLoadingDone( EFalse ),
    iFetchImageError( 0 ),
    // Control State Variables
    iZoomLevel( 0 ),
    iIsLoopOn( EFalse ),
    iIsFullScreenOn( EFalse ),
    // Pointer Variables
    iPointerDx( 0 ),
    iPointerDy( 0 ),
    iPointerX( 0 ),
    iPointerY( 0 ),
    // Current Match is invalid
    iIsSearchStrValid( EFalse ),
    iCurSearchMatchId( 0 ),
    iNumAnimInProgress( 0 ),
    iIsPointerDisplayed( ETrue ),                 
    iFindTextHiColor( KRgbBlack ),               
    iIgnorePointerKeyEvent( EFalse ),
    iPointerType( ESvgPointerDefault ),
    iEnableTextOps( ETrue ),
    iIsTextEditOn( EFalse ),
    iIsTextSelectOn(EFalse),
    iProgressiveRendering(aProgressiveRendering),
    iDisplayFlags( aFlag ),
    iEventHandlerAO(NULL),
    iThreadController(NULL),
    iIsWaitNoteDisplayed(EFalse),
#ifdef RD_SVGT_AUDIO_SUPPORT
	iVolumeLevel(KDefaultVolumeLevel),
	iPrevVolume(KDefaultVolumeLevel),
#endif //RD_SVGT_AUDIO_SUPPORT
    iIsTextChanged( EFalse ),
    iViewerCommand(EFalse),
    iPlayMSKLabel(EFalse),
    iContextMenuActivated(EFalse),
    iAElement(EFalse),
    iScrollKey(EFalse),
    iPointerDownAnimation(EFalse),
    iInteractiveElementEnteredCnt(0),
    iDownKeyOccurred(EFalse),
    iIsVolumeMuted(EFalse),
    iIsForeground(ETrue)
    {
    }


// -----------------------------------------------------------------------------
// CSVGTCustControl::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ConstructL(
    MSvgtAppObserver* aAppObs, // User Implementation of Mixin
    MSVGTProgressBarDrawImpl* aPBDrawImpl , // Implementation for rendering
                                            // progress indicator
	MSVGTMSKImpl* aMSKImpl,		//Interface for MSK labelling                                           
    MSvgtApplicationExitObserver* aExitObserver,
    const CCoeControl* aParent )
    {
    iEngineInitialized = EFalse;
    // Save the user parameters for later access.
    iAppObserver = aAppObs;
    iProgressBarDrawImpl = aPBDrawImpl;
    iMSKImpl=aMSKImpl;
    iExitObserver = aExitObserver;
   
    TRect lRect;
    GetNormalScreenLayoutRect( lRect );
    // Frame buffers creation
    CreateBitmapsL( lRect.Size() );

    // Icons(Pointer, Pan Ind) creation
    CreateIconsL();

#ifdef RD_SVGT_AUDIO_SUPPORT
    iVolumePopup = CAknVolumePopup::NewL(NULL, ETrue);
    iVolumePopup->SetObserver(this);
    _LIT( KVolumePopupTitle, "Presentation volume");
    iVolumePopup->SetTitleTextL(KVolumePopupTitle);
    iVolumePopup->SetRange( KMinVolume, KMaxVolume);
    iVolumePopup->SetValue( KDefaultVolumeLevel );
    
    
    // Open a connection to receive Volume Key events.
    iSelector = CRemConInterfaceSelector::NewL();
    iInterfaceSelector = CRemConCoreApiTarget::NewL(*iSelector,*this);
    TRAPD(err, iSelector->OpenTargetL());
    if(err != KErrNone)
    {
		#ifdef _DEBUG
		RDebug::Printf("Leave occured in OpenTargetL. Error code returned %d",err);
		#endif
    }
    
#endif  //RD_SVGT_AUDIO_SUPPORT

    // DrmHelper to display save related query
    iSvgDrmHelper = CSVGTDrmHelper::NewL();

    // Check if it is a preview file
    iPreviewMode = iSvgDrmHelper->IsPreviewL( iContentFileHandle );
   
    // Initialise the SVG engine.
    InitializeEngineL();
   
   
//############################################################
    // Create the timer for Pointer Inactivity
    if ( iDisplayFlags & ESvgDrawPointer )
        {
        iInactivityTimer = CSVGTPointerHideTimer::NewL( this );
        iInactivityTimer->After( KSccPointerHideTimerStartAfter );
        }

    // Create the timer for blinking Pause
    if ( iDisplayFlags & ESvgDrawPauseIndicator )
        {

        iBlinker = CPeriodic::NewL( CTimer::EPriorityStandard );
        }   

    // Create the timer for removal of info note
    if ( iDisplayFlags & ESvgDisplayErrorNotes)
        {
        // Create the timer for removal of info note
        iNoteRemoverTimer = CPeriodic::NewL( CTimer::EPriorityHigh );
        }

    if ( iDisplayFlags & ESvgDisplayErrorNotes )
        {
        // Create instance of DRM Helper for checking rights for content   
        iSvgDrmHelper->CheckRightsAmountL( iContentFileHandle );
        }
   
    //Initialize the backlight time to current time
    iPreviousBackLightTimeStamp.HomeTime();
    if(iProgressiveRendering)
        {
        iEventHandlerAO = CSvgtEventHandlerAO::NewL( aAppObs, this,
RThread().Id());
        
        iThreadController = CSvgtThreadController::NewL(this);
        //Separate thread is created to load the content
        // set up parameters to thread generate thread, leave if fails
        TInt result = iMainThread.Create( KSvgThread,
                                      ( TThreadFunction )LoadSvgContent,
                                      KDefaultStackSize,
                                      NULL, this, EOwnerProcess );
        User::LeaveIfError( result );
        // log on to thread & requests notification of thread completion
        iThreadController->IssueThreadMonitorRequest( iMainThread );

        // give thread low priority
        iMainThread.SetPriority( EPriorityMuchLess );
        // resume thread (wake it up sometime after this function returns)
        iMainThread.Resume();
        }
    else
        {
        // Start displaying wait note
        LaunchWaitNoteL( R_QTN_SVGT_WAITING_OPENING );
   
        // Load the SVGT content
        LoadContentL( iContentFileHandle );

        // Dismiss the wait note
        DismissWaitNote();

        TInt lLoadErr = DoHandleLoadingThreadError();
        if ( lLoadErr != KErrNone )
            {
            if ( lLoadErr != KErrNoMemory )
                {
                // All errors other than memory failure related are
                // ignored.
                lLoadErr = KErrNone;
                }
            User::Leave( lLoadErr );   
            }
       
        // Do the post load functionality
        DoPostLoadFuncL();   
        }
    if ( aParent && (iDisplayFlags & ESvgWindowOwning ))
        {
        CreateWindowL( aParent );
        }
//############################################################
    // Set the windows size
    SetRect( lRect );

    // Activate the window, which makes it ready to be drawn
    ActivateL();   
    }


// -----------------------------------------------------------------------------
// CSVGTCustControl::NewL
// Factory function for creating CSVGTCustControl objects.
// Returns: CSVGTCustControl* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
EXPORT_C CSVGTCustControl* CSVGTCustControl::NewL(
    RFile& aFileHandle,         // File Handle for the content
    MSvgtAppObserver* aAppObs,  // User Implementation of Mixin
    MSVGTProgressBarDrawImpl* aPBDrawImpl , // Implementation for rendering
                                            // progress indicator
	MSVGTMSKImpl* aMSKImpl,		//Interface for MSK labelling                                          
    MSvgtApplicationExitObserver* aExitObserver,
    TBool aProgressiveRendering,
    const CCoeControl* aParent,                                           
    TInt aDisplayFlag )
    {
    CSVGTCustControl* self = CSVGTCustControl::NewLC( aFileHandle,
                                                      aAppObs,
                                                      aPBDrawImpl,
                                                      aMSKImpl,
                                                      aExitObserver,
                                                      aProgressiveRendering,
                                                      aParent,
                                                      aDisplayFlag );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::NewLC
// Factory function for creating CSVGTCustControl objects.
// Returns: CSVGTCustControl* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTCustControl* CSVGTCustControl::NewLC(
    RFile& aFileHandle,         // File handle for the content
    MSvgtAppObserver* aAppObs , // User Implementation of Mixin
    MSVGTProgressBarDrawImpl* aPBDrawImpl , // Implementation for rendering
                                            // progress indicator
	MSVGTMSKImpl* aMSKImpl,		//Interface for MSK labelling                                            
    MSvgtApplicationExitObserver* aExitObserver,
    TBool aProgressiveRendering,
    const CCoeControl* aParent,                                           
    TInt aDisplayFlag )
    {
    CSVGTCustControl* self = new ( ELeave ) CSVGTCustControl( aFileHandle,
                                                             
aProgressiveRendering,
                                                              aDisplayFlag );
    CleanupStack::PushL( self );
    self->ConstructL( aAppObs, aPBDrawImpl ,aMSKImpl, aExitObserver, aParent );
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::~CSVGTCustControl
// Destructor
// -----------------------------------------------------------------------------
//
CSVGTCustControl::~CSVGTCustControl()
    {
    
#ifdef RD_SVGT_AUDIO_SUPPORT
    // if non volume key is pressed <br>
    if(iVolumePopup)
        {
        iVolumePopup->CloseVolumePopup();
        delete iVolumePopup;
        iVolumePopup=NULL;
        }
    if(iSelector)
        {
        delete iSelector;
        iInterfaceSelector=NULL;
        iSelector=NULL; //iSelector has been deleted by "delete iInterfaceSelector"
        }
    
#endif //RD_SVGT_AUDIO_SUPPORT       

    if(iProgressiveRendering)
        {
        TInt count = 0;
        if(iSvgModule)
            {
            if ( iSvgModule->IsLoading() )
                {
                iSvgModule->CancelLoad();
                do
                    {
                    User::After( 10 * 1000 ); // milliseconds
                    }while ( iSvgModule->IsLoading() && count++ < 100 );
                }
            }

        if(iThreadController)
            {
            iThreadController->StopThreadExecution(iMainThread);
            }
       
        iMainThread.Kill(KErrNone);
        iMainThread.Close();

        delete iEventHandlerAO;
       
        delete iThreadController;
        }
       
    // Hyperlink Handler
    if ( iSvgModule )
        {
        iSvgModule->RemoveHyperlinkListener( 
        	static_cast< MSvgHyperlinkListener*> ( this ) );
        iSvgModule->RemoveListener( static_cast<MSvgLoadingListener*>(this),
                                                ESvgLoadingListener);
        iSvgModule->RemoveListener( static_cast<MSvgLoadingListener*>(this),
                                                ESvgHyperlinkListener);
        iSvgModule->RemoveListener( static_cast<MSvgInteractiveElementListener*>(this), ESvgInteractiveElementListener);
        }
         
    // SVG Engine
    delete iSvgModule;
   
    // Free the icons
    FreeIcons();
   
    // Screen Bitmaps + Masks
    delete iSVGTBitMap;
    delete iSVGTBitMapMask;
   
    // delete SVG drmHelper
    delete iSvgDrmHelper;

   
   // Delete the arrays
   iBoundBoxArray.Close();
   iFoundTextArray.Close();
   iTextElemIdArray.Close();

    // Reset App Observer
    iAppObserver = NULL;
   
    // Reset Progress Bar Draw Implementation Callback
    iProgressBarDrawImpl = NULL;
    // Current Match is invalid
    iIsSearchStrValid = EFalse;
    iCurSearchMatchId = KSccConstZero;

    // Backlight Timer

    // Progress Indicator Timer
    delete iProgressBarTimer;
        
    // Pointer Inactivity Hide Timer
    delete iInactivityTimer;

    // Pause Blink Timer
    delete iBlinker;
   
    // Framebuffer  Bitmap
    delete iSVGTFrameBufBitMap;
   
    // Framebuffer Bitmap Context
    delete iSVGTFrameBufContext;
   
    // Framebuffer Bitmap Device
    delete iSVGTFrameBufDevice;
   
    // Delete the wait dialog
    delete iWaitDialog;
   
    // Delete the info note
    delete iInfoNote;
   
    // Delete the info note remover timer
    delete iNoteRemoverTimer;
   
    // Close the System RProperty
    iSystemState.Close(); 
   
    // Reset the loading thread error
    iLoadingThreadError = NULL;
   
    // Reset the exit observer
    iExitObserver = NULL;
   
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ProcessViewerCommandL
// Process commands for handling SVGT content
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ProcessViewerCommandL( TInt aCommandId ) // Command ID
                                                                // be processed
    {
   //Set the flag to indicate that the selection key is for options selection
    iViewerCommand=ETrue;
    switch( aCommandId )
        {
        case ESvgtDlgSaveCommand:
            break;
        case ESvgtDlgZoomInCommand:
            // Perform Zoom operation
            iSvgModule->Zoom( KSccZoomInFactor );
            // Do a Redraw
            iSvgModule->Redraw();
            //dummy mouse move
            iSvgModule->MouseMove( iPointerX, iPointerY );
            iZoomLevel++;
            UpdatePointer();
            break;
        case ESvgtDlgZoomOutCommand:
            if ( iZoomLevel > KSccConstZero )
                {
                iZoomLevel--;
                // Perform Zoom operation
                iSvgModule->Zoom( KSccZoomOutFactor );
                // Do a Redraw
                iSvgModule->Redraw();
                //dummy mouse move
                iSvgModule->MouseMove( iPointerX, iPointerY );
                UpdatePointer();
                }
            break;
        case ESvgtDlgZoomOptimalCommand:
            if ( iZoomLevel != KSccConstZero )
                {
                //This is to nullify only the zooming effect
                //Other transformations(panning, rotation) won't be restored
                TReal reducedFactor = 0.0;
                Math::Pow(reducedFactor, KSccZoomOutFactor, iZoomLevel);
                iZoomLevel = KSccConstZero;
                iSvgModule->Zoom(reducedFactor);
                iSvgModule->Redraw();
                }
            break;
        case ESvgtDlgSelectTextCommand:
            {
            if ( iEnableTextOps )
                {
                HBufC* lStrPtr = HBufC::NewLC( KMaxEditorTextLength ); 
                TPtr lPtr = lStrPtr->Des();
               
                if ( iCurrentTextElementId )
                    {
                    if ( iIsCurrentTextElement )
                        {
                        iSvgModule->GetTextForTextElement(
                            iCurrentTextElementId,
                            iIsTextEditOn,
                            lPtr );
                        }
                        else
                            {
                            iSvgModule->GetTextForTextAreaElement(
                                iCurrentTextElementId,
                                iIsTextEditOn,
                                lPtr );
                            }
                   
                    DoTextSelectionL( lPtr );
                    }
                CleanupStack::PopAndDestroy( lStrPtr );                   
                }
            }
            break;
        case ESvgtDlgEditTextCommand:
            {
            if ( iEnableTextOps )
                {
                if ( iCurrentTextElementId )
                    {
                    HBufC* lStrPtr = HBufC::NewLC( KMaxEditorTextLength ); 
                    TPtr lPtr = lStrPtr->Des();

                    if ( iIsCurrentTextElement )
                        {
                        iSvgModule->GetTextForTextElement(
                            iCurrentTextElementId,
                            iIsTextEditOn,
                            lPtr );
                        }
                        else
                            {
                            iSvgModule->GetTextForTextAreaElement(
                                iCurrentTextElementId,
                                iIsTextEditOn,
                                lPtr );
                            }
                   
                   
                    DoTextEditingL( iIsCurrentTextElement,
                                    iCurrentTextElementId,
                                    lPtr );
                    CleanupStack::PopAndDestroy( lStrPtr );                               
                    }
                }
            }
            break;
        case ESvgtDlgPlayCommand:
            ProcessPlayCmd();
            break;
        case ESvgtDlgPauseCommand:
            ProcessPauseCmd();
            break;
        case ESvgtDlgStopCommand:
            ProcessStopCmd();
            break;
//TOUCH SUPPORT START   
#ifdef RD_SVGT_AUDIO_SUPPORT
        case ESvgtDlgSetVolumeCommand:
            {
            // The volume popup will close itself after 1s 
            // unless another call to ShowVolumePopupL() is made
            if(iIsVolumeMuted)
            	{
            	// Set the volume of pop to zero if muted.
            	iVolumePopup->SetValue( 0 );
            	iVolumePopup->ShowVolumePopupL();
            	}
            else
            	{
            	// If not muted, then show the previous volume
            	iVolumePopup->SetValue( iPrevVolume );
            	iVolumePopup->ShowVolumePopupL(); 
            	}
            break;
            }
#endif //RD_SVGT_AUDIO_SUPPORT
//TOUCH SUPPORT END
        case ESvgtDlgLoopOnCommand:
            if ( IsContentFinite() )
                {
                iIsLoopOn = ETrue;
                }
            break;
        case ESvgtDlgLoopOffCommand:
            if ( IsContentFinite() )
                {
                iIsLoopOn = EFalse;
                }
            break;
        case ESvgtDlgFindTextCommand:
            ProcessFindTextCmdL();
            break;
        case ESvgtDlgFullScreenCommand: // Fall Through
        case ESvgtDlgNormalScreenCommand:
            // No processing here. The Command is processed by the dialog.
            // Dialog sets the size of the control, because of which
            // SizeChanged() of control is called. This queries the
            // layout rectangles and recreates the framebuffer.
            break;
        case ESvgtDlgViewDetailsCommand:
            {
            CSvgtFileViewDetailsDialog* fileDetailsDialog =
                CSvgtFileViewDetailsDialog::NewL();
            fileDetailsDialog->ExecuteLD( iContentFileHandle );

            break;
            }
        case ESvgtDlgAngle90:
            {
            DoRotation( ERotationAngle90 );
            }
            break;
        case ESvgtDlgAngle180:
            {
            DoRotation( ERotationAngle180 );
            }
            break;
        case ESvgtDlgAngle270:
            {
            DoRotation( ERotationAngle270 );
            }
            break;
        case ESvgtDlgHelpCommand:
            break;
        default:
            break;
        }
    if(iMSKImpl && !iPreviewMode)
    	{
    	TRAP_IGNORE(SetMSKLabelL());
    	}
}
// -----------------------------------------------------------------------------
// CSVGTCustControl::GetCurrentZoomLevel
// Get function for current zoom level. Zoom level is the number of times the
// user has zoomed in.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::GetCurrentZoomLevel(
    TInt& aZoomLevel ) const  // Current Zoom Level
    {
    aZoomLevel = iZoomLevel;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::GetCurrentAnimState
// Get function for current animation state
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::GetCurrentAnimState(
    TSvgtViewerAnimStatus& aAnimState )  const // Playing/Paused/Stopped
    {
    aAnimState = iAnimState;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::GetCurrentFullScreenStatus
// Get function for current viewing mode.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::GetCurrentFullScreenStatus(
    TBool& aIsFullScreenOn )  const //  Full Screen( ETrue )/Normal( EFalse )
    {
    aIsFullScreenOn = iIsFullScreenOn;
    }
// -----------------------------------------------------------------------------
// CSVGTCustControl::GetCurrentLoopStatus
// Get function for current loop mode.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::GetCurrentLoopStatus(
    TBool& aIsLoopOn )  const //  Loop On( ETrue )/Loop Off( EFalse )
    {
    aIsLoopOn = iIsLoopOn;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::GetNormalScreenLayoutRect
// Gets the normal screen layout rectangle for control from LAF.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::GetNormalScreenLayoutRect(
    TRect& aRect )  const // Rectangle specifying extent of control
    {
    // Get parent client rect.
    TRect myRect = iEikonEnv->EikAppUi()->ClientRect();
   
    if ( AknLayoutUtils::ScalableLayoutInterfaceAvailable() )
        {
        TAknLayoutRect lNormalLayoutRect;
        lNormalLayoutRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane( 0 ) ); // Normal Screen
        aRect = lNormalLayoutRect.Rect();
        }
        else
            {
            aRect = myRect;
            }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsLoopAllowed
// Indicates whether the Loop feature is allowed for the Control.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::IsLoopAllowed()  const
    {
    return ( IsContentFinite() );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsAnimationPresent
//  Finds whether the content has any animations
// @return TBool -  ETrue indicates content contains animations,
//                  EFalse otherwise
// -----------------------------------------------------------------------------
//
TBool  CSVGTCustControl::IsAnimationPresent() const
    {
    return ( iAnimationStatus != ESvgNoAnimationsPresent );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsContentFinite
//  Finds whether the content is of finite duration.
// @return TBool -  ETrue indicates Finite Duration,
//                  EFalse otherwise
// -----------------------------------------------------------------------------
//
TBool  CSVGTCustControl::IsContentFinite() const
    {
    // Does it not have any infinite repeat count
    // animations
    if ( ( iDuration != 0 ) &&
        ( iDuration != KSccIndefiniteDur ) )
        {
        return ETrue;
        }
    // Content contains atlease one infinite animation
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsProcessDone
// Returns ETrue if loading is done.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::IsProcessDone() const
    {
    return iIsLoadingDone;
    }
   
// -----------------------------------------------------------------------------
// CSVGTCustControl::SetCurrentFullScreenStatus
// Set function for current viewing mode.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::SetCurrentFullScreenStatus(
    TBool aIsFullScreenOn )  //  Full Screen( ETrue )/Normal( EFalse )
    {
    iIsFullScreenOn = aIsFullScreenOn;
    }   
// -----------------------------------------------------------------------------
// CSVGTCustControl::HandleApplicationForegroundEvent
// Does the handling of foreground event occurring.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::HandleApplicationForegroundEvent( TBool aForeground )
    { 
    iIsForeground = aForeground;
    if ( !aForeground )
        {
        // Application going into background, pause presentation.
        // We need to check first if there are any
        // animations at all in this presentation and pause
        // makes sense or not.
        if ( IsAnimationPresent() && iAnimState == ESvgAnimPlayingState )
            {
            TRAPD(errProcessViewerCmd,
                ProcessViewerCommandL( ESvgtDlgPauseCommand ));
           
            if ( errProcessViewerCmd != KErrNone )
                {
                // Error occurred , Stop Processing
                return;
                }
               
            if ( iBlinker->IsActive() )               
                {
                iBlinker->Cancel();
                iPauseIconVisible = ETrue;
                }
            }
        }
    else
        {
        // Application coming into foreground.
        // May need to start a flashing pause indicator so
        // that user would come to know presentation is paused.
        if ( IsAnimationPresent() && iAnimState == ESvgAnimPausedState )
            {
            if ( iPauseIconVisible )
                {
                ShowPauseIcon( ETrue );
                }
            }
        // Perform a redraw to avoid corrupted bitmap
        iSvgModule->Redraw();
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsTextSelectOn
// Indicates whether the "Select Text" menu item be displayed.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::IsTextSelectOn() const
    {
    return ( iIsTextSelectOn );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsTextEditOn
// Indicates whether the "Edit Text" menu item be displayed.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::IsTextEditOn() const
    {
    return ( iIsTextEditOn );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsTextContentChanged
// Indicates whether the text in the content was modified.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::IsTextContentChanged() const
    {
    return ( iIsTextChanged );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::SaveComplete
// Used to perform any cleanup once save is complete.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::SaveComplete( const TInt aError )
    {
    if ( !aError )
        {
        iIsTextChanged = EFalse;
       
        //Upadate the MSK label once saving is done
        if(iMSKImpl && !iPreviewMode)
        	{
        	TRAP_IGNORE(SetMSKLabelL());	
        	}
       
        }
    }
   
// -----------------------------------------------------------------------------
// CSVGTCustControl::UpdateScreen
// Implements MSvgRequestObserver::UpdateScreen.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::UpdateScreen()
    {
    if ( IsContentFinite() )
        {
        ResetBacklightTime();    
        }
    if ( iSVGTBitMapMask )
        {
        // Generate the mask for the bitmap
        iSvgModule->GenerateMask( iSVGTBitMapMask );
        }
	iSvgModule->IsPanPossibleFourWay(iPanLt,iPanRt,iPanUp,iPanDn);
       
    // Find the new position of bounding boxes if
    // Find Text was performed
    if ( iIsSearchStrValid )
        {
        CalcFindTextBBox();
        }
    if ( iIsLoadingDone )
        {
        DrawNow();
        }   
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ScriptCall
// Implements MSvgRequestObserver::ScriptCall.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::ScriptCall( const TDesC& /*aScript*/,
                                  CSvgElementImpl* /*aCallerElement*/ )
    {
    return EFalse;
    }


// -----------------------------------------------------------------------------
// CSVGTCustControl::FetchImage
// Implements MSvgRequestObserver::FetchImage. It executes asynchronously.
// To fetch the image each time the KErrGeneral should be return.
// -----------------------------------------------------------------------------
//   
TInt  CSVGTCustControl::FetchImage( const TDesC& aUri,
             RFs& aSession, RFile& aFileHandle )
    {

    TInt ret = 0;

    iImageFileType = ImageFileType(aUri);
    if ( iAppObserver )
        {
        ret = iAppObserver->FetchImage( aUri, aSession, aFileHandle ) ;
        }
    else
        {
        return KErrNotFound;
        }

    if ( !iFetchImageError )
        {
        iFetchImageError = ret;
        }

    if(iImageFileType == ELocalImageFile)
        {
        return ret;
        }
    else
        {
        return KErrGeneral;
        }
    }

TInt CSVGTCustControl::FetchFont( const TDesC& aUri ,
                                    RFs& aSession , RFile& aFileHandle )
    {
    _LIT(KPath, "C:\\data\\images\\");
    TBuf<KMaxPath> fileName;
    fileName.Copy(KPath);
    fileName.Append(aUri);
    return aFileHandle.Open( aSession, fileName, EFileShareReadersOnly);
    }
	  
// -----------------------------------------------------------------------------
// CSVGTCustControl:: GetSmilFitValue
// Implements the MSvgRequestObserver::GetSmilFitValue.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::GetSmilFitValue( TDes& aSmilValue )
    {
    aSmilValue.Copy( KSccSmilFitValue );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::UpdatePresentation
// Implements MSvgRequestObserver::UpdatePresentation.
// -----------------------------------------------------------------------------
//
void  CSVGTCustControl::UpdatePresentation( const TInt32&  aNoOfAnimation )
    {
    if ( iTotalNumerOfAnimation == KSccConstMinusOne )
        {
        if ( aNoOfAnimation == KSccConstZero )
            {
            iAnimationStatus = ESvgNoAnimationsPresent;
            return;
            }
        else
            {
            // First time Update Presentation is called
            // and animations are present.
            iTotalNumerOfAnimation = aNoOfAnimation;
            }
        }
    else
        {
        iTotalNumerOfAnimation -= aNoOfAnimation;
        }

    if ( iTotalNumerOfAnimation == KSccConstZero )
        {
        iAnimationStatus = ESvgAllAnimationsComplete;
        return;
        }

    iAnimationStatus =  ESvgAnimationsNotComplete;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::OfferKeyEventL
// Handles Key events by reimplementing CCoeControl::OfferKeyEventL.
// Returns:
//      EKeyWasConsumed: If this control uses this key.
//      EKeyWasNotConsumed: Otherwise.
// -----------------------------------------------------------------------------
//
TKeyResponse CSVGTCustControl::OfferKeyEventL(
    const TKeyEvent& aKeyEvent,  // Key Event
    TEventCode aType )           // Event Code
    {
   
    TKeyResponse retVal = EKeyWasNotConsumed;
    TInt keyScanCode = aKeyEvent.iScanCode;
    TInt keyCode = aKeyEvent.iCode;
    switch ( aType )
        {
        case EEventKeyDown:
            {
            // reset the repeat count
            iKeyRepeatCount = 0;
            // Reset the select cause
            iSelectCause = ESvgInvalidSelectCause;
            // UI Spec: Pointer should be displayed when
            // Select Key or Pointer is moved
            if ( keyScanCode == EStdKeyEnter ||
                 keyScanCode == EStdKeyDevice3 ||
                 keyScanCode == EStdKeyUpArrow ||
                 keyScanCode == EStdKeyDownArrow ||
                 keyScanCode == EStdKeyLeftArrow ||
                 keyScanCode == EStdKeyRightArrow )
                {
                if ( !iInactivityTimer )
                    {
                    // Ignore subsequent key events as the pointer should be
                    // displayed and no event is to be processed until key
                    // up event is received.
                    iIgnorePointerKeyEvent = ETrue;
                    }
                if( keyScanCode != EStdKeyDevice3 && keyScanCode != EStdKeyEnter && !iIsPointerDisplayed )
                	{
                	iScrollKey=ETrue; //needed to check when the pointer hidden
                	                  // and scroll key is pressed.
                   	}
                
               
                if( ( keyScanCode == EStdKeyDevice3 || keyScanCode == EStdKeyEnter )&&!iIsPointerDisplayed )
                    {
                    // Key has been lifted,
                    // Reset the Pointer Delta Values
                    iPointerDx = KSccConstZero;
                    iPointerDy = KSccConstZero;

                    // UI Spec: Pointer should be hidden after 15 sec of
                    // user inactivity
                    if ( iInactivityTimer )
                        {
                        // Should not be the case!
                        delete iInactivityTimer;
                        iInactivityTimer = NULL;
                        }
                    iInactivityTimer = CSVGTPointerHideTimer::NewL( this );
                    iInactivityTimer->After( KSccPointerHideTimerStartAfter );
                    }
                if(keyScanCode == EStdKeyDevice3 || keyScanCode == EStdKeyEnter )    
                    {
                    iDownKeyOccurred = ETrue;
                    iSvgModule->MouseDown( iPointerX, iPointerY );    
                    }
                SetPointerDisplayStatusAndRedraw( ETrue );
                
                retVal = EKeyWasConsumed;
                }
            }
            break;
        case EEventKeyUp:
            {
            if ( keyScanCode == EStdKeyEnter ||
                 keyScanCode == EStdKeyDevice3 ||
                 keyScanCode == EStdKeyUpArrow ||
                 keyScanCode == EStdKeyDownArrow ||
                 keyScanCode == EStdKeyLeftArrow ||
                 keyScanCode == EStdKeyRightArrow )
                {
                // Key has been lifted,
                // Reset the Pointer Delta Values
                iPointerDx = KSccConstZero;
                iPointerDy = KSccConstZero;

                // UI Spec: Pointer should be hidden after 15 sec of
                // user inactivity
                if ( iInactivityTimer )
                    {
                    // Should not be the case!
                    delete iInactivityTimer;
                    iInactivityTimer = NULL;
                    }
                iInactivityTimer = CSVGTPointerHideTimer::NewL( this );
                iInactivityTimer->After( KSccPointerHideTimerStartAfter );

                // Once the pointer is un-hidden, Ignore subsequent key events
                // and no event is to be processed until key up event is received.
                if ( iIgnorePointerKeyEvent )
                    {
                    iIgnorePointerKeyEvent = EFalse;
                    retVal = EKeyWasConsumed;
                    TRAP_IGNORE(SetMSKLabelL());
                    }
                else
                    {
                    // Assume key will be consumed
                    retVal = EKeyWasConsumed;
                   
                    if( ( keyScanCode == EStdKeyDevice3 || keyScanCode == EStdKeyEnter )&& iDownKeyOccurred)
                        {
                        iSvgModule->MouseUp(iPointerX,iPointerY);    
                        iDownKeyOccurred= EFalse;
                        }
                    }
                
                }                           
            }
            break;
        case EEventKey:
            {
            retVal = EKeyWasConsumed;
            iKeyRepeatCount++;

            // Once the pointer is un-hidden, Ignore subsequent joystick 
            // key events and no joystick event is to be processed until 
            // key up event is received.
            switch ( keyCode /*keyScanCode*/ )
                {
                case KSccZoomInKey: // Zoom In Key
                    {
                    if ( iKeyRepeatCount == 1 )
                        {
                        ProcessViewerCommandL( ESvgtDlgZoomInCommand );
                        }
                     
                    break;
                    }
                case EKeyUpArrow: // Pointer Up
                    {
                    if ( iIgnorePointerKeyEvent == EFalse)
                        {
                        iPointerDx = KSccConstZero;
                        iPointerDy -= KSccCursorMinDeltaY;
                        }
                    break;
                    }
                case EKeyDownArrow: // Pointer Down
                    if ( iIgnorePointerKeyEvent == EFalse)
                        {
                        iPointerDx = KSccConstZero;
                        iPointerDy += KSccCursorMinDeltaY;
                        }
                    break;
                case EKeyLeftArrow: // Pointer Left
                    if ( iIgnorePointerKeyEvent == EFalse)
                        {
                        iPointerDx -= KSccCursorMinDeltaX;
                        iPointerDy = KSccConstZero;
                        }
                    break;
                case EKeyRightArrow: // Pointer Right
                    if ( iIgnorePointerKeyEvent == EFalse)
                        {
                        iPointerDx += KSccCursorMinDeltaX;
                        iPointerDy = KSccConstZero;
                        }
                        break;
                case ENumberKey1:    //key 1
                    DoRotation(ERotationAngle90);
                    break;
                case ENumberKey3:    //key 3
                    DoRotation(-ERotationAngle90);
                    break;
                case ENumberKey7:    //key 7
                    DoRotation(ERotationAngle45);
                    break;
                case ENumberKey9:    //key 9
                    DoRotation(-ERotationAngle45);
                    break;
                case KSccZoomOutKey: // Zoom out key
                    // Zoom out if "zoom-out" key was pressed once
                    {
                    if ( iKeyRepeatCount == 1 )
                        {
                        ProcessViewerCommandL( ESvgtDlgZoomOutCommand );
                        }
                    else if ( ( iZoomLevel > 0 ) && ( iKeyRepeatCount == 2 ) )
                        {
                        ProcessViewerCommandL(ESvgtDlgZoomOptimalCommand );
                        }                             
                    break;    
                    }
                default:
                    {
                    // Unknown Key
                    retVal = EKeyWasNotConsumed;
                    }
                }
            }
            break;
        default:
            {
            // Unknown Key
            retVal = EKeyWasNotConsumed;
            }
            break;
        }
    if ( ( iPointerDx != KSccConstZero ) ||
            ( iPointerDy != KSccConstZero ) )
        {
        UpdatePointer();
        }
    return retVal;
    }

   
// -----------------------------------------------------------------------------
// CSVGTCustControl::HandleResourceChange
// This function is called by the framework when there is a layout switch/skin
// change
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::HandleResourceChange( TInt aType )
    {
    if ( aType == KAknsMessageSkinChange )
        {
        // Recreate the icons
        TRAPD( err, CreateIconsL() );
        if ( err !=  KErrNone )
            {
            // No Error Handling done here.
            }
        // Find Text Highlight Colour
        MAknsSkinInstance* skin = AknsUtils::SkinInstance();
       
        // Color is not updated if it not found from the skin
        AknsUtils::GetCachedColor(skin, iFindTextHiColor,
                                  KAknsIIDQsnHighlightColors,
                                  EAknsCIQsnHighlightColorsCG2);
        SetSize(Rect().Size());                          
        }

    CCoeControl::HandleResourceChange( aType );       
    }

       
// -----------------------------------------------------------------------------
// CSVGTCustControl::LinkEntered
// Callback Function called by the engine when the pointer enters a hyperlink
// -----------------------------------------------------------------------------
//

TBool CSVGTCustControl::LinkEntered( const TDesC& aUri)
    {
   
    CXmlElementImpl* ptr=NULL;
   
    // Get the elementID
    ptr=iSvgModule->GetElementById(iSvgModule->SvgDocument(),aUri);
	
	// Check for "a" element or an element which has animation on
	// mousedown/mouseup 
	if(!ptr)
		{
		iAElement=ETrue;
		iPointerType = ESvgPointerHand;
		// Sets the MSK label
        if(iMSKImpl && !iPreviewMode )
    	    {
    	    TRAP_IGNORE(SetMSKLabelL());	
    	    }  
		}
    
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::LinkExited
// Callback Function called by the engine when the pointer exits a hyperlink
// -----------------------------------------------------------------------------
//

TBool CSVGTCustControl::LinkExited( const TDesC& /*aUri*/ )
    {
   
    iPointerType = ESvgPointerDefault;
   
    // Reset the flag for a element
    iAElement=EFalse;
   
    //Set the MSK label
    if(iMSKImpl && !iPreviewMode)
    	{
    	TRAP_IGNORE(SetMSKLabelL());	
    	}
   
    return ETrue;
    }
// ---------------------------------------------------------------------------
// CSVGTCustControl::LinkActivated
// Callback Function called by the engine when a hyperlink is activated
// ---------------------------------------------------------------------------
//

TBool CSVGTCustControl::LinkActivated(
    const TDesC& aUri )
    {
    if (iSelectCause != ESvgInvalidSelectCause )
        {
        // Ignore this callback as it was already processed.
        return ETrue;
        }
    // Set the cause for select operation as Hyperlink so that
    // subsequent events trigerred are ignored.
    iSelectCause = ESvgHyperLinkSelectCause;
   
    if( iAppObserver )
        {
        if( IsThreadRunning() )
            {
            CSvgtEventLinkActivated* event = new 
                CSvgtEventLinkActivated( aUri );
            iEventHandlerAO->AddEventToList( event );
            iEventHandlerAO->MakeRequestComplete( KErrNone );
            return ETrue;
            }
        else
            {
            return iAppObserver->LinkActivated( aUri );    
            }
        }
    else
        {
        return EFalse;
        }
    }

// --------------------------------------------------------------------
// CSVGTCustControl::LinkActivatedWithShow
// Callback Function called by the engine when a hyperlink with show
// attribute is activated
// --------------------------------------------------------------------
//
TBool CSVGTCustControl::LinkActivatedWithShow(
    const TDesC& aUri ,
    const TDesC& aShow )
    {
    if (iSelectCause != ESvgInvalidSelectCause )
        {
        // Ignore this callback as it was already processed.
        return ETrue;
        }   
    // Set the cause for select operation as Hyperlink so that
    // subsequent events trigerred are ignored.
    iSelectCause = ESvgHyperLinkSelectCause;
    if( iAppObserver )
        {
        if( IsThreadRunning() )
            {
            CSvgtEventLinkActivatedWithShow* event = 
                new CSvgtEventLinkActivatedWithShow( aUri, aShow );
            iEventHandlerAO->AddEventToList( event );
            iEventHandlerAO->MakeRequestComplete( KErrNone );
           
            return ETrue;
            }
        else
            {
            return iAppObserver->LinkActivatedWithShow( aUri, aShow );     
            }
        }
    else
        {
        return EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::CreateBitmapsL
// This function is called to create the bitmaps used by the SVG engine
// for rendering.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::CreateBitmapsL(
    const TSize& aBitmapSize ) // Bitmap Size used to render content
    {
   
    if( iSVGTBitMap && (iSVGTBitMap->SizeInPixels() == aBitmapSize))
        {
        return ;
        }
   
    TDisplayMode dispMode = iEikonEnv->ScreenDevice()->DisplayMode();
    TInt errCode = KErrNone;
    // Modes currently supported by SVG engine are:
    // EGray2, EColor4K, EColor64K, EColor16M, EColor16MU.
    switch ( dispMode )
        {
        case EGray2:
        case EColor16M:
            {
            break;
            }
        case EColor16MU:
            {
            break;
            }
        default:
            {
            dispMode = EColor64K;
            break;
            }
        }
    // Create the bitmaps using local variables and assign
    // to member variable if successful
    CFbsBitmap* lTempBitmap = new ( ELeave ) CFbsBitmap();
   
    // Create the bitmap with size and display mode
    errCode = lTempBitmap->Create( aBitmapSize, dispMode );
    if ( errCode != KErrNone )
        {
        delete lTempBitmap;
        User::Leave( errCode );
        }
   
    // Check and destroy existing bitmap
    if ( iSVGTBitMap )
        {
        delete iSVGTBitMap;
        }
   
    // Assign newly created bitmap   
    iSVGTBitMap = lTempBitmap;
   
    // Create the bitmap mask
    lTempBitmap = new ( ELeave ) CFbsBitmap();
   
    // Create the bitmap with size and display mode
    errCode = lTempBitmap->Create( aBitmapSize, EGray256 );
    if ( errCode != KErrNone )
        {
        delete lTempBitmap;
        User::Leave( errCode );
        }

    // Check and destroy existing bitmap
    if ( iSVGTBitMapMask )
        {
        delete iSVGTBitMapMask;
        }
   
    // Assign newly created bitmap
    iSVGTBitMapMask = lTempBitmap;
       
    // Create the Framebuffer Bitmap
    lTempBitmap = new ( ELeave ) CFbsBitmap();
   
    // Create the bitmap with size and display mode
    errCode = lTempBitmap->Create( aBitmapSize, dispMode );
    if ( errCode != KErrNone )
        {
        delete lTempBitmap;
        User::Leave( errCode );
        }

    // Check and destroy existing bitmap
    if ( iSVGTFrameBufBitMap  )
        {
        delete iSVGTFrameBufBitMap;
        }
   
    // Assign newly created bitmap   
    iSVGTFrameBufBitMap = lTempBitmap;
       
    // Create the Bitmap Device and Contexts, used to reduce the number
    // of interactions with the windows server
    CFbsBitmapDevice* lTempBitmapDev = CFbsBitmapDevice::NewL(
            iSVGTFrameBufBitMap );
   
    CFbsBitGc* lTempBitmapCtx = NULL;
    // Create framebuffer context
    errCode = lTempBitmapDev->CreateContext( lTempBitmapCtx );
    if ( errCode != KErrNone )
        {
        delete lTempBitmapDev;
        User::Leave( errCode );
        }
   
    // Check and destroy existing bitmap device
    if ( iSVGTFrameBufDevice )
        {
        delete iSVGTFrameBufDevice;
        }
   
    // Check and destroy existing bitmap context
    if ( iSVGTFrameBufContext )
        {
        delete iSVGTFrameBufContext;
        }
   
    // Assign newly created bitmap device
    iSVGTFrameBufDevice = lTempBitmapDev;
    iSVGTFrameBufContext = lTempBitmapCtx;
    }
// -----------------------------------------------------------------------------
// CSVGTCustControl::CreateIconsL
// This function is called to create the icons used by the control.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::CreateIconsL()
    {
    // Skin instance
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();

    // Cleanup the old icons.
    FreeIcons();
   
    // Construct the MBM file name
    TParse parse;
    parse.Set( KSccIconFile, &KDC_APP_BITMAP_DIR, NULL);
    TPtrC iconFile = parse.FullName();

    //Create icon for drawing the pointer
    iPointerIcon = AknsUtils::CreateGulIconL(
            skin,
            KAknsIIDQgnIndiViewerPointer,
            iconFile,
            EMbmSvgtuicontrolQgn_indi_viewer_pointer,
            EMbmSvgtuicontrolQgn_indi_viewer_pointer_mask );

    //Create icon for drawing the hyperlink pointer
    iHandIcon = AknsUtils::CreateGulIconL(
            skin,
            KAknsIIDQgnIndiViewerPointerHand,
            iconFile,
            EMbmSvgtuicontrolQgn_indi_viewer_pointer_hand,
            EMbmSvgtuicontrolQgn_indi_viewer_pointer_hand_mask );

    //Create icon for drawing the hyperlink pointer
    iTextIcon = AknsUtils::CreateGulIconL(
           skin,
            KAknsIIDQgnIndiViewerPointerText,
            iconFile,
            EMbmSvgtuicontrolQgn_indi_viewer_pointer_text,
            EMbmSvgtuicontrolQgn_indi_viewer_pointer_text_mask );

    // Panning Indicators
    // Create icon for drawing the Pan Left
    iPanIndLeftIcon = AknsUtils::CreateGulIconL(
            skin,
            KAknsIIDQgnIndiViewerPanningLeft,
            iconFile,
            EMbmSvgtuicontrolQgn_indi_viewer_panning_left,
            EMbmSvgtuicontrolQgn_indi_viewer_panning_left_mask );

    // Create icon for drawing the Pan Right
    iPanIndRightIcon = AknsUtils::CreateGulIconL(
            skin,
            KAknsIIDQgnIndiViewerPanningRight,
            iconFile,
            EMbmSvgtuicontrolQgn_indi_viewer_panning_right,
            EMbmSvgtuicontrolQgn_indi_viewer_panning_right_mask );

    // Create icon for drawing the Pan Up
    iPanIndUpIcon = AknsUtils::CreateGulIconL(
            skin,
            KAknsIIDQgnIndiViewerPanningUp,
            iconFile,
            EMbmSvgtuicontrolQgn_indi_viewer_panning_up,
            EMbmSvgtuicontrolQgn_indi_viewer_panning_up_mask );

    // Create icon for drawing the Pan Down
    iPanIndDownIcon = AknsUtils::CreateGulIconL(
           skin,
           KAknsIIDQgnIndiViewerPanningDown,
           iconFile,
           EMbmSvgtuicontrolQgn_indi_viewer_panning_down,
           EMbmSvgtuicontrolQgn_indi_viewer_panning_down_mask );

    // Create icon for drawing the Pause
    iPauseIcon = AknsUtils::CreateGulIconL(
           skin,
           KAknsIIDQgnIndiCamsPaused,
           iconFile,
           EMbmSvgtuicontrolQgn_indi_cams_paused,
           EMbmSvgtuicontrolQgn_indi_cams_paused_mask );
    }
// -----------------------------------------------------------------------------
// CSVGTCustControl::InitializeEngineL
// Initiliazes the interface with the SVG engine.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::InitializeEngineL()
    {
    if ( !iSvgModule )
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
       
        iSvgModule = CSvgEngineInterfaceImpl::NewL( iSVGTBitMap,
                this, spec );
        iSvgModule->SetBackgroundColor( KSccArgbWhite );
        iSvgModule->AddHyperlinkListener( static_cast < MSvgHyperlinkListener* >
                                        (this) );
        // Register for notification of Animation Start/Stop.
        iSvgModule->AddAnimationListener( static_cast < MSvgAnimationListener* >
                         ( this ) );
        iSvgModule->AddListener(static_cast<MSvgLoadingListener*>(this),
            ESvgLoadingListener);
        iSvgModule->AddListener(static_cast<MSvgLoadingListener*>(this),
            ESvgHyperlinkListener);
        iSvgModule->AddListener( static_cast < MSvgInteractiveElementListener* >
                    (this), ESvgInteractiveElementListener);

#ifdef SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE   
        // In DRM protected contents, disable the text operations:
        iEnableTextOps = !( iSvgDrmHelper->IsProtected()||iPreviewMode );
       
        if ( iEnableTextOps )
            {
            // Add textArea element listener
            iSvgModule->AddListener( static_cast<MSvgTextAreaListener *>(this),
                                     ESvgTextAreaListener );
           
            // Add text element listener
            iSvgModule->AddListener( static_cast<MSvgTextListener *>(this),
                                     ESvgTextListener );
            }
#endif // SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE                           
       }

    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::LoadContentL
// This function uses the SVG Engine to load the SVGT content.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::LoadContentL(
    RFile& aFileHandle )    // Filename of SVGT content
    {
    // Enable DRM mode of engine
    iSvgModule->SetDRMMode( ETrue );
   
    // Rewind the file
    TInt pos = 0;
    aFileHandle.Seek( ESeekStart, pos );
   
    iLoadingThreadError = iSvgModule->Load( aFileHandle );

#ifdef SVGTUICONTROL_DBG_OPENVG_TESTING_ENABLE   
    iSvgModule->CustomOption( ETrue );
#endif // SVGTUICONTROL_DBG_OPENVG_TESTING_ENABLE   
    }

TInt CSVGTCustControl::LoadSvgContent(TAny* aAny)
    {
    CSVGTCustControl& loader = *( CSVGTCustControl* )aAny;
    CTrapCleanup* theTrapCleanup = CTrapCleanup::New();
    loader.SetThreadRunning(ETrue);
    TRAP_IGNORE( loader.InitSvgContentL() );
    delete theTrapCleanup;
    return 0;
    }

void CSVGTCustControl::InitSvgContentL()
    {
    CActiveScheduler* threadScheduler = new (ELeave) CActiveScheduler;
    CleanupStack::PushL( threadScheduler );
    CActiveScheduler::Install(threadScheduler);

    // Load the SVGT content
    LoadContentL( iContentFileHandle );
    CleanupStack::PopAndDestroy( threadScheduler );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::DisplayInfoNoteL
// This is a utility function for displaying the info notes
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::DisplayInfoNoteL( TInt aResourceId, // Resource Id of
                                                           // string
                                 TBool aIsWaitingDialog ) // Indicate whether \
                                                          // dialog should wait
    {
    if(iDisplayFlags & ESvgDisplayErrorNotes )
        {
        // Show information note
        HBufC* prompt = iCoeEnv->AllocReadResourceLC( aResourceId );
        CAknInformationNote* note = new ( ELeave ) CAknInformationNote(
                                                            aIsWaitingDialog );
        note->ExecuteLD( *prompt );
        CleanupStack::PopAndDestroy( prompt );
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::DoPostLoadFuncL
// This is a utility function for performing post-load functionality
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::DoPostLoadFuncL()
    {
    if ( iSvgModule )
        {
#ifdef RD_SVGT_AUDIO_SUPPORT
        SetPresentationVolume( iVolumeLevel );
#endif //RD_SVGT_AUDIO_SUPPORT

        // give thread normal priority
        iMainThread.SetPriority( EPriorityNormal );
        // Attach to property for call state property
        User::LeaveIfError( iSystemState.Attach( KPSUidCtsyCallInformation,
                                                 KCTsyCallState ) );
        // Attach to property for call type property
        User::LeaveIfError( iSystemState.Attach( KPSUidCtsyCallInformation,
                                                 KCTsyCallType ) );
        // Indicate that the content loading is complete
        iIsLoadingDone = ETrue;
   
        // Store SVG duration
        iDuration = iSvgModule->Duration();   

        // Start the rendering process
        iAnimState = ESvgAnimPlayingState;
        iSvgModule->Start();
		
		//Drawing the MSK label for the first time
		if(iMSKImpl && !iPreviewMode)
			{
			TRAP_IGNORE(SetMSKLabelL());
			}
			
        // Check whether the content is finite, inorder to display the
        // progress timer
        if ( IsContentFinite() )
            {
            // Start immediately so that the first count is displayed
            StartProgIndTimer( 0 );
            }

        if ( IsVoiceCallActive() )
            {
            ProcessPauseCmd();
            }
        }
    }   

// -----------------------------------------------------------------------------
// CSVGTCustControl::SaveSvgDom
// This function is a helper function to save the modified svg content
// given the file name.
// -----------------------------------------------------------------------------
TInt CSVGTCustControl::SaveSvgDom( const TDesC& aFileName )
    {
    TInt retVal = KErrNone;
    MSvgError* lSvgErr = iSvgModule->SaveSvgDom(
        (TInt)iSvgModule->SvgDocument(), aFileName );
    if ( ( !lSvgErr ) || ( lSvgErr->HasError() ) )
        {
        if ( lSvgErr )
            {
            // Attempt to find the system error code
            retVal = lSvgErr->SystemErrorCode();
            }
            else
                {
                // No free memory available
                retVal = KErrNoMemory;
                }
       
        // No error code found, indicate general error   
        if ( retVal == KErrNone )
            {
            retVal = KErrGeneral;
            }
        }
    return retVal;
    }
 // -----------------------------------------------------------------------------


//-----------------------------------------------------------------------------    
void CSVGTCustControl::GetViewPort( TInt  getWidth , 
                                    TInt  getHeight , 
                                    TBool isWidthInPercentage, 
                                    TBool isHeightInPercentage, 
                                    TInt& setWidth, TInt& setHeight )
 {
    if(!iSvgModule)
    {
        return;
    }
    if(!iSVGTBitMap)
    {
        return;
    }
    TSize svgSize = iSvgModule->ContentDimensionsInPercentage();
    TInt  per=100;
    TSize tempSize = iSvgModule->ContentDimensions();
    TInt scrW = iSVGTBitMap->SizeInPixels().iWidth;
    TInt scrH = iSVGTBitMap->SizeInPixels().iHeight;
    //both not in percentage
    if(!isWidthInPercentage && !isHeightInPercentage)
    {
        setWidth = tempSize.iWidth;
        setHeight = tempSize.iHeight;
    }
    //if width in percentage 
    else if(isWidthInPercentage && !isHeightInPercentage)
    {
        setWidth = svgSize.iWidth * scrW / per;
        //if svg had valid height take it else use default
        if(tempSize.iHeight)
        {
         setHeight = getHeight;
        }
    }
    //if height in percentage
    else if(!isWidthInPercentage && isHeightInPercentage)
    {
        setHeight =  svgSize.iHeight * scrH / per;
        if(tempSize.iWidth)
        {
         setWidth = getWidth;
        }
    }
    //if both height and width are in percentage
    else
    {
        setHeight =  svgSize.iHeight * scrH / per;
        setWidth = svgSize.iWidth * scrW / per;
    }
    
    //if height is greater than screenHeight
    //or if width is greater than screenwidth 
    //set the width and height by preserving the aspect ratio
    TReal32 sx,sy;

    if(setHeight > scrH || setWidth > scrW)
    {
        sx = (TReal32) (scrW) / (TReal32) (setWidth);
        sy = (TReal32) (scrH) / (TReal32) (setHeight);
        if ( sx > sy )
        {
            sx = sy;
        }
        else // ( sx < sy )
        {
            sy = sx;
        }
        setWidth = sx * setWidth;
        setHeight = sy * setHeight;
    }
    
    
 }
 
 
// -----------------------------------------------------------------------------
// CSVGTCustControl::DrawPointer
// This function blits the pointer bitmap on the screen at the current mouse
// co-ordinates which are maintained as data-members of CSVGTCustControl.
// -----------------------------------------------------------------------------
//

void CSVGTCustControl::DrawPointer() const
    {

    iSVGTFrameBufContext->SetPenStyle( CGraphicsContext::ENullPen );
    iSVGTFrameBufContext->SetBrushStyle( CGraphicsContext::ENullBrush );
   
   
    // Use the aid for the hotspot from the LAF.
    // This value is set in the SizeChanged function.
    // The hotspot information is used so that the bitmap is drawn in
    // correct position with respect to the mouse position.
    switch ( iPointerType )
        {
        case ESvgPointerHand:
            {
            // Draw the hand icon at the mouse position
            TPoint pointerStart( iPointerX - iHandAidRect.Rect().Width(),
                                 iPointerY - iHandAidRect.Rect().Height() );
            iSVGTFrameBufContext->BitBltMasked( pointerStart,
                 iHandIcon->Bitmap(),
                 iHandRect.Rect(),
                 iHandIcon->Mask(),
                 EFalse );
            break;
            }
        case ESvgPointerText:
            {
            // Draw the hand icon at the mouse position
            TPoint pointerStart( iPointerX - iTextAidRect.Rect().Width(),
                                 iPointerY - iTextAidRect.Rect().Height() );
            iSVGTFrameBufContext->BitBltMasked( pointerStart,
                 iTextIcon->Bitmap(),
                 iTextRect.Rect(),
                 iTextIcon->Mask(),
                 EFalse );
            break;
            }
        case ESvgPointerDefault:      // Fall Through
        default:                      // Fall Through - Should never be the case
            {
            // Draw the pointer icon at the mouse position
            TPoint pointerStart( iPointerX - iPointerAidRect.Rect().Width(),
                                 iPointerY - iPointerAidRect.Rect().Height() );
            iSVGTFrameBufContext->BitBltMasked( pointerStart,
                iPointerIcon->Bitmap(),
                iPointerRect.Rect(),
                iPointerIcon->Mask(),
                EFalse );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::UpdatePointer
// Called when pointer needs to be redrawn
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::UpdatePointer()
    {
    // Get the size of the content
    TSize svgSize = iSvgModule->Size();
    TInt rgnw = svgSize.iWidth;
    TInt rgnh = svgSize.iHeight;

    // This is to store whether the framebuffer changed
    TBool lFrameBufChange = EFalse;
   
    // iPointerDx contains the delta increment in X axis
    // Add it to the pointer coordinates
    iPointerX += iPointerDx;
   
    // If pointer coordinate becomes negative then reset it
    // and perform panning.
    if ( iPointerX < KSccConstZero )
        {
        iPointerX = KSccConstZero;
		if(iPanLt)
			{
			iSvgModule->Pan( KSccPanNumPixelsX, KSccConstZero );
			}
        lFrameBufChange = ETrue;
        }
   
    // If pointer coordinate becomes greater than content size
    // then set it back to (content width - 1) and
    // perform panning.
    if ( rgnw <= iPointerX )
        {
        iPointerX = rgnw - KSccConstOne;
		if(iPanRt)
			{
    	    iSvgModule->Pan( -KSccPanNumPixelsX, KSccConstZero );
			}
        lFrameBufChange = ETrue;
        }

    // iPointerDy contains the delta increment in Y axis
    // Add it to the pointer coordinates.
    iPointerY += iPointerDy;

    // If pointer coordinate becomes negative then reset it
    // and perform panning.
    if ( iPointerY < KSccConstZero )
        {
        iPointerY = KSccConstZero;
		if(iPanUp)
			{
            iSvgModule->Pan( KSccConstZero, KSccPanNumPixelsY );
			}
        lFrameBufChange = ETrue;
        }

    // If pointer coordinate becomes greater than content size
    // then set it back to (content height - 1) and
    // perform panning.
    if ( iPointerY >= rgnh )
        {
        iPointerY = rgnh - KSccConstOne;
		if(iPanDn)
		    {
	        iSvgModule->Pan( KSccConstZero, -KSccPanNumPixelsY );
		    }
        lFrameBufChange = ETrue;
        }
   
    // If Framebuffer changed, perform an engine redraw and recalculate
    // bound box coordinates
    if ( lFrameBufChange )
        {
        iSvgModule->Redraw();
        CalcFindTextBBox();       
        }
   
    // Indicate new mouse position to engine
    iSvgModule->MouseMove( iPointerX, iPointerY );
   
    // Perform a redraw to reflect new position of pointer
    DrawDeferred();
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::SetMSKLabel
// This function is called to set the MSK labels under various circumstances.
// -----------------------------------------------------------------------------
//   
void CSVGTCustControl::SetMSKLabelL()
{
	TInt lResourceID=0;
	TInt lCommandID=0;
	if(!iIsPointerDisplayed)
		{
		if (IsAnimationPresent())
			{
			lResourceID=R_SVGT_MSK_LABEL_CONTEXT;
			lCommandID=EAknSoftkeyContextOptions;
			}
		else if ( !IsAnimationPresent() && 
		    (iAppObserver->CanShowSave() || IsTextContentChanged() ) )
			{
			lResourceID=R_SVGT_MSK_LABEL_SAVE;
			lCommandID=EAknSoftkeySave;	
			}
		else
			{	
			lResourceID=R_SVGT_MSK_LABEL_CONTEXT;
			lCommandID=EAknSoftkeyOptions;		
			}
		}
	else
		{
		switch(iPointerType)
			{
			case ESvgPointerHand:
				{
				lCommandID=EAknSoftkeySelect;	
				if(iAElement)
					{
					lResourceID=R_SVGT_MSK_LABEL_OPEN;
					}
				else
					{
					iPlayMSKLabel=ETrue;
					lResourceID=R_SVGT_MSK_LABEL_PLAY;
					}
				}
				break;			
			case ESvgPointerText:
				{
				lCommandID=EAknSoftkeySelect;
				if(IsTextEditOn())
					{
					lResourceID=R_SVGT_MSK_LABEL_EDIT;
					}
				else
					{
					lResourceID=R_SVGT_MSK_LABEL_SELECT;
					}
				}
				break;
			case ESvgPointerDefault:
				{
				if (IsAnimationPresent())
					{
					lResourceID=R_SVGT_MSK_LABEL_CONTEXT;
					lCommandID=EAknSoftkeyContextOptions;
					}
				else if( !IsAnimationPresent() && 
				    ( iAppObserver->CanShowSave() || IsTextContentChanged() ) )
					{
					lResourceID=R_SVGT_MSK_LABEL_SAVE;
					lCommandID=EAknSoftkeySave;	
					}
				else
					{	
					lResourceID=R_SVGT_MSK_LABEL_CONTEXT;
					lCommandID=EAknSoftkeyOptions;		
					}
				}
				break;
			}
		}
	iMSKImpl->RemoveMSKLabel();
	iMSKImpl->DrawMSKLabelL(lResourceID,lCommandID);
}

// -----------------------------------------------------------------------------
// CSVGTCustControl::GetMSKLabelFlag
// This function is called to get either iPlayMSKLabel or
// iContextMenuActivated flag
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::GetMSKLabelFlag(TInt aButtonId) const
{
	if (aButtonId==EAknSoftkeyContextOptions)
		{
		return iContextMenuActivated;	
		}
	else   //Else the case will be for EAknSoftkeySelect for which the
           // iPlayMSKLabel has to be returned
		{
		return iPlayMSKLabel;	
		}
}

// -----------------------------------------------------------------------------
// CSVGTCustControl::SetMSKLabelFlag
// This function is called to set/reset either iPlayMSKLabel or
// iContextMenuActivated flag
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ChangeMSKLabelFlag(TInt aButtonId)
{
	if (aButtonId==EAknSoftkeyContextOptions)
		{
		iContextMenuActivated = !iContextMenuActivated;
		}
	else //Else the case will be for EAknSoftkeySelect
		{
		iPlayMSKLabel=!iPlayMSKLabel;
		}
}

// -----------------------------------------------------------------------------
// CSVGTCustControl::SwitchScreenModeL
// This function is called to change the viewing mode to Normal Screen/Full
// Screen.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::SwitchScreenModeL()
    {
    // When control mode is being switched, following
    // processing needs to happen:
    //  a. Save the current framebuffer
    //  b. Create the New Framebuffer by calling CreateBitmaps()
    //  c. Set the framebuffer to the new framebuffer
    //  d. Pause the content if it was paused.
    //  e. Also processing in following functions is affected:
    //      i. UpdateScreen - This is the callback of the engine to indicate
    //         that it has rendered the frame on the bitmap, and is ready to be
    //         drawn. Here we need to generate mask with correct bitmap
    //      ii. Draw -  This function needs to bitblt the bitmap
    //         along with the mask

    if ( iSvgModule )
        {
        // Create new bitmaps
        CreateBitmapsL( Rect().Size() );
        iSvgModule->Pause();       
        // Change the rendering bitmap to the appropriate Screen Bitmap
        iSvgModule->SetFrameBuffer( iSVGTBitMap );  
       
        // Check if content was playing
        if ( iAnimState != ESvgAnimPlayingState )
            {
            // Stop it, as content is paused
            iSvgModule->Pause();
            // Redraw the screen, to prevent blank screen from appearing
            iSvgModule->Redraw();
            }
        // Find Text Update
        // Get new position of Bounding Boxes
        CalcFindTextBBox();
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::CalcFindTextBBox
// This function is called to recalculate the bounding boxes for the find text.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::CalcFindTextBBox()
    {

    // Clear the Arrays
    iBoundBoxArray.Reset();
    iFoundTextArray.Reset();
    iTextElemIdArray.Reset();

    // If the Search String is not valid, then
    // no need to do anything.
    if ( !iIsSearchStrValid )
        {
        return EFalse;
        }
       
    // Invoke Engine call for searching text.
    TBool retVal = iSvgModule->SearchForText(
        iLastSearchText,
        iBoundBoxArray,
        iFoundTextArray,
        iTextElemIdArray,
        EFalse );
   
    if ( retVal == EFalse )
        {
        // Set the search string as invalid
        iIsSearchStrValid = EFalse;
        // Reset the string
      //Himanshu  iLastSearchText.Zero();
        }
       
    return ( retVal );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::PanToCurBBox
// This function is called to pan the content inorder that the current
// bounding box is visible.
// -----------------------------------------------------------------------------
//

void CSVGTCustControl::PanToCurBBox()
    {
    TPoint lRefPos( Rect().Center() );

    // We need to pan in such a way that the Top Left of the Bounding box
    // coincides with the current view's Center
    TPoint lPanAmount( lRefPos - FPRCenter(
        iBoundBoxArray[ iCurSearchMatchId ] ) );
    iSvgModule->Pan( lPanAmount.iX, lPanAmount.iY );
    // Do a Redraw
    iSvgModule->Redraw();
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ProcessFindTextCmd
// This function is called to perform the necessary processing for the find
// text command.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ProcessFindTextCmdL()
    {
    // The Processing involved is
    // 1. Query the text to be searched for from the User
    // 2. Is the text same as the text input before? (Case Insensitive Match)
    // 3. If Yes, then
    //      Perform "Find Next" functionality
    //          a. If the bounding box is not in view then
    //                 move to the same bound box
    //             Else,
    //                 Cycle through the list of bound-boxes by incrementing
    //                 with wrap around the iCurSearchMatchId while the text
    //                 is not visible
    //    else // New text entered
    //      Perform "Find First" Functionality
    //          If no Match is found then,
    //                 Display an "Text not found" info note
    //                 Reset the Last Searched Text
    //                 return to caller
    //             Else,
    //                 Find the List of Bound Boxes
    //                 Set the Current Match as the Index Zero of the BBox Array
    // 4. If the current bound box is not in view
    //      Pan to the Bound Box
    //      Recalculate the Bound Box Co-ordinates
    // 5. Move the mouse to the center of the bound box
    // 6. Return to caller.

    TBuf<KFindTextQueryMaxLen> lText( iLastSearchText );
    // Query the text to be searched for from the User
    CAknTextQueryDialog* dlg = new ( ELeave ) CAknTextQueryDialog(
        iLastSearchText );
    if( dlg->ExecuteLD( R_FIND_TEXT_QUERY_DIALOG ) )
        {
        // User entered text
        if ( lText.CompareF( iLastSearchText ) == 0 && 
            iIsSearchStrValid==1 ) // Case insensitive comparison
            {
            // Searching for same text = Find Next
            // If current bound box is not in view and is visible, then
            //      move to the same bound box
            // else  ; Bound Box is in view or is Not Visible
            //      find next visible box with wraparound.
            TBool lIsBoxInView = FPRIntersects( 
                iBoundBoxArray[ iCurSearchMatchId ] ,
                Rect() );
            if ( lIsBoxInView  ||
                    ( !iSvgModule->IsElementVisible(
                      iTextElemIdArray[ iCurSearchMatchId ] ) ) )
                {
                // Store the current match Index so that the end of the list
                // can be identified.
                TInt lastSearchMatchId = iCurSearchMatchId;
                // Increment iIsCurMatchId to point to next matching text
                iCurSearchMatchId = ( iCurSearchMatchId + KSccConstOne ) %
                    ( iBoundBoxArray.Count() );
                while ( ( iCurSearchMatchId != lastSearchMatchId ) &&
                            ( !iSvgModule->IsElementVisible(
                               iTextElemIdArray[ iCurSearchMatchId ] ) ) )
                    {
                    // Increment iIsCurMatchId to point to next matching text
                    iCurSearchMatchId = ( iCurSearchMatchId + KSccConstOne ) %
                        ( iBoundBoxArray.Count() );
                    }
                    if( iCurSearchMatchId == lastSearchMatchId ||
                        (iCurSearchMatchId==0 )   )
                    {
                    ProcessNoTextFoundL();
                    return;
                    }
                }
            }
        else
            {
            // New Search String
            iCurSearchMatchId = KSccConstZero;
            iIsSearchStrValid = ETrue;

            if ( !CalcFindTextBBox() )
                {
                ProcessNoTextFoundL();
                return;
                }
                else
                    {
                    // Match Found,
                    // Find the first box in the list that is visible
                    if ( !iSvgModule->IsElementVisible(
                        iTextElemIdArray[ iCurSearchMatchId ] ) )
                        {
                        // Store the current match Index so that the end of the
                        // list can be identified.
                        TInt lastSearchMatchId = iCurSearchMatchId;
                        // Increment iIsCurMatchId to point to next matching
                        // text
                        iCurSearchMatchId = ( iCurSearchMatchId +
                                KSccConstOne ) % ( iBoundBoxArray.Count() );
                        while ( ( iCurSearchMatchId != lastSearchMatchId ) &&
                                    ( !iSvgModule->IsElementVisible(
                                     iTextElemIdArray[ iCurSearchMatchId ] ) ) )
                            {
                            // Increment iIsCurMatchId to point to next
                            // matching text
                            iCurSearchMatchId = ( iCurSearchMatchId +
                                    KSccConstOne ) % ( iBoundBoxArray.Count() );
                            }
                        if ( iCurSearchMatchId == lastSearchMatchId )
                            {
                            ProcessNoTextFoundL();
                            return;
                            }
                        }
                    }
            }

        // To determine if a bounding-box is in view:
        //   Take the intersection ( TRect::Intersects ) of the
        //   Cust Control's rect and the bounding-box. If Intersects()
        //   returns true, bounding-box is in view
        TRect myRect= Rect();
        TBool lIsBoxInView = FPRIntersects( iBoundBoxArray[ iCurSearchMatchId ],
            myRect );
       
        if ( !lIsBoxInView )
            {
            // Box is not in view

            // Pan Content to bring the text in view
            PanToCurBBox();

            // Get new position of Bounding Boxes
            CalcFindTextBBox();
            }

        // Make the mouse visible
        SetPointerDisplayStatusAndRedraw( ETrue );
        // Move Mouse over to the centre of the Bound Box
        TPoint lMousePos =
            FPRCenter( iBoundBoxArray[ iCurSearchMatchId ] );
        iPointerX = lMousePos.iX;
        iPointerY = lMousePos.iY;

        UpdatePointer();
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::DrawFindTextBBox
//  Draws the Bounding Box for the current match in the SVGT content.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::DrawFindTextBBox(
    const TRect& aRect ) const // Rectangle which needs to be redrawn
    {
    // Is there any text to highlight?
    if ( iIsSearchStrValid )
        {
        // To determine if a bounding-box can be drawn:
        //  Need to check whether it is in view && It is visible
        //
        //  Check whether it is in view:
        //   Take the intersection ( TRect::Intersects ) of the CoeControl-rect
        //   and the bounding-box.if Intersects() returns true,
        //   bounding-box is in view
        //  Check whether it is visible:
        //   Call Engine API to check whether the text elem is visible.

        TBool lCanDrawBox = ( FPRIntersects(
            iBoundBoxArray[ iCurSearchMatchId ], aRect ) &&
            iSvgModule->IsElementVisible( iTextElemIdArray[
                iCurSearchMatchId ] ) );

        if ( lCanDrawBox )
            {
            // Set the Draw Mode to XOR
            iSVGTFrameBufContext->SetDrawMode( CGraphicsContext::EDrawModeXOR );
           
            // Set the Brush Style to No Fill
            iSVGTFrameBufContext->SetBrushStyle(
                CGraphicsContext::ESolidBrush );
           
            // Set the Pen Style to Solid and color to one obtd from LAF
            iSVGTFrameBufContext->SetPenStyle( CGraphicsContext::ESolidPen );
            iSVGTFrameBufContext->SetPenColor( iFindTextHiColor );
           
            // Set the line thickness
            iSVGTFrameBufContext->SetPenSize( TSize( KSccConstTwo,
                        KSccConstTwo ) );
           
            // Set the brush to White so that when the color is XORed with it
            // it is displayed in its complementary colours.
            iSVGTFrameBufContext->SetBrushColor( KRgbWhite );
            // Form the clipping rectangle so that text beyound the
            // framebuffer is not displayed
            TRect lClipRect( TPoint( 0,0 ), iSvgModule->Size() );
           
            iSVGTFrameBufContext->SetClippingRect( lClipRect);
            CArrayFix<TPoint>* lArray = NULL;
            TRAPD( err,
                FPR2PointArrayL( iBoundBoxArray[ iCurSearchMatchId ], 
                lArray ) );
            if ( err != KErrNone )
                {
                delete lArray;
                return;
                }
               
            iSVGTFrameBufContext->DrawPolygon(
                lArray );
           
            delete lArray;
            lArray = NULL;
           
            // Reset the graphic context parameters so that subsequent
            // Draw() functions are not affected.
            iSVGTFrameBufContext->Reset();
            }
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ProcessNoTextFoundL
//  This function is called to perform the necessary procesing when
//  text being searched using Find Text command is not found.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ProcessNoTextFoundL()
    {
    // Set the search string as invalid
    iIsSearchStrValid = EFalse;
    // Reset the string
   //Himanshu iLastSearchText.Zero();
   
    // No match found, display an information note
    TInt resourceId;

    resourceId = R_SVGT_TEXT_NOT_FOUND;
    // Read from Resource
    HBufC* prompt = iCoeEnv->AllocReadResourceLC(
        resourceId );
   
    // Start a timer to delete the note after 3 seconds.
    if (!iNoteRemoverTimer->IsActive())
        {
        iNoteRemoverTimer->Start( KSccOneSecond * 3, KSccOneSecond * 3,
                TCallBack( RemoveInfoNote, this ) );
        }
    // Create and display the note
    iInfoNote = new ( ELeave ) CAknInformationNote(
            ETrue ); // ETrue - Waiting Dialog
    iInfoNote->ExecuteLD( *prompt );
   
    // Reset the pointer
    iInfoNote = NULL;
           
    CleanupStack::PopAndDestroy( prompt );

    // Do a deferred draw in order to refresh the highlight
    DrawDeferred();
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::DrawPanInd
//  Draws the Panning indicators for the SVGT content.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::DrawPanInd() const
    {
    // Get current Control Rectangle
    TRect myRect = Rect();
    // Set the Brush Style and Pen Style to NULL type
    // so that the background of bitmap is correctly blitted.
    iSVGTFrameBufContext->SetPenStyle( CGraphicsContext::ENullPen );
    iSVGTFrameBufContext->SetBrushStyle( CGraphicsContext::ENullBrush );

    if ( iSvgModule )
        {
        // Pan Left?
        if ( iPanLt )
            {
            // Draw the indicator                   
            iSVGTFrameBufContext->BitBltMasked( iPanIndLeftRect.Rect().iTl,
                iPanIndLeftIcon->Bitmap(),
                myRect,
                iPanIndLeftIcon->Mask(),
                EFalse );                              
                   
            }

        // Pan Right?
        if ( iPanRt )
            {
            // Draw the indicator
            iSVGTFrameBufContext->BitBltMasked( iPanIndRightRect.Rect().iTl,
                iPanIndRightIcon->Bitmap(),
                myRect,
                iPanIndRightIcon->Mask(),
                EFalse );                              
            }

        // Pan Up?
        if ( iPanUp )
            {
            // Draw the indicator
            iSVGTFrameBufContext->BitBltMasked( iPanIndUpRect.Rect().iTl,
                iPanIndUpIcon->Bitmap(),
                myRect,
                iPanIndUpIcon->Mask(),
                EFalse );                              
            }

        // Pan Down?
        if ( iPanDn )
            {
            // Draw the indicator
            iSVGTFrameBufContext->BitBltMasked( iPanIndDownRect.Rect().iTl,
                iPanIndDownIcon->Bitmap(),
                myRect,
                iPanIndDownIcon->Mask(),
                EFalse );
            }
        }
   }

// -----------------------------------------------------------------------------
// CSVGTCustControl::CountAnimStarted
//  Counts the number of animations started in the SVGT content.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::CountAnimStarted()
    {
    iNumAnimInProgress++;
#ifdef _DEBUG
    iDbgNumAnimStarted++;
#endif       
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::CountAnimPaused
//  Counts the number of animations paused in the SVGT content.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::CountAnimPaused()
    {
    // Pause is for the whole content, rather than per animation.
    // Do not reset the count of animations in progress. Once the
    // animation is resumed it would receive as many AnimStopped()(as in
    // iNumAnimInProgress) as the animation progresses towards completion.
#ifdef _DEBUG
    iDbgNumAnimPaused++;
#endif       
    }
   
// -----------------------------------------------------------------------------
// CSVGTCustControl::CountAnimStopped
//  Counts the number of animations stopped in the SVGT content.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::CountAnimStopped()
    {
    iNumAnimInProgress--;
#ifdef _DEBUG
    iDbgNumAnimStopped++;
#endif           
    }   

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsAnimFirst
//  Function indicates whether the animation currently in progress is first one.
// @return TBool - ETrue - First one
//                 EFalse Otherwise.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::IsAnimFirst() const
    {
    return( iNumAnimInProgress == 0 );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::IsAnimLast
//  Function indicates whether the animation currently in progress is the last
//  one.
// @return TBool - ETrue - Last one
//                 EFalse Otherwise.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::IsAnimLast() const
    {
    return( iNumAnimInProgress == 0 );
    }   


// -----------------------------------------------------------------------------
// CSVGTCustControl::SetPointerDisplayStatusAndRedraw
// Function sets the Pointer display status and redraws the whole control
// inorder to update display status. Called when Pointer Hide Timer times out.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::SetPointerDisplayStatusAndRedraw(
    TBool aIsPointerDisplayed )
    {
    if ( iIsPointerDisplayed != aIsPointerDisplayed )
        {
        iIsPointerDisplayed = aIsPointerDisplayed;   
        //Set the MSK label
        if(iMSKImpl &&  iScrollKey && !iPreviewMode )
        	{
        	iScrollKey=EFalse;
        	TRAP_IGNORE(SetMSKLabelL());
        	}
        DrawDeferred();
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ShowPauseIcon()
// This function updates the pause indicator display in the main pane
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ShowPauseIcon(
    TBool aPauseIconVisible ) // ETrue - Start Pause Blink
    {
    if ( iDisplayFlags & ESvgDrawPauseIndicator )
        {       
        if (aPauseIconVisible && !iBlinker->IsActive())
            {
            iBlinker->Start( KSccOneSecond/2, KSccOneSecond/2,
                    TCallBack( BlinkPauseIcon, this ) );
            }
        if (!aPauseIconVisible && iBlinker->IsActive())
            {
            iBlinker->Cancel();
            }
        iPauseIconVisible = aPauseIconVisible;
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::BlinkPauseIcon
// Callback function of blink timer
// -----------------------------------------------------------------------------
//
TInt CSVGTCustControl::BlinkPauseIcon( TAny* aPtr )
    {
    if ( aPtr )
        {
        static_cast< CSVGTCustControl* >( aPtr )->DoBlinkPauseIcon();
        }
    return KErrNone;       
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::DoBlinkPauseIcon
// Function which does the actual work of toggling pause indicator
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::DoBlinkPauseIcon()
    {
    iPauseIconVisible = !iPauseIconVisible;
    DrawDeferred();
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::FreeIcons
// Function which frees the icons created by CreateIcons
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::FreeIcons()
    {
    // Pointer Icons - owned
    delete iPointerIcon;
    iPointerIcon = NULL;
   
    delete iHandIcon;
    iHandIcon = NULL;
   
    delete iTextIcon;
    iTextIcon = NULL;
   
    // Panning Indicator Icons - owned
    delete iPanIndLeftIcon;
    iPanIndLeftIcon = NULL;
   
    delete iPanIndRightIcon;
    iPanIndRightIcon = NULL;
   
    delete iPanIndUpIcon;
    iPanIndUpIcon = NULL;
   
    delete iPanIndDownIcon;
    iPanIndDownIcon = NULL;
   
    // Pause Indicator Icons - owned
    delete iPauseIcon;
    iPauseIcon = NULL;
    }

// ----------------------------------------------------------------------------
// CSVGTCustControl::LaunchWaitNoteL
// Helper function for launching wait note.
// ----------------------------------------------------------------------------
//
void CSVGTCustControl::LaunchWaitNoteL(
    TInt aResourceId ) // Resource ID indicating
                       // string to be displayed
                       // while waiting.
    {
    if ( iDisplayFlags & ESvgDisplayWaitNote )
        {
        if( iWaitDialog )
            {
            delete iWaitDialog;
            iWaitDialog = NULL;
            }

        iWaitDialog = new( ELeave ) CAknWaitDialog(
                                    reinterpret_cast<CEikDialog**>( 
                                    &iWaitDialog ),
                                    ETrue /* No visibility delay =
                                             Display Wait note immediately */ );

        iWaitDialog->PrepareLC( R_SVGT_WAIT_NOTE );

        HBufC* lNoteTextPtr = StringLoader::LoadLC( aResourceId );
        iWaitDialog->SetTextL( *lNoteTextPtr );
        CleanupStack::PopAndDestroy( lNoteTextPtr );

        // Launch dialog
        iWaitDialog->RunLD();
        }
    }

// ----------------------------------------------------------------------------
// CSVGTCustControl::DismissWaitNote
// Helper function for dismissing wait note.
// ----------------------------------------------------------------------------
//
void CSVGTCustControl::DismissWaitNote()
    {
    if ( iDisplayFlags & ESvgDisplayWaitNote )
        {
        if( iWaitDialog )
            {
            TRAPD( err, iWaitDialog->ProcessFinishedL() );
            if( err )
                {
                delete iWaitDialog;
                iWaitDialog = NULL;
                }
            }
        }
    }

// ----------------------------------------------------------------------------
// CSVGTCustControl::StartProgIndTimer
// Helper function for starting the progress indicator timer.
// ----------------------------------------------------------------------------
//
void CSVGTCustControl::StartProgIndTimer( TInt aAfter ) // Time period in
                                                        // micro-seconds after
                                                        // after which timer
                                                        // expires
    {
    // Stop the progress indicator timer if exists
    StopProgIndTimer();
       
    TRAPD(err, iProgressBarTimer =
                   CSVGTProgressBarTimer::NewL( this,
                   iDisplayFlags & ESvgUseBackLight ) );           
   
    if ( err == KErrNone )
        {
        // Expire after aAfter microseconds.
        iProgressBarTimer->After( aAfter );
        }       
    }                                            

// ----------------------------------------------------------------------------
// CSVGTCustControl::StopProgIndTimer
// Helper function for stopping the progress indicator timer.
// ----------------------------------------------------------------------------
//
void CSVGTCustControl::StopProgIndTimer()
    {
    // Delete the Progress Bar timer
    if ( iProgressBarTimer )
        {
        delete iProgressBarTimer;
        iProgressBarTimer = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ProcessPlayCmd
// This function is called to perform the necessary processing for the Play
// command.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ProcessPlayCmd()
    {
    // Play Command is applicable only if animations are present
    if ( IsAnimationPresent() )
        {
        if ( iAnimState != ESvgAnimPlayingState )
            {
            // Start the backlight timer if the content is of finite duration
            if ( IsContentFinite() )
                {
                if ( ( iDisplayFlags & ESvgUseBackLight ) )
                    {
#ifdef _DEBUG                    
                    iEikonEnv->InfoMsg( KSccBacklightOnStr );   
#endif 
/*
                    TRAPD( err, iBacklightTimer = CSVGTBacklightTimer::NewL() );
                    if ( err == KErrNone )
                        {
                        iBacklightTimer->After( KSccBacklightTimerStartAfter ); 
                        }           
*/                       
                    }
                }

            // Start the Progress Ind Timer if the content is of finite duration
            if ( IsContentFinite() )
                {
                // Start immediately so that current count is updated.
                StartProgIndTimer( 0 );
                }
           
            if ( iAnimState == ESvgAnimStoppedState )
                {
                // Rewind the content
                iSvgModule->Replay();
                }
            else
                {
                // Resume playback of the content
                iSvgModule->Resume();
                }
            // Set the state variable to indicate content is playing        
            iAnimState = ESvgAnimPlayingState;
           
            // Hide the display of Pause content
            ShowPauseIcon( EFalse );
            }
        }
    }
   
// -----------------------------------------------------------------------------
// CSVGTCustControl::ProcessPauseCmd
// This function is called to perform the necessary processing for the Pause
// command.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ProcessPauseCmd()
    {
    // Pause Command is applicable only if animations are present
    if ( IsAnimationPresent() )
        {
        if ( iAnimState == ESvgAnimPlayingState )
            {
            if ( IsContentFinite() )
                {
                // Stop the progress Bar timer
                StopProgIndTimer();
                }
            // Set the state variable.
            iAnimState = ESvgAnimPausedState;
            // Pause the content
            iSvgModule->Pause();
            // Show the pause indicator
            ShowPauseIcon( ETrue );
            }
        }
    }
   
// -----------------------------------------------------------------------------
// CSVGTCustControl::ProcessStopCmd
// This function is called to perform the necessary processing for the Stop
// command.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ProcessStopCmd()
    {
    // Stop Command is applicable only if animations are present
    if ( IsAnimationPresent() )
        {
        if ( IsContentFinite() )
            {
            // Stop the progress Bar timer
            StopProgIndTimer();
            }
        // Stop playing the content
        // Do a pause so that the content is not rewound.
        iSvgModule->Stop();
       
        // Reset the state variables
        iAnimState = ESvgAnimStoppedState;
        iProgIndTimeElap = 0;
       
        // Reset the number of animations in progress.
        iNumAnimInProgress = 0;
       
        // Stop the display of the Pause indicator
        ShowPauseIcon( EFalse );
       
        // Redraw the screen - so that pause icon is cleaned up
        DrawDeferred();
        }
    }       
// -----------------------------------------------------------------------------
// CSVGTCustControl::IsVoiceCallActive 
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::IsVoiceCallActive()
    {
    TBool call;
    TInt type = 0; 
    // Check whether a call is a voice call
    TInt error = iSystemState.Get( KPSUidCtsyCallInformation,
                                   KCTsyCallType, type );
    if ( !error && type == EPSCTsyCallTypeCSVoice )
        {
        call = ETrue;
        }
    else
        {
        call = EFalse;
        }
         
    TInt state = 0;
    // Check whether the voice call is active
    error = iSystemState.Get( KPSUidCtsyCallInformation, 
             KCTsyCallState, state );
    if ( call && !error && state == EPSCTsyCallStateConnected )
        {
        return ETrue;
        }
    else
        {       
        return EFalse;
        }   
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::RemoveInfoNote
// This function is called by the periodic timer to remove the info note that
// is not removed because of starvation.
// -----------------------------------------------------------------------------
//
TInt CSVGTCustControl::RemoveInfoNote( TAny* aPtr )
    {
    CSVGTCustControl* thisPtr = static_cast< CSVGTCustControl* >( aPtr );
    if ( thisPtr && thisPtr->iInfoNote )
        {
        delete thisPtr->iInfoNote;
        thisPtr->iInfoNote = NULL;
        }
    if ( thisPtr )       
        {
        if ( thisPtr->iNoteRemoverTimer->IsActive() )
            {
            thisPtr->iNoteRemoverTimer->Cancel();
            }
        }
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::DoRotation()
// This function rotates the presentation in clockwise direction with given
// angle about bitmap center
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::DoRotation( const TInt aAngle )
    {
    const TInt xCenter = iSVGTBitMap->SizeInPixels().iWidth / 2; 
    const TInt yCenter = iSVGTBitMap->SizeInPixels().iHeight / 2;
    const TReal32 angleInRadians = KDegreesToRadiansFactor * TReal32( aAngle );
    iSvgModule->Rotate( angleInRadians, xCenter, yCenter );
    // Do a Redraw
    iSvgModule->Redraw();
    UpdatePointer();
    }

#ifdef SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE   
// -----------------------------------------------------------------------------
// CSVGTCustControl::DoTextSelectionL()
// This function displays the existing text in the text/textArea in a
// read-only dialog so that the user can copy text into system clipboard
// from it.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::DoTextSelectionL(
    TDes& aText // Text Content descriptor
    ) const
    {
    CAknTextQueryDialog* dlgPtr = CAknTextQueryDialog::NewL( aText );
    dlgPtr->ExecuteLD( R_SVGT_TEXT_SELECTION_DIALOG );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::DoTextEditingL()
// This function displays the existing text in the text/textArea element which
// is editable and returns the modified text in the same descriptor.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::DoTextEditingL(
    TBool aIsTextElement, // Bool indicating whether element is text/textArea
    TInt aElemHandle, // Element handle from engine
    TDes& aText ) // Text Content descriptor
    {
    const TChar KOpeningBraces ='<';
    const TChar KClosingBraces ='>';
    TBool lTextQueryDone = EFalse;

    while ( !lTextQueryDone )
        {
        CAknTextQueryDialog* dlgPtr = CAknTextQueryDialog::NewL( aText );
        dlgPtr->SetPredictiveTextInputPermitted( ETrue );
        TInt retVal = dlgPtr->ExecuteLD( R_SVGT_EDIT_TEXT_QUERY_DIALOG );
        if (retVal)
            {
            if ( !iIsTextChanged )
                {
                // Set iIsTextChanged to indicate that text content is
                // changed and changes need to be saved
                iIsTextChanged = ETrue;
                }
           
            if( aText.Locate( KOpeningBraces ) != KErrNotFound ||
                    aText.Locate( KClosingBraces ) != KErrNotFound )
                {
                // display a error not with the msg that the text area
                // cant contain < or > since it will fuddle up the xml parsing
                DisplayInfoNoteL(R_SVGT_BRACES_FOUND_IN_TEXTAREA, ETrue );
                }
                else
                    {
                    // User entered valid text, go out of loop
                    lTextQueryDone = ETrue;
                    }
            }
            else
                {
                // User pressed cancel, no need to change the text
                // return back to caller
                return;
                }
        }

    if ( aIsTextElement )
        {
        // Update the text element in the engine
        iSvgModule->SetTextForTextElement( aElemHandle, aText );
        }
        else
            {
            // Update the textArea element in the engine
            iSvgModule->SetTextForTextAreaElement( aElemHandle, aText );
            }
    // Redraw the screen to reflect the new changes
    iSvgModule->Redraw();
           

    }
#endif // SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE 

// -----------------------------------------------------------------------------
// CSVGTCustControl::FPRCenter()
// This function calculates the center of the four point rectangle
// -----------------------------------------------------------------------------
//
TPoint CSVGTCustControl::FPRCenter(
    const MRect* aBBoxRect    // Four point rect
    ) const
    {
    TPoint pt1( 0, 0 ), pt2( 0, 0 ), pt3( 0, 0 ), pt4( 0, 0 );
    TPoint lCentre;

    if ( aBBoxRect )
        {
        aBBoxRect->GetPoints( pt1, pt2, pt3, pt4 );
        // Center of the rectangle is the center of the diagonal as
        // diagonals are congruent in a rectangle
        lCentre = TPoint( ( pt1.iX + pt3.iX ) / 2, ( pt1.iY + pt3.iY )/2 );
        }
    return lCentre;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::FPRIntersects()
// This function calculates whether the FPR intersects with the rectangle
// -----------------------------------------------------------------------------
TBool CSVGTCustControl::FPRIntersects(
    const MRect* aBBoxRect,   // Four point rect
    const TRect& aRect        // Rectangle
    ) const
    {
    TPoint pt1( 0, 0 ), pt2( 0, 0 ), pt3( 0, 0 ), pt4( 0, 0 );
    if ( aBBoxRect )
        {
        aBBoxRect->GetPoints( pt1, pt2, pt3, pt4 );   
        // TODO: Need to use clipping algorithm here - to check whether line is
        // within rectangle.
        TInt32 maxx,maxy,minx,miny;
        
        maxx = pt1.iX;
        if(pt2.iX > maxx)
        	maxx = pt2.iX;
        if(pt3.iX > maxx)
        	maxx = pt3.iX;
        if(pt4.iX > maxx)
        	maxx = pt4.iX;
        
        maxy = pt1.iY;
        if(pt2.iY > maxy)
        	maxy = pt2.iY;
        if(pt3.iY > maxy)
        	maxy = pt3.iY;
        if(pt4.iY > maxy)
        	maxy = pt4.iY;
        
        minx = pt1.iX;
        if(pt2.iX < minx)
        	minx = pt2.iX;
        if(pt3.iX < minx)
        	minx = pt3.iX;
        if(pt4.iX < minx)
        	minx = pt4.iX;
        
        miny = pt1.iY;
        if(pt2.iY < miny)
        	miny = pt2.iY;
        if(pt3.iY < miny)
        	miny = pt3.iY;
        if(pt4.iY < miny)
        	miny = pt4.iY;
        

        TPoint topleft(aRect.iTl);
        TPoint bottomright(aRect.iBr);

	    if ( ( bottomright.iX < minx ) || ( maxx < topleft.iX ) ||
         	 ( bottomright.iY  < miny) || ( maxy < topleft.iY ) )
        {
        return EFalse;
        }
        
        return ETrue;
        }
    return EFalse;   
    }
// -----------------------------------------------------------------------------
// CSVGTCustControl::FPR2PointArrayL()
// This function converts a Four Point Rect into an array of points
// -----------------------------------------------------------------------------
void CSVGTCustControl::FPR2PointArrayL(
    const MRect* aBBoxRect,         // Four Point Rect
    CArrayFix<TPoint>*& aArrayPtr   // Array of Points
    )  const
    {
    aArrayPtr = new (ELeave) CArrayFixFlat<TPoint> ( 4 );
    CleanupStack::PushL( aArrayPtr );
    TPoint pt1( 0, 0 ), pt2( 0, 0 ), pt3( 0, 0 ), pt4( 0, 0 );
   
    if ( aBBoxRect )
        {
        aBBoxRect->GetPoints( pt1, pt2, pt3, pt4 );
        }
    aArrayPtr->AppendL( pt1 );
    aArrayPtr->AppendL( pt2 );
    aArrayPtr->AppendL( pt4 );
    aArrayPtr->AppendL( pt3 );   
   
    CleanupStack::Pop( aArrayPtr ); // Ownership transferred to caller
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::Draw
// Reimplements CCoeControl::Draw to draw this control's view to the screen
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::Draw(
    const TRect& aRect ) const
    {
    iSVGTFrameBufContext->Clear();
   
    if ( iSVGTBitMap && iSVGTBitMapMask )
        {
        iSVGTFrameBufContext->BitBltMasked( iViewerPaneRect.Rect().iTl,
                                            iSVGTBitMap,
                                            iViewerPaneRect.Rect(),
                                            iSVGTBitMapMask,
                                            ETrue );
        }
   
    // Draw the bounding box
    DrawFindTextBBox( aRect );

    if ( iDisplayFlags & ESvgDrawPanIndicator )
        {
        // Draw Panning Indicator
        DrawPanInd();
        }
   
    // draw paused icon
    if ( iPauseIconVisible )
        {
        iSVGTFrameBufContext->BitBltMasked( iPauseRect.Rect().iTl,
            iPauseIcon->Bitmap(),
            Rect(),
            iPauseIcon->Mask(),
            EFalse ); 
        }

    
    // Draw Pointer
    if ( iIsPointerDisplayed && iDisplayFlags & ESvgDrawPointer )
        {
        DrawPointer();   
        }

    // Get the standard graphics context
    CWindowGc& gc = SystemGc();
    // Blit our framebuffer bitmap to the System GC   
    gc.BitBlt( iViewerPaneRect.Rect().iTl, iSVGTFrameBufBitMap );
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ConstructFromResourceL
// Since this control is to be displayed in a dialog, this function is
// reimplented from CCoeControl::ConstructFromResourceL inorder to read
// resource values from resource file.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ConstructFromResourceL(
    TResourceReader& /*aReader*/) // Resource Reader
    {
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::SizeChanged
// Handles resizing of control reimplementing CCoeControl::SizeChanged.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::SizeChanged()
    {
    TRect myRect = Rect();
   
   // Commented this section because,Indicators were not displaying in
   // case of theme change
   // if ( myRect == iCurrentLayout )
   //    {
   // Layout has not changed, do nothing
   // return;
   // } 
   // 
       
    if ( AknLayoutUtils::ScalableLayoutInterfaceAvailable() )
        {
        TInt lVariety = 0; // Normal Screen
        if ( iIsFullScreenOn )
            {
            lVariety = 1; // Full Screen
            }
       
        // Main Viewer Pane
        iViewerPaneRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane( lVariety ) );
       
        // Panning Indicators
        iPanIndLeftRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane_g3( lVariety ) );
        iPanIndRightRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane_g4( lVariety ) );
        iPanIndUpRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane_g1( lVariety ) );
        iPanIndDownRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane_g2( lVariety ) );
      // Panning Indicators Rectangle for Touch Area
      	iPanIndLeftAidRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::aid_main_viewer_pane_g3( lVariety ) );
        iPanIndRightAidRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::aid_main_viewer_pane_g4( lVariety ) );
        iPanIndUpAidRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::aid_main_viewer_pane_g1( lVariety ) );
        iPanIndDownAidRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::aid_main_viewer_pane_g2( lVariety ) );
       
        // Pointers
        iPointerRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane_g5( lVariety ) );
        iHandRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane_g7( lVariety ) );
        iTextRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane_g7( lVariety ) );
       
        // Pause Indicator
        iPauseRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::main_viewer_pane_g8( lVariety ) );

        // Aid for display of pointer
        // In order to draw the cursor in the correct location on
        // the screen, the viewer needs to know the offsetX and offSetY
        // of the cursor hotspot in the icon bitmap.
        // Because of Scalable UI the size of the icons can be changed
        // and this would result in the change of these co-ordinates.
        // he viewer needs to get this information from the LAF
        // so that viewer can draw the cursor in the correct position
        // on the screen.
       
        iPointerAidRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::aid_hotspot_pointer_arrow_pane() );
        iHandAidRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::aid_hotspot_pointer_hand_pane() );
        iTextAidRect.LayoutRect( myRect,
            AknLayoutScalable_Apps::aid_hotspot_pointer_text2_pane() );
        }
        else
            {
            // Hardcode the values for normal resolution (172x208)
           
            iViewerPaneRect.LayoutRect( myRect,
                0, 0, 0, 0, 0, myRect.Width(), myRect.Height() );
               
            iPanIndLeftRect.LayoutRect( myRect,
                    0, 0, ( myRect.Height() / KSccConstTwo ) - 8, 0, 0, 6, 15 );
            iPanIndRightRect.LayoutRect( myRect,
                    0, ( myRect.Width() - 6 ),
                    (myRect.Height() / KSccConstTwo ) - 8, 0, 0, 6, 15 );
            iPanIndUpRect.LayoutRect( myRect,
                    0, ( ( myRect.Width() / KSccConstTwo ) - 8 ),  0, 0, 0,
                    15, 6 );
            iPanIndDownRect.LayoutRect( myRect,
                    0, ( ( myRect.Width() / KSccConstTwo ) - 8 ), 
                    myRect.Height() - 6, 0, 0, 15, 6 );

            iPointerRect.LayoutRect( myRect,
                    0, 0, 0, 0, 0, 18, 24 );
            iHandRect.LayoutRect( myRect,
                    0, 0, 0, 0, 0, 18, 24 );
            iTextRect.LayoutRect( myRect,
                    0, 0, 0, 0, 0, 18, 24 );

           
            iPauseRect.LayoutRect(myRect,
                    0, 0, 0, 0, 0, 13, 13 );
           
            iPointerAidRect.LayoutRect( myRect,
                    0, 0, 0, 0, 0, 0, 0 );
            iHandAidRect.LayoutRect( myRect,
                    0, 0, 0, 0, 0, 0, 0 );                   
            iTextAidRect.LayoutRect( myRect,
                    0, 0, 0, 0, 0, 0, 0 );                   
            }
   
    // Set the size of the bitmaps
    // No Error Handling for SetSize
    // Pointers
    AknIconUtils::SetSize( iPointerIcon->Bitmap(), iPointerRect.Rect().Size() );
    AknIconUtils::SetSize( iHandIcon->Bitmap(), iHandRect.Rect().Size() );
    AknIconUtils::SetSize( iTextIcon->Bitmap(), iTextRect.Rect().Size() );

    // Panning Indicators

    // Left Ind
    AknIconUtils::SetSize( iPanIndLeftIcon->Bitmap(),
            iPanIndLeftRect.Rect().Size() );

    // Right Ind
    AknIconUtils::SetSize( iPanIndRightIcon->Bitmap(),
            iPanIndRightRect.Rect().Size() );
   
    // Up Ind
    AknIconUtils::SetSize( iPanIndUpIcon->Bitmap(),
            iPanIndUpRect.Rect().Size() );

    // Down Ind
    AknIconUtils::SetSize( iPanIndDownIcon->Bitmap(),
            iPanIndDownRect.Rect().Size() );

    // Pause Ind
    AknIconUtils::SetSize( iPauseIcon->Bitmap(),
            iPauseRect.Rect().Size() );

    // Recreate the framebuffer.
    TRAPD( errSwitchMode, SwitchScreenModeL() );
    if ( errSwitchMode != KErrNone )
        {
        // Let environment handle error
        iCoeEnv->HandleError( errSwitchMode );
        }
   
    // Dummy mouse-move to the engine
    iSvgModule->MouseMove( iPointerX, iPointerY );                   
   
    // Save current layout so that we know if the Layout changed.
    iCurrentLayout = myRect;
    }


// -----------------------------------------------------------------------------
// CSVGTCustControl::AnimationStarted
// Handles callback notification from SVG engine indicating Animation Started.
// Reimplements MSvgAnimationListener::AnimationStarted.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::AnimationStarted( TBool aIsAnimationIndefinite )
    {
    // If the content contains any infinite animations - then the
    // backlight behaves normally.
    if ( !IsContentFinite() )
        {
        return ETrue;
        }
       
    if ( aIsAnimationIndefinite )
        {
#ifdef _DEBUG
        iEikonEnv->InfoMsg( KSccBacklightOffStr );   
#endif 
       
        }
        else
            {
            if ( IsAnimFirst() )
                {               
#ifdef _DEBUG                    
                iEikonEnv->InfoMsg( KSccBacklightOnStr );   
#endif 
                if ( ( iDisplayFlags & ESvgUseBackLight ) )
                    {
                        
                    }
                }
            CountAnimStarted();
            }
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::AnimationEnded
// Handles callback notification from SVG engine indicating Animation Ended.
// Reimplements MSvgAnimationListener::AnimationEnded.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::AnimationEnded()
    {
    // If the content contains any infinite animations - then the
    // backlight behaves normally.
    if ( !IsContentFinite() )
        {
        return ETrue;
        }
   
    CountAnimStopped();
    if ( IsAnimLast() )
        {
#ifdef _DEBUG
        iEikonEnv->InfoMsg( KSccBacklightOffStr );   
#endif 
        // All animations have ended
       
        }
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::AnimationPaused
// Handles callback notification from SVG engine indicating Animation Paused.
// Reimplements MSvgAnimationListener::AnimationPaused.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::AnimationPaused()
    {
    // If the content contains any infinite animations - then the
    // backlight behaves normally.
    if ( !IsContentFinite() )
        {
        return ETrue;
        }
   
#ifdef _DEBUG
    iEikonEnv->InfoMsg( KSccBacklightOffStr );   
#endif 
    // Animation is paused,
    CountAnimPaused();

    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::UpdateProgressBar
// This function updates the progress bar in the navi pane. It is called back
// by the progress bar timer.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::UpdateProgressBar()
    {
    
    TReal lSecElapsedRounded = 0.0;
    TReal lSecElapsedReal    = 0.0;
    TReal lTotalTimeReal     = 0.0;
    TReal lTotalTimeRounded  = 0.0;
    TUint32 lTimeElapsed     = 0;
        
    lSecElapsedReal = (TReal)iSvgModule->MediaTime() 
            / KSccNumMSecPerSec;
            
    lTotalTimeReal = (TReal)iDuration / KSccNumMSecPerSec;      
        if ( iProgressBarDrawImpl )
            {
            
        
           // Round off the Elapsed time inorder to display time accurately.
           Math::Round( lSecElapsedRounded, lSecElapsedReal, 0 );
        
           // Round off the Total time inorder to display time accurately.
           Math::Round( lTotalTimeRounded, lTotalTimeReal, 0 );

           lTimeElapsed = lSecElapsedRounded;
           TRAPD( err, iProgressBarDrawImpl->DrawProgressBarL( lTimeElapsed, 
           lTotalTimeRounded ) );
           if ( err != KErrNone )
              {
              // Error Case, Nothing is done now.
               }    
            
      
           // Stop the Progress Bar timer
           StopProgIndTimer();
           iProgIndTimeElap = lSecElapsedReal ;
   
            }
            
        // Check whether end of animation is reached.
        // 01-Feb-2008 Fix for TSW Id "CGSR-7B6DFW"
        // Earlier this check was happening only if iProgressBarDrawImpl is there
        // for screen saver iProgressBarDrawImpl is not there but still the animation
        // should work for the duration set for ScreenSaver
        
     if ( lSecElapsedReal >= (TReal)iDuration / KSccNumMSecPerSec )
        {
        TInt err = 0;
        // Reset the state variables
        iProgIndTimeElap = 0;
        
        // Stop the animation and rewind.
        TRAP(err, ProcessViewerCommandL( ESvgtDlgStopCommand ) );
        if ( err != KErrNone )
            {
            // Error Case, Nothing is done now.               
            }
        // If loop is on then replay the content.
        if ( !iIsLoopOn )
            {
            // Animation is done, no need to continue progress bar.
            return;
            }
            else
                {
                // Loop option enabled. Replay content.
                TRAP( err, ProcessViewerCommandL( ESvgtDlgPlayCommand ));
                if ( err != KErrNone )
                    {
                    // Error Case, Nothing is done now.
                    }
                return;
                }
        }        
            
            
            
            
    // Restart the timer
    StartProgIndTimer( KSccProgIndTimerYieldDur ); 
    }   



// -----------------------------------------------------------------------------
// CSVGTCustControl::PointerInactivityTimeout()
// This function updates the pointer display status in the navi pane.
// It is called back by the pointer hide timer.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::PointerInactivityTimeout()
    {
    // UI Spec: Hide Pointer as time of inactivity has passed
    SetPointerDisplayStatusAndRedraw( EFalse );
   
    //Set the MSK label
    if(iMSKImpl && !iPreviewMode )
    	{
    	TRAP_IGNORE(SetMSKLabelL());
    	}
   		
    // Delete the inactivity timer
    delete iInactivityTimer;
    iInactivityTimer = NULL;
    }

#ifdef SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE
// -----------------------------------------------------------------------------
// CSVGTCustControl::TextAreaEntered()
// This callback is called to notify when a pointer enters a textbox element.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::TextAreaEntered( TInt aTextAreaHandle )
    {
    // Save the current element handle
    iCurrentTextElementId = aTextAreaHandle;
   
    // Indicate that it is a text area element
    iIsCurrentTextElement = EFalse;
   
    // check if there is a 'a' element or interactive element
    // within which text is present
    
    if(iAElement || iPointerDownAnimation)
    	{
    	iPointerType=ESvgPointerHand;	
    	}
    else
    	{
    	// Text symbol to indicate user can perform some action by clicking
    	iPointerType = ESvgPointerText;
    	}
    // Remember that textArea element is entered, so that
    // subsequent text element notifications are ignored
    iTextAreaEntered = ETrue;
   
    // Enable Select Menu Option
    iIsTextSelectOn = ETrue;
    HBufC* lStrPtr = HBufC::New( KSccMaxTextLength );
    if ( !lStrPtr )
        {
        // Error while allocation, no memory
        iCoeEnv->HandleError( KErrNoMemory );
        return ETrue;
        }
    TPtr lPtr = lStrPtr->Des();
   
    // Check whether the Text is editable
    iSvgModule->GetTextForTextAreaElement( aTextAreaHandle,
        iIsTextEditOn,
        lPtr );   

    delete lStrPtr;
   
    //Display MSK label Select/Edit
    if(iMSKImpl && !iPreviewMode)
    	{
    	TRAP_IGNORE(SetMSKLabelL());	
    	}
   	
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::TextAreaExited()
// This callback is called to notify when a pointer exits a textbox element.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::TextAreaExited( TInt /* aTextAreaHandle */)
    {
    // Reset the current element handle
    iCurrentTextElementId = 0;
   
    // Reset the element flag to text element
    iIsCurrentTextElement = ETrue;
   
    // check if there is a 'a' element or interactive element
    // within which text is present
    if(iAElement || iPointerDownAnimation)
    	{
    	iPointerType=ESvgPointerHand;	
    	}
    else
    	{
    	// Reset the pointer to default pointer
    	iPointerType = ESvgPointerDefault;   
    	}
    // Remember that textArea element is exitted
    iTextAreaEntered = EFalse;

    // Disable the Select and Edit menu option
    iIsTextSelectOn = EFalse;
    iIsTextEditOn = EFalse;
    //Set the MSK label
    if(iMSKImpl && !iPreviewMode)
    	{
    	TRAP_IGNORE(SetMSKLabelL());	
    	}
   
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::TextAreaActivated()
// This callback is called to notify that a text element has been activated.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::TextAreaActivated( TInt aTextAreaHandle )
    {
    if (iSelectCause != ESvgInvalidSelectCause )
        {
        // Ignore this callback as it was already processed.
        return ETrue;
        }
    // Set the cause for select operation as text area so that
    // subsequent events trigerred are ignored.
    iSelectCause = ESvgTextAreaSelectCause;

    HBufC* lStrPtr = HBufC::New( KMaxEditorTextLength );         
    if ( !lStrPtr )
        {
        // Error while allocation, no memory
        iCoeEnv->HandleError( KErrNoMemory );
        return ETrue;
        }
    TPtr lPtr = lStrPtr->Des();
   
    // Check whether the Text is editable
    iSvgModule->GetTextForTextAreaElement( aTextAreaHandle,
        iIsTextEditOn,
        lPtr );
       
    if ( iIsTextEditOn && !iSvgModule->IsLoading() )
        {
        TRAPD( err, DoTextEditingL(
                        EFalse,              // Text Area Element
                        aTextAreaHandle,     // Element Handle
                        lPtr ) );            // buffer pointer
        if ( err != KErrNone )
            {
            // No error handling done.
            }
                       
        }
        else
            {
            TRAPD( err,
                DoTextSelectionL( lPtr );
                );
            if ( err != KErrNone )
                {
                // No error handling done.
                }
            }
   
    delete lStrPtr;       
    return ETrue;
    }
   
// -----------------------------------------------------------------------------
// CSVGTCustControl::TextEntered()
// This callback is called to notify that a text element has been entered.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::TextEntered( TInt aTextHandle )
    {
    if ( iTextAreaEntered )
        {
        // Ignore all text element notifications when cursor is
        // within a textArea element
        return ETrue;
        }
       
    // Save the current element handle
    iCurrentTextElementId = aTextHandle;
   
    // Set the flag to indicate element type is text element
    iIsCurrentTextElement = ETrue;
   
    // check if there is a 'a' element or interactive element
    // within which text is present
    
    if(iAElement || iPointerDownAnimation)
    	{
    	iPointerType=ESvgPointerHand;	
    	}
    else
    	{
    	// Text symbol to indicate user can perform some action by clicking
    	iPointerType = ESvgPointerText;
    	}
    // Enable Select Menu Option
    iIsTextSelectOn = ETrue;

    HBufC* lStrPtr = HBufC::New( KSccMaxTextLength );
   
    if ( !lStrPtr )
        {
        // Error while allocation, no memory
        iCoeEnv->HandleError( KErrNoMemory );
        return ETrue;
        }
   
    TPtr lPtr = lStrPtr->Des();
   
    // Check whether the Text is editable
    iSvgModule->GetTextForTextElement( aTextHandle,
        iIsTextEditOn,
        lPtr );
   
    delete lStrPtr;
     // Display MSK label Select/Edit
    if(iMSKImpl && !iPreviewMode)
    	{
    	TRAP_IGNORE(SetMSKLabelL());	
    	}
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::TextExited()
// This callback is called to notify that a text element has been exited.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::TextExited( TInt /* aTextHandle */ )
    {
    if ( iTextAreaEntered )
        {
        // Ignore all text element notifications when cursor is
        // within a textArea element
        return ETrue;
        }
           
    // Reset the current element handle
    iCurrentTextElementId = 0;
    
    // check if there is a 'a' element or interactive element
    // within which text is present
    if(iAElement || iPointerDownAnimation)
    	{
    	iPointerType=ESvgPointerHand;	
    	}
    else
    	{
    	// Reset the pointer to default
    	iPointerType = ESvgPointerDefault;
    	}
    // Disable the Select and Edit menu option
    iIsTextSelectOn = EFalse;
    iIsTextEditOn = EFalse;
   
    //Set the MSK label
    if(iMSKImpl && !iPreviewMode)
    	{
    	TRAP_IGNORE(SetMSKLabelL());	
    	}
       	
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::TextActivated()
// This callback is called to notify that a text element has been activated.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::TextActivated( TInt aTextHandle )
    {
    if (iSelectCause != ESvgInvalidSelectCause )
        {
        // Ignore this callback as it was already processed.
        return ETrue;
        }
    // Set the cause for select operation as text so that
    // subsequent events trigerred are ignored.
    iSelectCause = ESvgTextSelectCause;
    HBufC* lStrPtr = HBufC::New( KMaxEditorTextLength );         
    if ( !lStrPtr )
        {
        // Error while allocation, no memory
        iCoeEnv->HandleError( KErrNoMemory );
        return ETrue;
        }
    TPtr lPtr = lStrPtr->Des();

    // Check whether the Text is editable
    iSvgModule->GetTextForTextElement( aTextHandle,
        iIsTextEditOn,
        lPtr );

    if ( iIsTextEditOn && !iSvgModule->IsLoading())
        {
        TRAPD( err,
            DoTextEditingL( ETrue,             // Text Element
                            aTextHandle ,      // Element Handle
                            lPtr );            // Buffer Pointer
            );
        if ( err != KErrNone )
            {
            // No Error handling done.
            }
        }
        else
            {
            TRAPD( err,
                DoTextSelectionL( lPtr );
                );
            if ( err != KErrNone )
                {
                // No Error handling done.
                }
            }
    delete lStrPtr;
    return ETrue;
    }
   
#endif // SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE   

// -----------------------------------------------------------------------------
// CSVGTCustControl::InteractiveElementEntered()
// This callback is called to notify that an interactive element has been entered.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::InteractiveElementEntered(TPtrC /*aElementIdPtr*/, TUint16 aInteractonTypeList )
    {
  
    if( (aInteractonTypeList & ESvgInteractiveEventClick )||
        (aInteractonTypeList & ESvgInteractiveEventMousedown ) ||
        (aInteractonTypeList & ESvgInteractiveEventMouseup ) )
        {
        
        iPointerType = ESvgPointerHand;
        
        // Increment the count maintained for overlapping interactive elements
        // with pointer-down events so that the iPointertype is set properly
        iInteractiveElementEnteredCnt++;    
        
        // Need to set flag so that when the element is within 
        // textarea or text
        iPointerDownAnimation=ETrue;
            
        // Sets the MSK label
        if(iMSKImpl && !iPreviewMode)
    	    {
    	    TRAP_IGNORE(SetMSKLabelL());	
    	    }
        }
    return ETrue;
    }
// -----------------------------------------------------------------------------
// CSVGTCustControl::InteractiveElementExited()
// This callback is called to notify that an interactive element has been exited.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::InteractiveElementExited(TPtrC /*aElementIdPtr*/)
    {
    
    // The count of interactive elements with pointer down events registered
    // is decremented 
    if(iPointerDownAnimation)
        {
        iInteractiveElementEnteredCnt--;    
        }
    
    
    if(!iInteractiveElementEnteredCnt)
        {
        iPointerType = ESvgPointerDefault;
        
        iPointerDownAnimation=EFalse;
        //Set the MSK label
        if(iMSKImpl && !iPreviewMode)
    	    {    	
    	    TRAP_IGNORE(SetMSKLabelL());	
    	    }
        }
       
    return ETrue;
    }
// -----------------------------------------------------------------------------
// CSVGTCustControl::ReportAllElements
// Implements MSvgLoadingListener::ReportAllElements.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::ReportAllElements()
    {
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::WillAssignImageData
// Implements MSvgLoadingListener::WillAssignImageData.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::WillAssignImageData()
    {
    // Return ETrue to indicate that AssignImageData will be used to
    // asynchronously load the image in the engine
    if(iProgressiveRendering)
    	{
    	return ETrue;
    	}
    else
    	{
    	return EFalse;
    	}
	}

// -----------------------------------------------------------------------------
// CSVGTCustControl::ImageDataReference
// Implements MSvgLoadingListener::ImageDataReference.
// -----------------------------------------------------------------------------
//
void CSVGTCustControl::ImageDataReference( const TDesC& aUri )
    {
    CSvgtEvent* event = NULL;
    if ( aUri.Length() > KEmbededImageTagLength &&
        aUri.Left(KEmbededImageTagLength) == KDataTagName )
        {
        event = new CSvgtEventEmbededImage(aUri);
        }
    else
        {
        event = new CSvgtEventFetchImage(aUri);
        }
    if ( iEventHandlerAO )
        {
        iEventHandlerAO->AddEventToList( event );       
        iEventHandlerAO->MakeRequestComplete( KErrNone );
        }  
    }
// -----------------------------------------------------------------------------
// CSVGTCustControl::ReportAllElements
// Implements MSvgLoadingListener::DocumentStart.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::DocumentStart()
    {
    return EFalse;
    }
   
// -----------------------------------------------------------------------------
// CSVGTCustControl::DocumentEnd
// Implements MSvgLoadingListener::DocumentEnd.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::DocumentEnd()
    {
    if(iProgressiveRendering)
        {
        iEventHandlerAO->SetDocumentComplete();
        iEventHandlerAO->AddRedrawEventToList( ETrue );
        iEventHandlerAO->MakeRequestComplete( KErrNone );
        }
    return EFalse;
    }
   
// -----------------------------------------------------------------------------
// CSVGTCustControl::ElementStart
// Implements MSvgLoadingListener::ElementStart.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::ElementStart( const TDesC& aTagName,
                    MSvgAttributeList& /*aAttributeList*/)
    {
    if ( iProgressiveRendering && aTagName == KSvgTagName && iEngineInitialized
                                                                 == EFalse )
        {
        iEngineInitialized = ETrue;
        iSvgModule->AddListener( static_cast < MSvgViewPortListener*>(this), ESvgViewPortListener );
        iSvgModule->InitializeEngine( NULL, EFalse);
        }
    else if ( iProgressiveRendering )
        {
//        iEventHandlerAO->RequestRedraw();
//        User::After( 10 * 1000 );
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ElementEnd
// Implements MSvgLoadingListener::ElementEnd.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::ElementEnd( const TDesC& /* aTagName */)
    {
    if(iProgressiveRendering)
        {
        iEventHandlerAO->AddRedrawEventToList();
        iEventHandlerAO->MakeRequestComplete( KErrNone );
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ExternalDataRequested
// Implements MSvgLoadingListener::ExternalDataRequested.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::ExternalDataRequested( const TDesC& /* aUri */)
    {
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ExternalDataReceived
// Implements MSvgLoadingListener::ExternalDataReceived.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::ExternalDataReceived( const TDesC& /* aUri */)
    {
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::ExternalDataRequestFailed
// Implements MSvgLoadingListener::ExternalDataRequestFailed.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::ExternalDataRequestFailed( const TDesC& /* aUri */)
    {   
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTCustControl::UnsupportedElement
// Implements MSvgLoadingListener::UnsupportedElement.
// -----------------------------------------------------------------------------
//
TBool CSVGTCustControl::UnsupportedElement( const TDesC& /* aTagName */,
                    MSvgAttributeList& /* aAttributeList*/ )
    {
    return EFalse;
    }
void CSVGTCustControl::ImagesLoaded(TInt aError)
    {
    if(aError != KErrNone)
        {
        if(!iIsWaitNoteDisplayed)
            {
            iIsWaitNoteDisplayed = ETrue;
            TRAP_IGNORE( DisplayInfoNoteL( R_SVGT_FILE_NOT_COMPLETE, ETrue ) );
            }
        }
    }

void CSVGTCustControl::DoExitFromDialogL()
    {
    if ( iExitObserver )
        {
        iExitObserver->HandleApplicationExitL(EEikCmdExit);
        }
    }
   
void CSVGTCustControl::AssignImageData( const TDesC& aUri, HBufC8* aData )
    {
    if(aData)
        {
        iSvgModule->AssignImageData(aUri, aData);
        // Do a redraw to reflect the image loaded
        iSvgModule->Redraw();
        }
    else
        {
        // Show information note
        if(!iIsWaitNoteDisplayed)
            {
            iIsWaitNoteDisplayed = ETrue;
            TRAP_IGNORE( DisplayInfoNoteL( R_SVGT_FILE_NOT_COMPLETE, ETrue ) );
            }
        }
    }
TInt CSVGTCustControl::DoHandleLoadingThreadError()
    {
    TInt lErrCode = KErrNone;
    // An error note "Unable to open file" for eg. should be displayed
    // in the following conditions
    //  a. Error Object contains NULL. This means possible out of memory
    //     when create error object.
    //  b. Error Object has an error which is not a warning - This
    //     indicates unrecoverable error in parsing -
    //     invalid/corrupted content
    if ( (! iLoadingThreadError ) || ( iLoadingThreadError &&
                                       iLoadingThreadError->HasError() &&
                                       !iLoadingThreadError->IsWarning() ) )
        {
        TRAP_IGNORE( DisplayInfoNoteL( R_SVGT_FILE_OPEN_FAILURE, ETrue ) );
   
        // Exiting from the dialog   
        if ( iLoadingThreadError )
            {
            lErrCode = iLoadingThreadError->SystemErrorCode();
            if ( lErrCode == KErrNone )
                {
                lErrCode = KErrGeneral;
                }
            }
        else
            {
            lErrCode = KErrNoMemory;
            }
        }

    if ( lErrCode == KErrNone )
        {
// Placed into the DoPostLoadFuncL
        }

    return lErrCode;
    }
   
CSVGTCustControl::TFileType
        CSVGTCustControl::ImageFileType( const TDesC& aUri ) const
    {

    TUriParser UriParser;
   
    TBuf<KMaxFileName> localUri;
    // url starts with www so append http://
    if ( !aUri.Left( KSchemaLength ).Compare( KWww ) )    
        {
        localUri.Append( KHttp );
        localUri.Append( aUri );
        if ( UriParser.Parse( localUri ) )
            {
            return ( EInvalidFilePath );
            }
        }
    else
        {
        if ( UriParser.Parse( aUri ) )
            {
            return ( EInvalidFilePath );
            }
        }
   
    if ( UriParser.IsPresent( EUriHost ) )
        {
        // Extract the Path, filename and extension from the uri
        TFileName filename = UriParser.Extract( EUriPath );
        TParsePtrC fileParser( filename );
   
        if ( fileParser.NamePresent() && fileParser.ExtPresent() )
            {                       
            if ( fileParser.Ext().Compare( KJpg ) == 0  || 
                 fileParser.Ext().Compare( KJpeg ) == 0 ||
                 fileParser.Ext().Compare( KBmp ) == 0  ||
                 fileParser.Ext().Compare( KPng ) == 0  ||
                 fileParser.Ext().Compare( KSvg ) == 0  ||
                 fileParser.Ext().Compare( KSvgz ) == 0 )
                {
                // file name and ext present and
                // it is amy of the supported image types
                return ERemoteImageFile;        
                }
            else
                {
                //file Extension present but not a supported image ext,
                //may be some html file
                return ENotAnImageFile;
                }
            }
        else
            {
            //Should be a domain name so return ENotAnImageFile
            return ENotAnImageFile;
            }
        }
    else
        {    
        return ELocalImageFile;
        }                       
    }

/**
* CSVGTCustControl::ResetBacklightTime
* Resets the backlight time for finite content
*/
void CSVGTCustControl::ResetBacklightTime()
    {
    TTimeIntervalSeconds secondCount;
    TTime currTime;
    currTime.HomeTime();
    currTime.SecondsFrom(iPreviousBackLightTimeStamp, secondCount);
   
    if ( iDisplayFlags & ESvgUseBackLight )
        {
        if(secondCount.Int() >= KBackLightTimeInterval)
           {
           User::ResetInactivityTime();
           iPreviousBackLightTimeStamp = currTime;
           }       
        }
    }

void CSVGTCustControl::StopEventHandler()
    {
    if ( iEventHandlerAO )
        {
        iEventHandlerAO->Cancel();
        }
    }

void CSVGTCustControl::SetThreadRunning(TBool aIsRunning)
    {
    iIsThreadRunning = aIsRunning;
    }

TBool CSVGTCustControl::IsThreadRunning() const
    {
    return iIsThreadRunning;
    }

void CSVGTCustControl::PerformEngineRedraw()   
    {
    iSvgModule->Redraw();
    }

//TOUCH SUPPORT START   
#ifdef RD_SCALABLE_UI_V2
void CSVGTCustControl::HandlePointerEventL(const TPointerEvent& aPointerEvent)
	{
	if(aPointerEvent.iType == TPointerEvent::EButton1Down)
		{
        if(iPntrObserver && iPntrObserver->HandleCustControlPointerEventL(aPointerEvent))
            return;

        // Reset the select cause
        iSelectCause = ESvgInvalidSelectCause;

		TIndicatorType indicatorType =
            ActiveIndicator(aPointerEvent.iPosition);
		if (indicatorType != ENoIndicator)
		    {
		    if(indicatorType == EPauseIndicator)
                {
                ProcessPlayCmd();
                }
            else
                {
        	    PanPresentation( indicatorType );
                }
		    }
    
            // Get the size of the content
    	TSize svgSize = iSvgModule->Size();
    	TInt rgnw = svgSize.iWidth;
    	TInt rgnh = svgSize.iHeight;     
        if ( aPointerEvent.iPosition.iX >= rgnw || aPointerEvent.iPosition.iY >= rgnh  )
        	return;  
        iPointerX = aPointerEvent.iPosition.iX;
        iPointerY = aPointerEvent.iPosition.iY;
        iSvgModule->MouseDown( iPointerX, iPointerY );       

        SetPointerDisplayStatusAndRedraw(ETrue);
        UpdatePointer();

		Window().RequestPointerRepeatEvent(KInitRepeatInterval, ActiveIndicatorRect(indicatorType));
		}
	else if(aPointerEvent.iType == TPointerEvent::EButtonRepeat)
		{
        UpdatePointer();
		TIndicatorType indicatorType =
            ActiveIndicator(aPointerEvent.iPosition);
		PanPresentation( indicatorType );
		Window().RequestPointerRepeatEvent(KInitRepeatInterval, ActiveIndicatorRect(indicatorType));
		}
	else if(aPointerEvent.iType == TPointerEvent::EButton1Up)
		{
		Window().CancelPointerRepeatEventRequest();
        // UI Spec: Pointer should be hidden after 15 sec of
        // user inactivity
        if ( iInactivityTimer )
            {
            // Should not be the case!
            delete iInactivityTimer;
            iInactivityTimer = NULL;
            }
        iInactivityTimer = CSVGTPointerHideTimer::NewL( this );
        iInactivityTimer->After( KSccPointerHideTimerStartAfter );

		TIndicatorType indicatorType =
            ActiveIndicator(aPointerEvent.iPosition);
		if (indicatorType == ENoIndicator)
		    {
            iSvgModule->MouseUp( iPointerX, iPointerY );
		    }
		}
	else if(aPointerEvent.iType == TPointerEvent::EDrag)
	    {
	       // Get the size of the content
    	TSize svgSize = iSvgModule->Size();
    	TInt rgnw = svgSize.iWidth;
    	TInt rgnh = svgSize.iHeight;     
        if ( aPointerEvent.iPosition.iX >= rgnw || aPointerEvent.iPosition.iY >= rgnh  )
        	return;
        iPointerX = aPointerEvent.iPosition.iX;
        iPointerY = aPointerEvent.iPosition.iY;

        iSvgModule->MouseMove( iPointerX, iPointerY );
        SetPointerDisplayStatusAndRedraw(ETrue);
        UpdatePointer();
	    }
	}

void CSVGTCustControl::SetContainerWindowL(const CCoeControl &aContainer)
    {
    CCoeControl::SetContainerWindowL(aContainer);
    EnableDragEvents();
    }
    
void CSVGTCustControl::PanPresentation( 
    const CSVGTCustControl::TIndicatorType aIndicatorType )
    {
	switch( aIndicatorType )
	    {
	    case ELeftIndicator:
	        {
			iSvgModule->Pan( KSccPanNumPixelsX, KSccConstZero );
			break;
	        }
	    case ERightIndicator:
	        {
			iSvgModule->Pan( -KSccPanNumPixelsX, KSccConstZero );
			break;
	        }
	    case EUpIndicator:
	        {
			iSvgModule->Pan( KSccConstZero, KSccPanNumPixelsY );
			break;
	        }
	    case EDownIndicator:
	        {
			iSvgModule->Pan( KSccConstZero, -KSccPanNumPixelsY );
			break;
	        }
	    }
	    iSvgModule->Redraw();
	    DrawDeferred();
	}

CSVGTCustControl::TIndicatorType CSVGTCustControl::ActiveIndicator(
    const TPoint aPosition ) const
	{
	TIndicatorType indicatorType = ENoIndicator;
	
    if( iPanIndLeftAidRect.Rect().Contains( aPosition ) && iPanLt)
		{
		indicatorType = ELeftIndicator;
		}
    else if( iPanIndRightAidRect.Rect().Contains( aPosition ) && iPanRt)
		{
		indicatorType = ERightIndicator;
		}
    else if( iPanIndUpAidRect.Rect().Contains( aPosition ) && iPanUp)
		{
		indicatorType = EUpIndicator;
		}
    else if( iPanIndDownAidRect.Rect().Contains( aPosition ) && iPanDn)
		{
		indicatorType = EDownIndicator;
		}
	
	return indicatorType ;
	}

TRect CSVGTCustControl::ActiveIndicatorRect( TIndicatorType aIndicator) const
    {
    TRect indicatorRect(0,0,0,0);
    
    switch(aIndicator)
        {
        case ELeftIndicator:
            {
    		indicatorRect = iPanIndLeftAidRect.Rect();
            }
            break;
        case ERightIndicator:
            {
    		indicatorRect = iPanIndRightAidRect.Rect();
            }
            break;
        case EUpIndicator:
            {
    		indicatorRect = iPanIndUpAidRect.Rect();
            }
            break;
        case EDownIndicator:
            {
    		indicatorRect = iPanIndDownAidRect.Rect();
            }
            break;
        }

	return indicatorRect;	
    }	

#endif //RD_SCALABLE_UI_V2

#ifdef RD_SVGT_AUDIO_SUPPORT
void CSVGTCustControl::HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType)
    {
    if((aControl == iVolumePopup) && ( aEventType == MCoeControlObserver::EEventStateChanged))
        {
#ifdef _DEBUG
        TName msg;
        msg.Format(_L("Volume level: %d"), iVolumePopup->Value());
        iEikonEnv->InfoMsgWithAlignment(TGulAlignment(EHCenterVCenter), msg);
#endif  
		// Save the current volume as previous volume before setting
        // new volume level
        iPrevVolume=iVolumeLevel;    
        iVolumeLevel = iVolumePopup->Value();
        SetPresentationVolume( iVolumeLevel );
        }
    }
    
void CSVGTCustControl::SetPresentationVolume( TInt aVolumeLevel )
    {
    TInt volumeInPercent = aVolumeLevel * 10;

    if(volumeInPercent)
        {
        
        iSvgModule->SetAudioVolume(volumeInPercent);
        if(iIsVolumeMuted)
            {
            iIsVolumeMuted = EFalse;
            if(iAudioMuteListener)
                {
                iAudioMuteListener->VolumeMuted(EFalse);    
                }
            
            }
        }
        
    else
        {
        iSvgModule->MuteAudioVolume();
        if(!iIsVolumeMuted)
            {
            iIsVolumeMuted = ETrue;
            if(iAudioMuteListener)
                {
                iAudioMuteListener->VolumeMuted(ETrue);    
                }
            
            }
        
        }
        
    }

TInt CSVGTCustControl::PresentationVolume() const
    {
    return iVolumeLevel;
    }
    
void CSVGTCustControl::MrccatoCommand(TRemConCoreApiOperationId aOperationId, 
                             TRemConCoreApiButtonAction aButtonAct )
    {
    switch(aOperationId)
        {
        case ERemConCoreApiVolumeUp:
            {
            switch (aButtonAct)
                {
                case ERemConCoreApiButtonPress:
                    {
                    break;
                    }
                case ERemConCoreApiButtonRelease:
                    {
                    break;
                    }
                case ERemConCoreApiButtonClick:
                    {
                    FilterAndSendCommand(EStdKeyIncVolume);
                    break;
                    }
                default:
                    {
                    break;
                    }
                }
            break;
            }
        case ERemConCoreApiVolumeDown:
            {
            switch (aButtonAct)
                {
                case ERemConCoreApiButtonPress:
                    {
                    break;
                    }
                case ERemConCoreApiButtonRelease:
                    {
                    break;
                    }
                case ERemConCoreApiButtonClick:
                    {
                    FilterAndSendCommand(EStdKeyDecVolume);
                    break;
                    }
                default:
                    {
                    break;
                    }
                }
            break;
            }
        default:
            {
            break;
            }
        }        
    }

void CSVGTCustControl::FilterAndSendCommand(TInt aCommandId)
    {    
    TInt currentVolume = iVolumeLevel;
    
    if(!iIsForeground)
        {
        return;
        }
        
    switch(aCommandId)
        {
        case EStdKeyIncVolume:
            currentVolume=(currentVolume< KMaxVolume)? ++currentVolume : KMaxVolume;
            break;
        case EStdKeyDecVolume:
            currentVolume=(currentVolume> KMinVolume)? --currentVolume : KMinVolume;
            break;  
        default:
            break;      
        }
         
    iVolumePopup->SetValue( currentVolume );
    TRAP_IGNORE(iVolumePopup->ShowVolumePopupL());
    iVolumePopup->DrawNow();

    TRAP_IGNORE(HandleControlEventL( iVolumePopup, MCoeControlObserver::EEventStateChanged ));    
}

EXPORT_C void CSVGTCustControl::AddListener(MSvgCustControlListener *aListener, TCustControlListener aType)
    {
    if(aType == EAudioListener)
        {
        iAudioMuteListener = (MSvgAudioMuteListener *)aListener;    
        }
    
    }
TInt CSVGTCustControl::GetPrevVolume() const
    {
    return iPrevVolume;
    } 
#endif  //RD_SVGT_AUDIO_SUPPORT

//TOUCH SUPPORT END

// End of File
