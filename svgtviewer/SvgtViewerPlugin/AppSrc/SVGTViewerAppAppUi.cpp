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
* Description:  AppUI for SVGT Viewer App.
*
*/


#include <eikapp.h>
#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <w32std.h>                                        // RWsSession 
#include <e32std.h> 
#include <ctsydomainpskeys.h>                              // For call handling
#include <e32property.h>
#include <StringLoader.h>
#include <coeutils.h>
#include <svgtviewerapp.rsg>
#include <SVGTAppObserverUtil.h>
#include "SVGTViewerAppDbgFlags.hrh"
#include "SVGTViewerAppAppUi.h"
#include "SVGTViewerAppDocument.h"
#include "SVGTViewerApp.pan"
#include "SVGTUIDialog.h"
#include "SVGTViewerApp.hrh"


#include "SvgtViewerAppView.h"

#ifdef RD_MULTIPLE_DRIVE    
#include <pathinfo.h>
#include <driveinfo.h>
#endif
// Constants

#ifdef SVGTVIEWERAPP_DBG_OPEN_HARDCODE_FILE
_LIT(KDummyTstFileName, "c:\\data\\images\\01-guide.svg");
#endif

// Constant for periodic timer
const TInt KOneMicroSec = 1;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::CSVGTViewerAppAppUi
// Default Constructor for CSVGTViewerAppAppUi.
// -----------------------------------------------------------------------------
//
CSVGTViewerAppAppUi::CSVGTViewerAppAppUi()             
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::ConstructL
// Second Phase constructor for CSVGTViewerAppAppUi.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::ConstructL()
    {
    BaseConstructL( EAknEnableSkin|EAknEnableMSK );    // Enable skin support and MSK label 
    												  // for this Application    												
                                   
    ResetEmbeddedAppRunning(); //By default, there are no embedded app started.    

    iContainer = new (ELeave) CSvgtViewerAppView;
    iContainer->ConstructL(ClientRect());
	// add container to stack; enables key event handling.
	AddToStackL(iContainer);
    
#ifndef SVGTVIEWERAPP_DBG_OPEN_HARDCODE_FILE    
    // Launch file only in non-hardcoded option.
    // Start an idle timer to start the dialog synchronously.
    iIdleTimer = CPeriodic::NewL(CActive::EPriorityIdle);
    // Using the periodic timer so that active object for
    // open file service gets a chance to execute
    iIdleTimer->Start(KOneMicroSec, KOneMicroSec, 
        TCallBack(CallBackFuncL,this));
#endif    
    }
    
// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
CSVGTViewerAppAppUi::~CSVGTViewerAppAppUi()
    {
    iAppDialog = NULL; 
    iAppObserver = NULL;      
    if ( iIdleTimer )
        {
        delete iIdleTimer;
        iIdleTimer = NULL;
        }

	RemoveFromStack(iContainer);
    delete iContainer;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::HandleCommandL
// Reimplements CAknAppUi::HandleCommandL inorder to handle commands.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::HandleCommandL( TInt aCommand )
    {
#ifdef SVGTVIEWERAPP_DBG_OPEN_HARDCODE_FILE        
    TFileName fileName;     
#endif    
    switch ( aCommand )
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            break;
        case ESVGTViewerAppCommand1:
#ifdef _DEBUG
            iEikonEnv->InfoMsg( _L( "not implemented" ) );
#endif
#ifdef SVGTVIEWERAPP_DBG_OPEN_HARDCODE_FILE            
            fileName.Append( KDummyTstFileName );
            OpenFileL( fileName );
#endif        
            break;

        default:           
            break;
        }
    }





// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::OpenFileL
// Reimplements CAknAppUi::OpenFileL inorder to provide for opening SVG 
// content through doc handler.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::OpenFileL( 
    RFile& aFileHandle ) // File Handle to 
                         // SVGT content
    {
    // Display an info note if a video call is in progress.
    // SVG content cannot be displayed in this case.    
    if ( IsVideoCall() )
        {
        HBufC* text = StringLoader::LoadLC( 
                R_SVGT_VIEWER_INFO_VIDEO_CALL_ONGOING );
        CAknInformationNote* dlg = new (ELeave) CAknInformationNote( ETrue );
        dlg->ExecuteLD( text->Des() );
        CleanupStack::PopAndDestroy( text );  // text
        Exit();
        }
    
    TRAPD( err, iAppDialog = CSVGTUIDialog::NewL() );
    
    if ( err != KErrNone )
        {
        // Reset the pointer to dialog
        iAppDialog = NULL;
        iEikonEnv->HandleError( err );
        Exit();
        }

    // Get the File's fullpath, Handle and pass it to App Observer Util. 
    TFileName filename;
    CSVGTViewerAppDocument *appDocument = 
            static_cast< CSVGTViewerAppDocument* >(iDocument);

    if ( appDocument->IsFileFullPathAvailable() )
        {
        appDocument->GetFileFullPath(filename);
        }

    TRAP( err, iAppObserver = CSVGTAppObserverUtil::NewL( iAppDialog,
                                     &aFileHandle, 
                                     filename,
                                     appDocument->IsFileFullPathAvailable(),
                                     CanShowSave(),
                                     ShouldMoveContent()) );
    if ( err != KErrNone )
        {
        // Delete the dialog
        delete iAppDialog;
        // Reset the dialog pointer
        iAppDialog = NULL;
        // Reset the App Observer Pointer
        iAppObserver = NULL;
        iEikonEnv->HandleError( err );
        Exit();
        }
    
    if( iAppDialog )
        {
        TRAP( err, iAppDialog->ExecuteLD( aFileHandle, iAppObserver ) );
        if ( err != KErrNone )
            {
            if( err == KLeaveExit )
                {
            	delete iAppDialog;
            	delete iAppObserver;
            	
            	iAppObserver = NULL;
            	iAppDialog   = NULL;
            	
            	Exit();
                }
            // Reset the dialog pointer
            iAppDialog = NULL;
            // Delete the app observer 
            delete iAppObserver;
            iEikonEnv->HandleError( err );
            Exit();
            }
        }
    // Delete the app observer
    delete iAppObserver;
    
    // Reset the App Observer Pointer
    iAppObserver = NULL;
    
    // Dialog has destroyed itself, Reset the pointer
    iAppDialog = NULL;
    
    Exit();
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::OpenFileL
// Reimplements CAknAppUi::OpenFileL inorder to provide for opening SVG 
// content through doc handler.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::OpenFileL( 
    const TDesC& aFileName) // File Name of SVGT content
    {
    // Display an info note if a video call is in progress.
    // SVG content cannot be displayed in this case.
    if ( IsVideoCall() )
        {
        HBufC* text = StringLoader::LoadLC( 
                R_SVGT_VIEWER_INFO_VIDEO_CALL_ONGOING );
        CAknInformationNote* dlg = new (ELeave) CAknInformationNote( ETrue );
        dlg->ExecuteLD( text->Des() );
        CleanupStack::PopAndDestroy( text );  // text
        Exit();
        }
    
    // Open File Handle for the file

    RFile fileHandle;
    TInt err = fileHandle.Open( iEikonEnv->FsSession(), 
                                aFileName, EFileShareAny );
    if ( err != KErrNone )
        {       
        iEikonEnv->HandleError( err );
        Exit();
        }


    TRAP( err, iAppDialog = CSVGTUIDialog::NewL() );
    if ( err != KErrNone )
        {
        // Reset the Dialog pointer
        iAppDialog = NULL;
        // Close the file handle
        fileHandle.Close();

        iEikonEnv->HandleError( err );
        Exit();
        }
    
    TRAP( err, 
        iAppObserver = CSVGTAppObserverUtil::NewL( iAppDialog, 
                                     &fileHandle, 
                                     aFileName,
                                     ETrue,
                                     CanShowSave(),
                                     ShouldMoveContent() ) );
    if ( err != KErrNone )
        {
        // Reset the AppObserver Pointer
        iAppObserver = NULL;
        // Reset the Dialog pointer
        iAppDialog = NULL;
        // Close the file handle
        fileHandle.Close();
        
        iEikonEnv->HandleError( err );
        Exit();
        }
        
    if( iAppDialog )
        {
        TRAP( err, iAppDialog->ExecuteLD( fileHandle, iAppObserver ) );

        if ( err != KErrNone )
            {
            if( err == KLeaveExit )
                {
                delete iAppDialog;
                delete iAppObserver;
            	
                iAppObserver = NULL;
                iAppDialog   = NULL;
            	
                Exit();
                }
            // Delete the AppObserver Pointer
            delete iAppObserver;
            // Reset the AppObserver Pointer
            iAppObserver = NULL;
            // Reset the Dialog pointer
            iAppDialog = NULL;
            // Close the file handle
            fileHandle.Close();
        
            iEikonEnv->HandleError( err );
            Exit();
            }
        }
    
    // Delete the App Observer
    delete iAppObserver;
   
    // Reset the App Observer pointer
    iAppObserver = NULL;

    // Dialog has destroyed itself, Reset the pointer
    iAppDialog = NULL;
    
    // Close the file handle and session
    fileHandle.Close();
    
    Exit();
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::SetAllowMove
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::SetAllowMove(TInt32 aMoveAllowed)
    {
    iMoveAllowed = aMoveAllowed;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::ProcessCommandParametersL
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::ProcessCommandParametersL(
    TApaCommand /* aCommand */ ,
    TFileName& aDocumentName, 
    const TDesC8& /* aTail */ )
    {
    return ConeUtils::FileExists(aDocumentName);
    }    

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::HandleScreenDeviceChangedL
// Handle the screen size being changed
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::HandleScreenDeviceChangedL()
    {
    CAknAppUiBase::HandleScreenDeviceChangedL();  
    }
// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::HandleResourceChangeL
// Handle the screen size being changed
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::HandleResourceChangeL( TInt aType )
    {
    CAknAppUi::HandleResourceChangeL( aType );
    }
// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::HandleForegroundEventL
// App Framework calls this api to give background/foreground events to the
// application. Param: aForeground : ETrue: Foreground. EFalse: Background.
// -----------------------------------------------------------------------------
//

void CSVGTViewerAppAppUi::HandleForegroundEventL( TBool aForeground )
    {   
    // Pass the application foreground or goes to background,
    // give the events to the underlying dialog and the custom
    // control so that the animation is paused when app goes
    // to background and so on. But the app might have
    // opened another application embedded (Browser or another
    // viewer). So need to qualify these events with these
    // conditions. Whenever opening/closing another app 
    // embedded, need to pass background/foreground events to
    // dialog, and call Set/ResetEmbeddedAppRunning.

    // Passing the event to the base class
    CAknAppUi::HandleForegroundEventL( aForeground );
    
    if ( !IsEmbeddedAppRunning() )
        {
        if ( iAppDialog )
            {
            iAppDialog->HandleApplicationForegroundEvent(
                            aForeground );
            }

        }
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::IsVideoCall()
// Used to find out if the videocall is going on. The
// viewer is not allowed to open if viedo call is 
// going on.
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::IsVideoCall() const
    {
    TBool lVideoCallConnected = EFalse;
    TInt lType = 0;  
    TInt error = RProperty::Get( KPSUidCtsyCallInformation, 
                                 KCTsyCallType, lType );
    
    // EPSTelephonyCallTypeH324Multimedia is used for 3G video calls
    if ( !error && lType == EPSCTsyCallTypeH324Multimedia )
        {
        // Call is of video call, check whether call is connected
        TInt lState = 0;         
        error = RProperty::Get( KPSUidCtsyCallInformation, 
                                KCTsyCallState, lState );
        if ( !error &&
         ( lState != EPSCTsyCallStateUninitialized &&
           lState != EPSCTsyCallStateNone &&
           lState != EPSCTsyCallStateDisconnecting ) )
            {
            lVideoCallConnected = ETrue;    
            }
        }
    return lVideoCallConnected;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::IsEmbeddedAppRunning()
// Used to find out if viewer application has in turn 
// started an embedded application.
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::IsEmbeddedAppRunning() const
    {
    return iEmbeddedAppRunning;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::SetEmbeddedAppRunning()
// Used to set state in Viewer application if it has in turn 
// started an embedded application.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::SetEmbeddedAppRunning()
    {
    iEmbeddedAppRunning = ETrue;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::ResetEmbeddedAppRunning()
// Used to set state in viewer application if embedded
// application started by it is closed now.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppAppUi::ResetEmbeddedAppRunning()
    {
    iEmbeddedAppRunning = EFalse;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::CallBackFuncL()
// Used to callback by the idle timer in order to launch 
// the dialog
// -----------------------------------------------------------------------------
//
TInt CSVGTViewerAppAppUi::CallBackFuncL(TAny *aPtr)
    {
    CSVGTViewerAppAppUi* appUi = 
        static_cast< CSVGTViewerAppAppUi* >( aPtr );
    if ( appUi )    
        {
        return ( appUi->LaunchDialogL() );    
        }
        else
            {
            // To indicate that the timer is not
            // called again, return false to the timer
            // RunL
            return ( EFalse );
            }
    }
    
// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::LaunchDialogL()
// This function accesses the document and launches the 
// content.
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::LaunchDialogL()
    {
    // Check if file-handle in document is valid
    if ( static_cast< CSVGTViewerAppDocument* >
        (iDocument)->IsFileHandleValid() )
        {
        // Obtain the file-handle
        RFile& fileHandle = 
        (static_cast< CSVGTViewerAppDocument* >
        (iDocument)->GetSvgtContentFileHandle());
        
        // Cancel the periodic timer
        if ( iIdleTimer->IsActive() )
            {
            iIdleTimer->Cancel();
            }
            
        // Launch the content
        OpenFileL( fileHandle );
 
        // Delete and reset the idle-timer
        delete iIdleTimer;
        iIdleTimer = NULL;  
 
        // Return False to indicate that 
        // timer need not callback further.
        return EFalse; 
        
        }
        else
            {
            // Return True to indicate that the 
            // timer needs to callback this function 
            // again, as the file-handle is not valid 
            // yet.
            return ETrue;           
            }
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::WeAreEmbedded
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::WeAreEmbedded() const
{
   // SVGT-Viewer will be opened in embedded mode always
   return ETrue;       
}

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::CanShowSave
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::CanShowSave() const 
    {
    // By default dont show the save option
    TBool ret = EFalse;
    
    // Check if we are launched embedded
    if (WeAreEmbedded())
        {
        // If the file is not a local file then 
        // display the save option
        if ( !IsLocalFile() )
            {
            ret = ETrue;
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::ShouldMoveContent
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::ShouldMoveContent() const
    {
    return iMoveAllowed;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::IsLocalFile
//  This function returns ETrue if file is local, EFalse otherwise
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::IsLocalFile() const 
    {
    // By default file is not local
    TBool isLocal = EFalse;
   
    // Obtain the file-handle
    RFile& fileHandle = (static_cast<CSVGTViewerAppDocument*>
                        (iDocument)->GetSvgtContentFileHandle());
    // Check whether local/remote  
    TRAP_IGNORE( isLocal = IsLocalFileL( fileHandle ) );
    
    return isLocal;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppAppUi::IsLocalFileL
//  This function checks whether the filepath of the content opened is 
//  in the PHONE_MEMORY_IMAGES_PATH or MEMORYCARD_IMAGES_PATH
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppAppUi::IsLocalFileL( RFile& aFileHandle ) const 
    {
    // By default file is not local
    TBool retVal = EFalse;

#ifdef RD_MULTIPLE_DRIVE    

    HBufC* fileNameBuf = HBufC::NewLC( KMaxFileName );
    TPtr fileNamePtr = fileNameBuf->Des();
    aFileHandle.FullName( fileNamePtr );

    RFs fsSession = iEikonEnv->FsSession();
    TName driveRootPath;
     
    TInt intDrive;
    DriveInfo::TDriveArray driveArray;
    User::LeaveIfError( DriveInfo::GetUserVisibleDrives( fsSession, driveArray ) );
    for(TInt i=0; i < driveArray.Count(); i++)
        {
        TChar driveLetter = driveArray.LetterAt(i);
        User::LeaveIfError( RFs::CharToDrive(driveLetter, intDrive) );
        User::LeaveIfError( PathInfo::GetRootPath(driveRootPath, intDrive) );

        // Check whether filepath consists of phone mem path or mmc path
        if ( driveRootPath.FindF( fileNamePtr.Left(driveRootPath.Length() ) ) != KErrNotFound )
            {
            // Path Found means file is local
            retVal = ETrue;
            break;
            }
        }

    CleanupStack::PopAndDestroy( fileNameBuf );    
#else
    // Get the phone memory path
    HBufC* phoneMemImgPath = iEikonEnv->AllocReadResourceLC( 
            R_SVGT_VIEWER_PHONE_MEMORY_IMAGES_PATH );

    // Get the Memory Card path
    HBufC* memCardImgPath = iEikonEnv->AllocReadResourceLC(
            R_SVGT_VIEWER_MEMORYCARD_IMAGES_PATH );
    
    // Get the content filename
    HBufC* fileNameBuf = HBufC::NewLC( KMaxFileName );
    TPtr fileNamePtr = fileNameBuf->Des();
    aFileHandle.FullName( fileNamePtr );

    // Check whether filepath consists of phone mem path or mmc path
    if ( phoneMemImgPath->FindF( fileNamePtr.Left( 
            phoneMemImgPath->Length() ) ) != KErrNotFound
         || memCardImgPath->FindF( fileNamePtr.Left( 
            memCardImgPath->Length() ) ) != KErrNotFound )
        {
        // Path Found means file is local
        retVal = ETrue;
        }

    // Perform Cleanup         
    CleanupStack::PopAndDestroy( fileNameBuf );
    CleanupStack::PopAndDestroy( memCardImgPath );
    CleanupStack::PopAndDestroy( phoneMemImgPath );
#endif    

    return retVal;
    }
//End of File
