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
* Description:  This file implements the SVGT Dialog which
*                is used to display SVGT content
*
*/

#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <uikon/eikctrlstatus.h>
#endif

#include <e32std.h>
#include <AknDialog.h>
#include <coeaui.h>
#include <eikenv.h>
#include <bautils.h>
#include <eikdialg.h>
#include <eikmenup.h>
#include <eikspane.h>

#ifdef __SERIES60_HELP
#include <hlplch.h>
#endif // __SERIES60_HELP

#include <coeaui.h>
#include <aknnavi.h> 
#include <aknnavide.h> 
#include <AknWaitNoteWrapper.h>
#include <SVGRequestObserver.h>
#include <aknnotewrappers.h>
#include <akntitle.h>
#include <data_caging_path_literals.hrh>
#include <StringLoader.h>
#include <SVGTUIControl.rsg>
#include <csxhelp/svgt.hlp.hrh>
//-----------

// User Includes
#include "SVGTAppObserver.h"
#include "SVGTCustControl.h"
#include "SVGTUIDialog.h"
#include "svgtfileviewdetails.h"

#include "SVGTUIDialog.hrh"
#include "SvgtDecoratorControl.h"

const TInt KButtonContainerDisplayTime = 5000000;

// Constants

// The Middle softkey has position 3 at button group's stack
const TInt KMiddleSKId=3;
_LIT( KResourceFileName, "Z:SVGTUIControl.rsc" );
#ifdef _DEBUG
//_LIT( KUnknownCommandStr, "Unknown Command" );
#endif

#ifdef __SERIES60_HELP
// Help Context UID = DLL UID3
const TUid KSvgtHelpContextUid = { 0x101F8749 };
#endif // __SERIES60_HELP

// Times longer than this many seconds are shown using the long format
const TInt KShortTimeLimit = 59*60 + 59; 

// Maximum number of seconds shown in the time duration field
const TInt KMaxTotalTimeSeconds = 9*60*60 + 59*60 + 59; // 59:59

// Maximum length for both the time elapsed/remaining string "mm:ss/mm:ss"
// and post-recording text "Length: mm:ss"
const TInt KMaxTimeTextLength = 40;

// This many microseconds in a second
const TInt KMicro = 1000000;  

// Key Constant for toggling between Full/Normal Screen
const TInt KSvgScreenToggleKey = '*';
// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTUIDialog::CSVGTUIDialog
// Default Constructor. Initialises Dialog State Variables.
// -----------------------------------------------------------------------------
//
CSVGTUIDialog::CSVGTUIDialog()
    {
    // SVGT Viewer State Variables
    iSaveEnable = EFalse;
    iPreview = EFalse;
    iCustControl = NULL;
    iAppObserver = NULL;
    iIsVolumeMuted = EFalse;
    iLastCommandID = EAknSoftkeyOptions;
    iIsButtonContainerVisible = ETrue;
    iQuietExitWhenOrientationChange = EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::ConstructL
// Second Phase Constructor for CSVGTUIDialog. Opens resource file, reads the
// dialog menu resource and calls CAknDialog::ConstructL with the resource as
// param.
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::ConstructL()
    {
    // Find which drive this DLL is installed.
    TFileName fileName;

    // Get the full resource file
    TParse lParse;

    // Add the resource dir
    lParse.Set( KResourceFileName, &KDC_RESOURCE_FILES_DIR, NULL);

    // Get the filename with full path
    fileName = lParse.FullName();

    BaflUtils::NearestLanguageFile( iEikonEnv->FsSession(), fileName ); //for
                                                               // localization
    iResourceOffset = iEikonEnv->AddResourceFileL( fileName );

    // Read time format strings from AVKON resource
    iTimeFormatShort = 
        iEikonEnv->AllocReadResourceL( R_QTN_TIME_DURAT_MIN_SEC_WITH_ZERO );
    iTimeFormatLong =
        iEikonEnv->AllocReadResourceL( R_QTN_TIME_DURAT_LONG );

    iBtnCntrTimer = CPeriodic::NewL( CActive::EPriorityStandard);
    CAknDialog::ConstructL( R_SVGT_DIALOG_MENUBAR );

    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::NewL
// Factory function for creating CSVGTUIDialog objects.
// Returns: CSVGTUIDialog* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
EXPORT_C CSVGTUIDialog* CSVGTUIDialog::NewL()
    {
    CSVGTUIDialog* self = CSVGTUIDialog::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::NewLC
// Factory function for creating CSVGTUIDialog objects. It also pushes the
// created dialog object onto the cleanup stack.
// Returns: CSVGTUIDialog* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
EXPORT_C CSVGTUIDialog* CSVGTUIDialog::NewLC()
    {
    CSVGTUIDialog* self = new ( ELeave ) CSVGTUIDialog();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// Destructor
CSVGTUIDialog::~CSVGTUIDialog()
    {
    if ( iResourceOffset != 0 )
        {
        iEikonEnv->DeleteResourceFile( iResourceOffset );
        }
    // Custom Control will be automatically destroyed by dialog framework
    iCustControl = NULL;
    // App Observer is reset.
    iAppObserver = NULL;

    // Set the text in status pane to the stored one            
    if (iAppTitleText)
        {
        // Get the Status Pane Control
        CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
        // Get the title pane 
        CAknTitlePane* tp = NULL;
        TRAPD( errGetControl, 
            tp = static_cast< CAknTitlePane* >( sp->ControlL( 
            TUid::Uid( EEikStatusPaneUidTitle ) ) ) );
        if ( errGetControl == KErrNone && tp )
            {
            //set application name in the title pane
            TRAPD( errSetText, 
                tp->SetTextL(*iAppTitleText) );
            if ( errSetText != KErrNone )
                {
                // No error handling here
                }
            }
        
        delete iAppTitleText;
        iAppTitleText = NULL;

        delete iProgressText;
        iProgressText = NULL;
        }
    
    // Clear the Navi-Pane Decorator    
    if ( iNaviDecorator )
        {
        // Restore the old navi-pane
        CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
        
        CAknNavigationControlContainer* np = NULL;
        TRAPD( errGetNaviControl, 
            np = static_cast< CAknNavigationControlContainer* >
            ( sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) ) );
        if ( errGetNaviControl == KErrNone && np )
            {
            TRAPD( errPushDefNaviControl, 
                np->PushDefaultL ()); // Set default.    
            if ( errPushDefNaviControl != KErrNone )
                {
                // No error handling here.
                }
            }
        delete iNaviDecorator;
        }    
    iCbaGroup=NULL;
    // Callers responsibility of closing the file handle.
    iFileHandlePtr = NULL;
    // Delete the format strings
    delete iTimeFormatShort;
    delete iTimeFormatLong;
    
    delete iBtnCntrTimer;
    }

TInt CSVGTUIDialog::ButtonContainerTimerCallBack(TAny* aAny)
    {
    CSVGTUIDialog* dlg = (CSVGTUIDialog*)(aAny);
    dlg->iBtnCntrTimer->Cancel();
    dlg->iIsButtonContainerVisible = EFalse;
    TRAP_IGNORE(dlg->SwitchDialogLayoutL( ETrue ));
    
    return KErrNone;
    }

TBool CSVGTUIDialog::HandleCustControlPointerEventL(const TPointerEvent& /* aPointerEvent */)
    {
	TBool fullScreenStatus;
	TBool isEventHandled = EFalse;
    TCallBack callBack(CSVGTUIDialog::ButtonContainerTimerCallBack, this);

    iBtnCntrTimer->Cancel();
    
    if(iIsButtonContainerVisible)
        {
        return isEventHandled;
        }
    
	iCustControl->GetCurrentFullScreenStatus(fullScreenStatus);
    if(fullScreenStatus)
        {
        TRect appRect = iEikonEnv->EikAppUi()->ApplicationRect();
        
        if(iBtnGrpPos.iX && iBtnGrpPos.iY)
            SetSize(TSize(appRect.Width() - iBtnGrpSize.iWidth, appRect.Height() - iBtnGrpSize.iHeight));
        if(iBtnGrpPos.iX)
            SetSize(TSize(appRect.Width() - iBtnGrpSize.iWidth, appRect.Height()));
        else if(iBtnGrpPos.iY)
            SetSize(TSize(appRect.Width(), appRect.Height() - iBtnGrpSize.iHeight));
        
        CEikButtonGroupContainer& bgc = ButtonGroupContainer();
        //bgc.MakeVisible(ETrue);
        //bgc.SetSize(iBtnGrpSize);
        //bgc.SetExtent(iBtnGrpPos, iBtnGrpSize);
        //bgc.DrawDeferred();
        iIsButtonContainerVisible = ETrue;
        isEventHandled = ETrue;
        iBtnCntrTimer->Start(KButtonContainerDisplayTime, 0, callBack);
        }
        
    return isEventHandled;
    }
    
// -----------------------------------------------------------------------------
// CSVGTUIDialog::ExecuteLD
// Reimplements CAknDialog::ExecuteLD. It initialises the member variables of
// the class and calls CAknDialog::ExecuteLD() with the dialog resource.
// -----------------------------------------------------------------------------
//
TInt CSVGTUIDialog::ExecuteLD(
    RFile& aFileHandle,         // Filename of SVGT Content
    MSvgtAppObserver* ptr )     // Applications Implementation of callbacks
    {
    // Save this pointer since there are leaving functions in the flow
    CleanupStack::PushL( this );    

    iFileHandlePtr = &aFileHandle;
    iIsFileHandleValid = ETrue;
    iAppObserver = ptr;
    iQuietExitWhenOrientationChange = iAppObserver->ExitWhenOrientationChange();

    // Get the Status Pane Control
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    
    // Get the Title Pane Control
    CAknTitlePane* tp = static_cast< CAknTitlePane* >( sp->ControlL(
            TUid::Uid( EEikStatusPaneUidTitle ) ) );

    // Store the Application's title text
    iAppTitleText = HBufC::NewL(tp->Text()->Length());
    *iAppTitleText = *tp->Text();
    
    // Display the filename in the title pane of application
    
    TFileName lFileName;
    // Get the file name from the file handle
    if ( !aFileHandle.Name( lFileName ) )
        {
        TParse p;
        p.Set( lFileName, NULL, NULL);

        // SetTextL also ensures that if the length exceeds the layout space
        // available the text is truncated from the end.
        tp->SetTextL( p.NameAndExt() ); // p.NameAndExt() gives "Filename.ext"
        
        }

    CSVGTDrmHelper *drmHelper = CSVGTDrmHelper::NewLC();
    // Check if the "Save" Option should be shown or not.
    // The calling application of the dialog decides this.
    if ( iAppObserver )
        {
        iSaveEnable = iAppObserver->CanShowSave();
        }       
     
    // Check Rights Status
    iPreview = drmHelper->IsPreviewL( aFileHandle );
    
    CleanupStack::PopAndDestroy( drmHelper );
    
    // ExecuteLD will delete this, so we have to Pop it...
    CleanupStack::Pop( this );
    
    if ( iPreview )
        {              
        return  CAknDialog::ExecuteLD( R_SVGT_VIEWER_PREVIEW_DIALOG );  
        }
    else
        {
        return CAknDialog::ExecuteLD( R_SVGT_VIEWER_DIALOG );       
        }
    
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::FormatTimeL
// Convert a time given in seconds to a descriptor.
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::FormatTimeL( 
                              const TUint& aDuration, // duration in 
                                                      // seconds
                              TDes& aBuf   // Buffer holding formatted 
                                           // string
                               ) const
                                           
    {
    TTime time( TInt64( aDuration ) * KMicro );

    if ( time.Int64() > TInt64( KMaxTotalTimeSeconds ) * KMicro )
        {
        time = TInt64( KMaxTotalTimeSeconds ) * KMicro;
        }

    if ( time.Int64() < TInt64( KShortTimeLimit ) * KMicro )
        {
        time.FormatL( aBuf, *iTimeFormatShort );
        }
    else
        {
        time.FormatL( aBuf, *iTimeFormatLong );
        }

    // Arabic number conversion
    AknTextUtils::LanguageSpecificNumberConversion( aBuf );
    }
    
// -----------------------------------------------------------------------------
// CSVGTUIDialog::SaveNeededL
// This function checks if Save is displayed and saving is already done
// and prompts a confirmation query to the user and then calls DoSaveL
// -----------------------------------------------------------------------------
//
TBool CSVGTUIDialog::SaveNeededL( TInt aButtonId )
    {
    if ( ( iAppObserver->CanShowSave() || 
           iCustControl->IsTextContentChanged()) &&
         !iAppObserver->IsSavingDone() && 
         !iPreview &&            
         LaunchSaveQueryDialogL() )
        {
        iAppObserver->DoSaveL( aButtonId );
        return ETrue;       
        }
    else
        {
        return EFalse;
        }          
    }
// -----------------------------------------------------------------------------
// CSVGTUIDialog::LaunchSaveQueryDialogL
// -----------------------------------------------------------------------------
//
TBool CSVGTUIDialog::LaunchSaveQueryDialogL() const
    {
    HBufC* titleText = NULL;

    TFileName lFileName;
    if ( iFileHandlePtr )
        {
        iFileHandlePtr->Name( lFileName );
        }
    else
        {
        return EFalse;
        }
    titleText = StringLoader::LoadLC( R_QTN_SVGT_QUERY_CONF_SAVE_DOWNLOADED, 
                              lFileName );

    CAknQueryDialog* query = CAknQueryDialog::NewL( );
    TBool retVal = query->ExecuteLD( R_SVGT_CONFIRMATION_QUERY_YES_NO, 
                                                   *titleText );
    CleanupStack::PopAndDestroy( titleText );
    
    if ( retVal == EAknSoftkeyOk || retVal == EAknSoftkeyYes )
        {
        return ETrue;
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::SwitchDialogLayoutL
// This function changes the layout of the dialog
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::SwitchDialogLayoutL( TBool aIsFullScreen )
    {
    if ( aIsFullScreen )
        {
        // Full Screen Mode
        // Expand Dialog Window to occupy full screen
        SetExtentToWholeScreen();
        // Hide the Status Pane
        CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
        sp->MakeVisible( EFalse );
        
        // Hide the CBA
        CEikButtonGroupContainer* cp = &( ButtonGroupContainer() );
        //cp->SetExtent( TPoint( 0, 0 ), TSize( 0, 0) );
        cp->SetSize( TSize( 0, 0) );
        
        }
    else
        {
        // Normal Screen Mode
        // Show the Status Pane
        CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
        sp->MakeVisible( ETrue );
        
        // Restore the size and position of the dialog
        // to normal screen size(Client Rectangle)
        TRect lRect;
        iCustControl->GetNormalScreenLayoutRect( lRect );
        SetSizeAndPosition( lRect.Size() );
        // Restore original layout
        Layout();
        iBtnCntrTimer->Cancel();
        }
    
    iIsButtonContainerVisible = !aIsFullScreen;
    
    }
    
// -----------------------------------------------------------------------------
// CSVGTUIDialog::DynInitMenuPaneL
// Reimplements CAknDialog::DynInitMenuPaneL inorder to support dynamic hiding
// of menu items based on current state of the viewer.
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::DynInitMenuPaneL(
    TInt aResourceId,           // Resource Id for which this func is called
    CEikMenuPane* aMenuPane )   // Menupane object pointer for manipulation
    {
    iBtnCntrTimer->Cancel();
   
if(
  (aResourceId==R_SVGT_DIALOG_MENU)
  &&iCustControl->GetMSKLabelFlag(EAknSoftkeyContextOptions)
  &&(aMenuPane)
  )
    	{
    	iCustControl->ChangeMSKLabelFlag(EAknSoftkeyContextOptions);
    	if ( iAppObserver )
    		{
    		iSaveEnable = iAppObserver->CanShowSave();
    		}
    	aMenuPane->SetItemDimmed(ESvgtDlgSaveCommand,
    	                        !( iSaveEnable||
    	                        iCustControl->IsTextContentChanged() ) );
          
        TSvgtViewerAnimStatus lAnimState;
        iCustControl->GetCurrentAnimState( lAnimState );
        TBool isLoadingDone = iCustControl->IsProcessDone();
        aMenuPane->SetItemDimmed(ESvgtDlgPlayCommand,
        						!isLoadingDone ||
        						(lAnimState == ESvgAnimPlayingState ));
        aMenuPane->SetItemDimmed(ESvgtDlgPauseCommand,
                                !isLoadingDone ||
                                (lAnimState != ESvgAnimPlayingState ));                                                
        aMenuPane->SetItemDimmed(ESvgtDlgStopCommand,
                                !isLoadingDone ||
                                (lAnimState ==ESvgAnimStoppedState));                                
                                         
          
        //All the other menu items are dimmed since they are not the part of
        // context menu
        aMenuPane->SetItemDimmed(ESvgtDlgZoomInCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgZoomOutCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgSelectTextCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgEditTextCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgLoopCascadeCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgFindTextCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgFullScreenCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgNormalScreenCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgListRotateCascadeCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgViewDetailsCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgListDownloadCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgHelpCommand,ETrue);
        aMenuPane->SetItemDimmed(EAknCmdExit,ETrue);           
        #ifdef RD_SVGT_AUDIO_SUPPORT
        aMenuPane->SetItemDimmed( ESvgtDlgMuteCommand,ETrue);
        aMenuPane->SetItemDimmed( ESvgtDlgUnmuteCommand,ETrue);
        aMenuPane->SetItemDimmed(ESvgtDlgSetVolumeCommand,ETrue);
        #endif //RD_SVGT_AUDIO_SUPPORT         
        }
    
    else if ( ( aResourceId == R_SVGT_DIALOG_MENU ) && ( aMenuPane ) )
        {
        if ( iAppObserver )
            {
            iSaveEnable = iAppObserver->CanShowSave();
            }
        aMenuPane->SetItemDimmed(
                        ESvgtDlgSaveCommand,
                        !( iSaveEnable || 
                        iCustControl->IsTextContentChanged() ) );
        TInt zoomLvl = 0;
        iCustControl->GetCurrentZoomLevel( zoomLvl );
        aMenuPane->SetItemDimmed(
                        ESvgtDlgZoomOutCommand,
                        ( zoomLvl == 0 ) );
        
        // Select Text Option
        aMenuPane->SetItemDimmed( ESvgtDlgSelectTextCommand, 
            !iCustControl->IsTextSelectOn() );
        
        // Edit Text Option
        aMenuPane->SetItemDimmed( ESvgtDlgEditTextCommand, 
            !iCustControl->IsTextEditOn() );
            
        if ( !iCustControl->IsAnimationPresent() )
            {
            // No Animations case - dim the Play/Pause/Stop buttons
            aMenuPane->SetItemDimmed(
                        ESvgtDlgPlayCommand,
                        ETrue );

            aMenuPane->SetItemDimmed(
                        ESvgtDlgPauseCommand,
                        ETrue );
            
            aMenuPane->SetItemDimmed(
                        ESvgtDlgStopCommand,
                        ETrue );
            aMenuPane->SetItemDimmed(
                        ESvgtDlgLoopCascadeCommand,
                        ETrue );                            
            }
            else
                {
                // Animations case - normal behaviour
                // Play/Pause Options
                TSvgtViewerAnimStatus lAnimState;
                iCustControl->GetCurrentAnimState( lAnimState );
                TBool isLoadingDone = iCustControl->IsProcessDone();
                
                aMenuPane->SetItemDimmed(
                                ESvgtDlgPlayCommand,
                                !isLoadingDone || 
                                 ( lAnimState == ESvgAnimPlayingState ) );

                aMenuPane->SetItemDimmed(
                                ESvgtDlgPauseCommand,
                                !isLoadingDone || 
                                 ( lAnimState != ESvgAnimPlayingState ) );
                
                aMenuPane->SetItemDimmed(
                                ESvgtDlgStopCommand,
                                !isLoadingDone || 
                                 ( lAnimState == ESvgAnimStoppedState ) );
                // Loop Option 
                aMenuPane->SetItemDimmed(
                                ESvgtDlgLoopCascadeCommand,
                                !isLoadingDone || 
                                !iCustControl->IsLoopAllowed() );
                }
#ifdef RD_SVGT_AUDIO_SUPPORT            
        aMenuPane->SetItemDimmed( ESvgtDlgMuteCommand, iIsVolumeMuted );

        aMenuPane->SetItemDimmed( ESvgtDlgUnmuteCommand, !iIsVolumeMuted );
#endif //RD_SVGT_AUDIO_SUPPORT
        TBool isFullScreenOn;
        iCustControl->GetCurrentFullScreenStatus( isFullScreenOn );
        aMenuPane->SetItemDimmed(
                        ESvgtDlgFullScreenCommand,
                        isFullScreenOn );

        aMenuPane->SetItemDimmed(
                        ESvgtDlgNormalScreenCommand,
                        !isFullScreenOn );

        aMenuPane->SetItemDimmed(
                        ESvgtDlgFindTextCommand,
                        EFalse );

        aMenuPane->SetItemDimmed(
                        ESvgtDlgViewDetailsCommand,
                        !iIsFileHandleValid);
        aMenuPane->SetItemDimmed(
                        ESvgtDlgListDownloadCommand,
                         !iAppObserver->CanShowDownload() );
        }
        else 
            {
            if ( ( aResourceId == R_SVGT_LOOP_SUBMENU ) && ( aMenuPane ) )
                {
                // Loop On/Off Submenu option
                TBool isLoopOn = ETrue;
                iCustControl->GetCurrentLoopStatus( isLoopOn );
                if ( isLoopOn )
                    {
                    aMenuPane->SetItemButtonState( ESvgtDlgLoopOnCommand, 
                                                   EEikMenuItemSymbolOn );
                    aMenuPane->SetItemButtonState( ESvgtDlgLoopOffCommand,
                                             EEikMenuItemSymbolIndeterminate );
                    }
                    else
                        {
                        aMenuPane->SetItemButtonState( ESvgtDlgLoopOffCommand, 
                                                       EEikMenuItemSymbolOn );
                        aMenuPane->SetItemButtonState( ESvgtDlgLoopOnCommand, 
                                             EEikMenuItemSymbolIndeterminate );
                        }
                }
            }
    }
// -----------------------------------------------------------------------------
// CSVGTUIDialog::DrawMSKLabelL
// Reimplements MSVGTMSKImplL::DrawMSKLabelL.This function is called by the
// CSVGTCustControl whenever MSK label needs to be updated.
// -----------------------------------------------------------------------------
// 
void CSVGTUIDialog::DrawMSKLabelL(TInt aResourceID,TInt aCommandID)
{
    iLastCommandID = aCommandID;
	iCbaGroup= &( ButtonGroupContainer() );
  	HBufC* middleSKText = StringLoader::LoadLC(aResourceID );
    iCbaGroup->AddCommandToStackL(KMiddleSKId, aCommandID,*middleSKText );
    iCbaGroup->MakeCommandVisible(aCommandID, ETrue);
	CleanupStack::PopAndDestroy( middleSKText );
	iCbaGroup->DrawDeferred();		
}

// ----------------------------------------------------------------------------
// CSVGTUIDialog::RemoveMSKLabel
// Reimplements MSVGTMSKImplL::RemoveMSKLabel.This function is called by the 
// CSVGTCustControl before adding new MSK label to button group stack.   
// ----------------------------------------------------------------------------
//
void CSVGTUIDialog::RemoveMSKLabel()
{
	iCbaGroup= &( ButtonGroupContainer() );
	MEikButtonGroup* lButtonGroup=iCbaGroup->ButtonGroup();
	TInt lCmdID=lButtonGroup->CommandId(KMiddleSKId);
	iCbaGroup->RemoveCommandFromStack(KMiddleSKId,lCmdID);
	iCbaGroup->DrawDeferred();

}

// -----------------------------------------------------------------------------
// CSVGTUIDialog::OkToExitL
// Reimplements CAknDialog::OkToExitL inorder to display custom menu and
// trap the back softkey to provide restore view functionality/exit.
// -----------------------------------------------------------------------------
//
TBool CSVGTUIDialog::OkToExitL( TInt aButtonId ) // ButtonId of button pressed
    {
    if ( aButtonId == EAknSoftkeyOptions )
        {
        DisplayMenuL();
        return EFalse;
        }
    else if(aButtonId==EAknSoftkeyContextOptions)
   		{
        //Flag to differentiate context menu from normal options menu
       	iCustControl->ChangeMSKLabelFlag(aButtonId);
         		
       	//Display context menu 
       	iMenuBar->SetContextMenuTitleResourceId( R_SVGT_DIALOG_MENUBAR );
  		iMenuBar-> TryDisplayContextMenuBarL();
    		    
    	//No task swapper for context menu
   	    iMenuBar->SetMenuType(CEikMenuBar::EMenuOptionsNoTaskSwapper);
      			
     	//Restore the options menu
   		iMenuBar->SetMenuTitleResourceId( R_SVGT_DIALOG_MENUBAR );
   		iMenuBar->SetMenuType(CEikMenuBar::EMenuOptions);
          
       	return EFalse;
        }
    else if(aButtonId ==EAknSoftkeySave)
    	{
    	//Save the content
    	ProcessCommandL(ESvgtDlgSaveCommand);   
    	return EFalse;
    	}
  
    else
        {
        if ( aButtonId == EAknSoftkeyBack )
            {
            // Check whether full screen is active
            TBool isFullScreenActive;
            iCustControl->GetCurrentFullScreenStatus( isFullScreenActive );

            if ( isFullScreenActive )
                {
                //  Yes[FullScreen]: revert back to normal screen
                ProcessCommandL( ESvgtDlgNormalScreenCommand );
                return EFalse;
                }

            // Check whether content is zoomed in
            TInt zoomLvl = 0;
            iCustControl->GetCurrentZoomLevel( zoomLvl );

            if ( zoomLvl > 0 )
                {
                //  Yes[ZoomedIn]: restore to original view
                iCustControl->ProcessViewerCommandL(
                    ESvgtDlgZoomOptimalCommand );
                return EFalse;
                }
            return !SaveNeededL( aButtonId );            
            }
            else
                {
                // Ignore Select Key..
                if ( aButtonId == EAknSoftkeySelect || aButtonId == EEikBidOk )
                    {
					//If animation on mousedown/mouseup is present trigger it
					if(iCustControl->GetMSKLabelFlag(aButtonId))
						{
						TKeyEvent lKeyEvent;
						lKeyEvent.iScanCode=EStdKeyDevice3;
						iCustControl->OfferKeyEventL(lKeyEvent,EEventKey);	
						iCustControl->ChangeMSKLabelFlag(aButtonId);
						TRAP_IGNORE(iCustControl->SetMSKLabelL());
						}
					
						
				
                    // Select button pressed, ignore.
                    return EFalse;
                    }
                if ( aButtonId == EEikCmdExit )
                    {
                    return !SaveNeededL( aButtonId ); 
                    }
                }
            
        }
        
    return ETrue;
    }


// -----------------------------------------------------------------------------
// CSVGTUIDialog::IsLoadingDone
// This function is called by the application to check whether the
// loading of the svg content is done.
// -----------------------------------------------------------------------------
//
TBool CSVGTUIDialog::IsLoadingDone()
    {
    return iCustControl->IsProcessDone();
    }   

// -----------------------------------------------------------------------------
// CSVGTUIDialog::HandleApplicationForegroundEvent
// This API is used by the application to give background/foreground events
// to the Dialog. The dialog inturn passes the events to custom control.
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::HandleApplicationForegroundEvent( TBool aForeground )
    {
    if ( iCustControl )
        {
        iCustControl->HandleApplicationForegroundEvent( aForeground );
        }
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::StopUsingContentFileHandle
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::StopUsingContentFileHandle()
    {
    // NOTE: Currently the custom control and the engine are
    // not actively using the File handle. e.g. Engine uses
    // the file handle only during loading of content. And custom
    // control uses it when File Details are shown. 
    // If required, in future need to pass this event to custom control.
    // Block the filedetails option now.
    iIsFileHandleValid = EFalse;
    }
    
// -----------------------------------------------------------------------------
// CSVGTUIDialog::StartUsingContentFileHandle
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::StartUsingContentFileHandle()
    {
    iIsFileHandleValid = ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::IsTextContentChanged
// Function used by the client to check if text in the content was modified, 
// to check if save is needed.
// -----------------------------------------------------------------------------
//
TBool CSVGTUIDialog::IsTextContentChanged()
    {
    return (iCustControl->IsTextContentChanged());    
    }
        
// -----------------------------------------------------------------------------
// CSVGTUIDialog::SaveSvgDom
// Function used by the client to save the modified svg content
// -----------------------------------------------------------------------------
//
TInt CSVGTUIDialog::SaveSvgDom( const TDesC& aFileName )
    {
    if ( iCustControl )
        {
        return ( iCustControl->SaveSvgDom( aFileName ) );
        }
        else 
            {
            return KErrGeneral;            
            }
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::SaveComplete
// Function used by the client to indicate saving is complete
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::SaveComplete( const TInt aError )
    {
    if ( !aError )
        {
        // Indicate to Custom Control to perform any cleanup
        // once save is complete
        iCustControl->SaveComplete( aError );
        }
    }
    
// -----------------------------------------------------------------------------
// CSVGTUIDialog::ProcessCommandL
// Reimplements CAknDialog::ProcessCommandL inorder to handle dialog menu
// commands.
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::ProcessCommandL( TInt aCommandId )  // Command Id
    {
    // The Commands are handled by the custom CCoe Control and
    // a boolean varialble is used inorder to keep track whether
    // the control command processing needs to be invoked or not.
    TBool procCmdReqd = ETrue;
    CAknDialog::ProcessCommandL( aCommandId );
    switch ( aCommandId )
        {
        case ESvgtDlgSaveCommand: 
            if ( iAppObserver )
                {
                iAppObserver->DoSaveL( ESvgtDlgCommand1 );
                }
            break;
        case ESvgtDlgHelpCommand:
#ifdef __SERIES60_HELP            
            procCmdReqd = EFalse;
            HlpLauncher::LaunchHelpApplicationL( iEikonEnv->WsSession(),
                iEikonEnv->EikAppUi()->AppHelpContextL() );
#endif // __SERIES60_HELP           
            break;
        case ESvgtDlgFindTextCommand:
            break;
        case ESvgtDlgViewDetailsCommand:
            break;
        case ESvgtDlgListDownloadCommand:    
            {          
            if ( iAppObserver )
                {
                iAppObserver->DisplayDownloadMenuL();
                }                           
            break;              
            }
        case ESvgtDlgFullScreenCommand:
            {
            if ( iCustControl )
                {
                iCustControl->SetCurrentFullScreenStatus( ETrue );
                SwitchDialogLayoutL( ETrue ); // ETrue = Full Screen Mode
                // Redraw Status Pane and CBA by doing a draw on dialog
                DrawNow();
                }
            break;
            }
        case ESvgtDlgNormalScreenCommand:
            {
            if ( iCustControl )
                {
                iCustControl->SetCurrentFullScreenStatus( EFalse );            
                SwitchDialogLayoutL( EFalse ); // EFalse = Normal Screen Mode
                // Redraw Status Pane and CBA by doing a draw on dialog
                DrawNow();                
                }
            break;
            }
//@@ Touch support
#ifdef RD_SVGT_AUDIO_SUPPORT
        case ESvgtDlgMuteCommand:
            iIsVolumeMuted = !iIsVolumeMuted;
            iCustControl->SetPresentationVolume(0);

            if(iCustControl->IsContentFinite())
                SetNaviPaneInfoL(iIsVolumeMuted, *iProgressText);
            else
                SetNaviPaneInfoL(iIsVolumeMuted, KNullDesC);
            break;
            
        case ESvgtDlgUnmuteCommand:
            iIsVolumeMuted = !iIsVolumeMuted;
            // Set the volume level to the value it was before muting
            iCustControl->SetPresentationVolume(iCustControl->GetPrevVolume());
            
            if(iCustControl->IsContentFinite())
                SetNaviPaneInfoL(iIsVolumeMuted, *iProgressText);
            else
                SetNaviPaneInfoL(iIsVolumeMuted, KNullDesC);
            break;            
        case ESvgtDlgSetVolumeCommand:       // Fall Through
#endif //RD_SVGT_AUDIO_SUPPORT        
//@@ Touch support
        case ESvgtDlgLoopOnCommand:     // Fall Through
        case ESvgtDlgLoopOffCommand:    // Fall Through
        case ESvgtDlgPlayCommand:       // Fall Through
        case ESvgtDlgPauseCommand:      // Fall Through
        case ESvgtDlgStopCommand:       // Fall Through
        case ESvgtDlgZoomInCommand:     // Fall Through
        case ESvgtDlgZoomOutCommand:    // Fall Through
        case ESvgtDlgAngle90:           // Fall Through
        case ESvgtDlgAngle180:          // Fall Through
        case ESvgtDlgAngle270:          // Fall Through
        case ESvgtDlgSelectTextCommand: // Fall Through
        case ESvgtDlgEditTextCommand:   // Fall Through
            break;
        case EAknSoftkeyBack: // Fall Through
        case EEikCmdExit:
            // Close this dialog
            procCmdReqd = EFalse; // No Further Processing required.
            TryExitL( aCommandId );
            break;
        case EAknCmdExit:          
            // Close this dialog
            procCmdReqd = EFalse; // No Further Processing required.
            if ( !SaveNeededL( aCommandId ) )
                {
                TryExitL( aCommandId );  
                iAvkonAppUi->ProcessCommandL( EAknCmdExit );                
                }
            break;
        default:
            procCmdReqd = EFalse;
            break;
        }
    // If custom control has been created and control needs
    // to process the command invoke the control's ProcessViewerCommandL()
    // function
    if ( procCmdReqd && iCustControl )
        {
        iCustControl->ProcessViewerCommandL( aCommandId );
        }
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::CreateCustomControlL
// Reimplements CAknDialog::CreateCustomControlL inorder to add the SVGT
// custom control as part of the dialog in the UI framework.
// Returns: SEikControlInfo which contains the SVGT Cust Control, if
//                          aControlType == ESVGTCustControl
//                     else,
//                           the structure does not contain any control.
// -----------------------------------------------------------------------------
//
SEikControlInfo CSVGTUIDialog::CreateCustomControlL(
        TInt aControlType ) // Control Type
    {
    SEikControlInfo controlInfo;
    controlInfo.iControl = NULL;
    controlInfo.iTrailerTextId = 0;
    controlInfo.iFlags = EEikControlHasEars;//EEikControlIsNonFocusing;

    switch ( aControlType )
        {
        case ESVGTCustControl:
            iCustControl = CSVGTCustControl::NewL(
                            *iFileHandlePtr,
                            iAppObserver,
                            this,
                            this, 
                            this, 
                            ETrue,    // Progressive rendering enabled.
                                      // To enable it pass ETrue 
                                      // otherwise EFalse.
                            NULL,
                            ESvgDisplayIcons|
                            ESvgDrawPanIndicator|
                            ESvgDrawPointer |
                            ESvgDisplayErrorNotes |
                            ESvgDrawPauseIndicator | 
                            ESvgUseBackLight
                            ); 
            iCustControl->AddListener(this,EAudioListener);
            controlInfo.iControl = iCustControl;
            controlInfo.iControl->SetContainerWindowL(*this);                
            iCustControl->SetPointerObserver(this);
            break;
        default:
			return CAknDialog::CreateCustomControlL(aControlType);
        }
    return controlInfo;
    }


// -----------------------------------------------------------------------------
// CSVGTUIDialog::SizeChanged
// Reimplements CAknDialog::SizeChanged inorder to support the resize of the
// dialog when functions such as SetRect, SetExtent are called on the dialog.
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::SizeChanged()
    {
    CAknDialog::SizeChanged();
    iCustControl->SetExtent( Rect().iTl, Rect().Size() );
    AknLayoutUtils::LayoutMetricsPosition(AknLayoutUtils::EControlPane, iBtnGrpPos);
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EControlPane, iBtnGrpSize);
    DrawNow();
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::OfferKeyEventL
// Handles Key events by reimplementing CCoeControl::OfferKeyEventL.
// Returns:
//      EKeyWasConsumed: If this control uses this key.
//      EKeyWasNotConsumed: Otherwise.
// -----------------------------------------------------------------------------
//
TKeyResponse CSVGTUIDialog::OfferKeyEventL(
    const TKeyEvent& aKeyEvent, // Key Event
    TEventCode aType)           // Event Code
    {
    if ( iCustControl )
        {
        if ( aType == EEventKey )
            {
            if ( aKeyEvent.iScanCode == EStdKeyEnter  )
                {
                TryExitL( iLastCommandID );
                
                return iCustControl->OfferKeyEventL( aKeyEvent, aType );
                }
            
            if ( aKeyEvent.iCode == KSvgScreenToggleKey && 
                 aKeyEvent.iRepeats == 0 ) // Ignore Repeat Events
                {
                // Screen Mode Change Handling
                TBool isFullScreenOn;
                iCustControl->GetCurrentFullScreenStatus( isFullScreenOn );
                if ( isFullScreenOn )
                    {
                    // Toggle to Normal Screen
                    ProcessCommandL( ESvgtDlgNormalScreenCommand );
                    }
                    else
                        {
                        // Toggle to Full Screen
                        ProcessCommandL( ESvgtDlgFullScreenCommand );
                        }
                // No further processing required.
                return EKeyWasConsumed;
                }
            }
        // Pass the key to the custom control
        TKeyResponse lCcRetVal = iCustControl->OfferKeyEventL( aKeyEvent, 
                                                               aType );
        if ( lCcRetVal == EKeyWasConsumed )
            {
            return EKeyWasConsumed;
            }
        }
    return CAknDialog::OfferKeyEventL( aKeyEvent, aType );        
    }

#ifdef __SERIES60_HELP
// -----------------------------------------------------------------------------
// CSVGTUIDialog::OfferKeyEventL
// Handles Key events by reimplementing CCoeControl::OfferKeyEventL.
// Returns:
//      EKeyWasConsumed: If this control uses this key.
//      EKeyWasNotConsumed: Otherwise.
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KSvgtHelpContextUid;    
    aContext.iContext = KSVGT_HLP_VIEWER;
    }
#endif // __SERIES60_HELP

// -----------------------------------------------------------------------------
// CSVGTUIDialog::HandleResourceChange
// Handles change of skin/Layout
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::HandleResourceChange( TInt aType )
    {
    CAknDialog::HandleResourceChange( aType );
    if( iQuietExitWhenOrientationChange )
        {
           TryExitL( EAknCmdExit );  
           return;
        }
    if ( aType == KAknsMessageSkinChange )
        {
        // No skin related action performed.
        }
        else if ( aType == KEikDynamicLayoutVariantSwitch )
            {
            if ( iCustControl )
                {
                TBool isFullScreenOn;
                // Check whether the custom control is using full-screen or
                // normal screen
                iCustControl->GetCurrentFullScreenStatus( isFullScreenOn );                
                // Change the layout to reflect layout switch
                TRAP_IGNORE( SwitchDialogLayoutL( isFullScreenOn ) );
                }
            }

    // If it is a Layout switch, need to do a redraw
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        // Redraw the dialog
        DrawDeferred();
        }
    }
// -----------------------------------------------------------------------------
// CSVGTUIDialog::DrawProgressBarL
// This function updates the progress bar in the navi pane. It is called back 
// by the custom control.
// -----------------------------------------------------------------------------
//
void CSVGTUIDialog::DrawProgressBarL( TUint aTimeElapsed, // Time Elapsed in 
                                                          // Seconds
    TUint aTimeTotal ) // Total time in seconds
    {
        
    TBuf<KMaxTimeTextLength> timeElapsedBuf; // Time Elapsed, e.g. "00:12"
    TBuf<KMaxTimeTextLength> totalTimeBuf; // Total duration, e.g. "00:55"
    
    // Format the time to user readable format. The format is locale dependent
    FormatTimeL( aTimeElapsed, timeElapsedBuf );  
    FormatTimeL( aTimeTotal, totalTimeBuf );  

    // Format label text based on resource
    CDesCArrayFlat* strings = new (ELeave) CDesCArrayFlat( 2 ); // Allocate 2 
                                                                // strings for
                                                                // elapsed time
                                                                // and total 
                                                                // time strings
    CleanupStack::PushL( strings );
    strings->AppendL( timeElapsedBuf );
    strings->AppendL( totalTimeBuf );

    delete iProgressText;
    iProgressText = NULL;
    iProgressText = StringLoader::LoadL( R_QTN_SVGT_TIME_INDIC, 
                                      *strings, 
                                      iEikonEnv );
    // Destroy the array of strings
    CleanupStack::PopAndDestroy( strings );
    
    SetNaviPaneInfoL( iIsVolumeMuted, *iProgressText);
    }    

void CSVGTUIDialog::HandleApplicationExitL( TInt aExitCommand )
    {
    if((aExitCommand == EAknSoftkeyBack) || ( aExitCommand == EEikCmdExit))
        {
        this->TryExitL(aExitCommand);
        }
    else
        {
        User::LeaveIfError(KErrArgument);
        }
    }

EXPORT_C void CSVGTUIDialog::AssignImageData( const TDesC& aUri, HBufC8* aData )
    {
    iCustControl->AssignImageData(aUri, aData);
    }

// -----------------------------------------------------------------------------
// CSVGTUIDialog::SetNaviPaneInfoL
// Sets the mute icon & text to the navi pane.
// -----------------------------------------------------------------------------
void CSVGTUIDialog::SetNaviPaneInfoL(TBool aIsVolumeMuted, const TDesC& aText)
    {
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    CAknNavigationControlContainer* np = 
            static_cast<CAknNavigationControlContainer*>
            ( sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) );
    // Save the old Navi Decorator so that it can be deleted after 
    // creating the new navi decorator 
    CAknNavigationDecorator* lOldNaviDecorator = iNaviDecorator;
            
    iNaviDecorator = NULL;

    CSvgtDecoratorControl* decoControl = new (ELeave) CSvgtDecoratorControl;
    CleanupStack::PushL(decoControl);
    decoControl->ConstructL();
    decoControl->ShowVolumeMutedIcon(aIsVolumeMuted);
    decoControl->SetTextL(aText);
    iNaviDecorator = CAknNavigationDecorator::NewL(np, decoControl);
    iNaviDecorator->SetContainerWindowL(*this);
    CleanupStack::Pop(decoControl);
        
    // Clear the old Navi-Pane Decorator
    if ( lOldNaviDecorator )
        {
        delete lOldNaviDecorator ;
        lOldNaviDecorator = NULL;
        }
    // Push the new Navi decorator in the Navigation Pane
    np->PushL( *iNaviDecorator );
    }    
    
void CSVGTUIDialog::VolumeMuted(TBool aVolumeMuted)
    {
    if(iCustControl->IsContentFinite())
        {
        TRAP_IGNORE( SetNaviPaneInfoL(aVolumeMuted, *iProgressText));
        }
    else
        {
        TRAP_IGNORE(SetNaviPaneInfoL(aVolumeMuted, KNullDesC));
        }
    iIsVolumeMuted = aVolumeMuted;
    }
// End of File
