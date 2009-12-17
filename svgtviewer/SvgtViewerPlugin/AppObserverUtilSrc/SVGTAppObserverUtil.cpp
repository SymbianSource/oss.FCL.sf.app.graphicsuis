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
* Description:  This class provides helper functions and a reference 
*                implementation for hyperlink handling, image fetching
*                and saving of the svg content. 
*
*/


#include <BrowserLauncher.h>
#include <DocumentHandler.h>
#include <uri16.h>
#include <uriutils.h>
#include <eikenv.h>
#include <eikappui.h>
#include <eikapp.h>
#include <eikdef.h>
#include <flogger.h>
#include <CDownloadMgrUiUserInteractions.h>
#include <CDownloadMgrUiDownloadsList.h>
#include <CDownloadMgrUiLibRegistry.h>
#include <BrowserOverriddenSettings.h>

#ifdef __SERIES60_NATIVE_BROWSER
#include <BrowserUiSDKCRKeys.h>    // Include only if s60 native 
                                   // browser available
#endif

#include <centralrepository.h> 
#include <apgtask.h>                                      // TApaTaskList
#include <apmstd.h>                                       // RApaLsSession
#include <commdb.h>
#include <ApDataHandler.h>                                // CApDataHandler
#include <ApAccessPointItem.h>                            // CApAccessPointItem
#include <ApUtils.h>                                      // CApUtils
#include <ApSelect.h>                                     // CApSelect 
#include <aknnotewrappers.h>
#include <AknGlobalNote.h>
#include <AknWaitDialog.h>
#include <AknCommonDialogs.h>
#include <AknProgressDialog.h>
#include <StringLoader.h>
#include <data_caging_path_literals.hrh>
#include <bautils.h>
#include <eikprogi.h>
#include <e32svr.h>
#include <SVGTAppObserverUtil.rsg>
#include <SVGTAppObserverUtil.h>
#include <pathinfo.h>
#include <sysutil.h>
#include <imcvcodc.h>   //TImCodecB64
#include <AiwGenericParam.h>

#include "SVGTViewerAppDbgFlags.hrh"
#include "SVGTViewerAppDownloads.h"
#include "SVGTAppObserver.h"                              // temp
#include "SVGTUIDialog.h"                                 // AppDialog
#include "SVGTUIDialog.hrh"

#ifdef RD_MULTIPLE_DRIVE    
#include <AknCommonDialogsDynMem.h>
#endif
// CONSTANTS 

_LIT( KBrowserParameter, "4" ); // magic number for browser to open the URL
_LIT( KSpace, " " );

_LIT(KJpg,   ".jpg" );
_LIT(KJpeg,  ".jpeg" );
_LIT(KPng,   ".png" );
_LIT(KBmp,   ".bmp" );
_LIT(KSvg,   ".svg" );
_LIT(KSvgz,  ".svgz" );

#ifndef __SERIES60_NATIVE_BROWSER
    // Below constants need to be used if s60 native browser is not used.
    const TUid KCRUidBrowser   = {0x10008D39};
    const TUint32 KBrowserDefaultAccessPoint =  0x0000000E;
#endif // __S60_REMOVE_BROWSER

// SMIL Fit Value
_LIT( KSvgtDefSmilFitValue,"meet" );
// Resource file for App Observer Util
_LIT( KResourceFileName, "Z:SVGTAppObserverUtil.rsc" );

_LIT( KEmptyString, "" );
_LIT( KWww, "www" );
_LIT( KHttp, "http://");
_LIT( KDotDot, ".." );
_LIT( KBSlashStr, "/" );
_LIT( KSlashStr, "\\" );
_LIT( KDotSlashStr, ".\\" );
_LIT( KColonStr, ":" );
_LIT( KNewLineStr, "\n" );

#ifdef SVGTVIEWERAPP_DBG_FLAG
_LIT( KFileLoggingDir, "SVGTViewer" );
_LIT( KFileLog, "SaveLog.txt" );
#endif

// Maximum length of thread Id in Characters
const TInt KSvgMaxThreadIdStr = 10;

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CSVGTAppObserverUtil* CSVGTAppObserverUtil::NewL( 
                        CSVGTUIDialog* aAppDialog,
                        RFile* aFileHdlPtr,
                        const TDesC& aFilename, 
                        TBool aIsFileFullPathAvailable,
                        TBool aCanSaveContent,
                        TBool aShouldMoveContent)
    {
    CSVGTAppObserverUtil* self = 
                             CSVGTAppObserverUtil::NewLC( aAppDialog,
                                        aFileHdlPtr,
                                        aFilename, 
                                        aIsFileFullPathAvailable,
                                        aCanSaveContent,
                                        aShouldMoveContent );
    CleanupStack::Pop( self );
    return self;   
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSVGTAppObserverUtil* CSVGTAppObserverUtil::NewLC( CSVGTUIDialog* aAppDialog,
                                            RFile* aFileHdlPtr,
                                            const TDesC& aFilename, 
                                            TBool aIsFileFullPathAvailable,
                                            TBool aCanSaveContent,
                                            TBool aShouldMoveContent)
    {
    CSVGTAppObserverUtil* self = 
                          new( ELeave ) CSVGTAppObserverUtil( aAppDialog );
    CleanupStack::PushL( self );
    self->ConstructL( aFileHdlPtr, aFilename, aIsFileFullPathAvailable, 
            aCanSaveContent, aShouldMoveContent );
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CSVGTAppObserverUtil* CSVGTAppObserverUtil::NewL( 
                        CSVGTUIDialog* aAppDialog,
                        RFile* aFileHdlPtr )
    {
    CSVGTAppObserverUtil* self = 
                             CSVGTAppObserverUtil::NewLC( aAppDialog,
                                        aFileHdlPtr );
    CleanupStack::Pop( self );
    return self;   
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSVGTAppObserverUtil* CSVGTAppObserverUtil::NewLC( 
                                            CSVGTUIDialog* aAppDialog,
                                            RFile* aFileHdlPtr )
    {
    CSVGTAppObserverUtil* self = 
                          new( ELeave ) CSVGTAppObserverUtil( aAppDialog );
    CleanupStack::PushL( self );
    
    self->ConstructL( aFileHdlPtr );
    return self;
    }

// -----------------------------------------------------------------------------    
// Destructor
// -----------------------------------------------------------------------------
EXPORT_C CSVGTAppObserverUtil::~CSVGTAppObserverUtil()
    {  
    
#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED         

    CSVGTViewerAppDownloads* downloads;
    
    iDownloadIter.SetToFirst();
    while ((downloads = iDownloadIter++) != NULL)
        {
        downloads->QueLink().Deque();
        delete downloads;
        iDownloadIter.SetToFirst();
        }
    
    // Close the DMgr UI Library first, as it uses the same 
    // Download Manager session that the Viewer uses
    delete iDMgrUiReg;
    
    // Close the Download Manager connection
    iDownloadMgr.Close();

    iDMgrDownloadsList = NULL;
    iDMgrUserInteractions = NULL;  
#endif     
    // Reset the app dialog, it is already freed in OpenFileL
    iAppDialog = NULL;     

    delete iFileManager;
    delete iSvgFileName;
    delete iSvgDstFileName;
    delete iDocHandler;
    
    delete iWaitNote;
    delete iTempFileName;
    if ( iResourceOffset != 0 )
        {
        iEikEnv->DeleteResourceFile( iResourceOffset );
        }
        
    // Reset the SVG File Handle as it is owned by the caller
    iSvgFileId = NULL;
    // Reset the Progress Info implementor pointer
    iProgressInfo = NULL;
    // Reset Environment pointer
    iEikEnv = NULL;
    }
    
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::FetchImage
// Called by Svg engine when there is an image tag encountered in the svg file
// parameters:
// aUri: the uri that needs to be downloaded or fetched locally
// aSession: File Session in which the file is opened.
// aFileHandle: handle of the downloaded file or locally fetched file.
// return value:
// KErrNone if the file is fetched successfully
// KErrNotFound if the file could not be fetched.
// -----------------------------------------------------------------------------
//   
EXPORT_C TInt CSVGTAppObserverUtil::FetchImage( 
                          const TDesC&  aUri , RFs&  aSession , 
                          RFile&  aFileHandle  )
    {
    TInt ret = KErrNone;
    
    TDownloadNeeded downloadNeeded = ELocalImageFile;
    TRAPD( error, downloadNeeded = IsDownloadNeededL( aUri ) );
    
    if ( error != KErrNone )
        {
        return KErrNotFound;
        }
    
    if ( downloadNeeded == ELocalImageFile )
        {
        TFileName fileName;
        if ( GetLocalFile( aUri, fileName, *iSvgFileName ) )
            {
            return aFileHandle.Open( aSession, fileName, EFileShareAny );            
            }
        else
            {
            return KErrNotFound;
            }        
        }
    else if ( downloadNeeded == ERemoteImageFile )
        {
        TRAPD( err,ret = StartDownloadingImageL( aUri, 
                                               aSession,
                                               aFileHandle,
                                               EFalse,
                                               EFalse /* Asynchronous mode */) ); 
        if ( err != KErrNone )
            {
            return KErrNotFound;
            }
        else
            {
            return ret;         
            }
        
        }
    else
        {
        return KErrNotFound;
        }   
    
    }

TInt CSVGTAppObserverUtil::FetchFont( const TDesC& /* aUri */, 
                                    RFs& /* aSession */, RFile& /* aFileHandle */ )
    {
    return KErrNotSupported;
    }

// ---------------------------------------------------------------------------
// CSVGTAppObserverUtil::LinkActivated
// Callback Function called by svg engine when a hyperlink is activated
// ---------------------------------------------------------------------------
//

EXPORT_C TBool CSVGTAppObserverUtil::LinkActivated( const TDesC& aUri )
    {  
    if( aUri.Length() == 0 )
        {
        return KErrNone;
        }
    TRAPD( error, LinkActivatedL( aUri ) );
    return ( error == KErrNone );
    }


// --------------------------------------------------------------------
// CSVGTAppObserverUtil::LinkActivatedWithShow
// Callback Function called by the svg engine when a hyperlink with show 
// attribute is activated 
// --------------------------------------------------------------------
//
EXPORT_C TBool CSVGTAppObserverUtil::LinkActivatedWithShow( 
    const TDesC& aUri ,
    const TDesC& aShow )
    {
    if( aUri.Length() == 0 )
        {
        return KErrNone;
        }
    TRAPD( error, LinkActivatedWithShowL( aUri, aShow ) );
    return ( error == KErrNone );
    }    

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::GetSmilFitValue
// Default implementation for observer function to get SMIL fit value.
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::GetSmilFitValue( TDes& aSmilValue )
    {
    aSmilValue.Copy( KSvgtDefSmilFitValue );
    }   
     
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::SetDownloadMenu
// Sets/Unsets the iDownloadMenuEnabled
// -----------------------------------------------------------------------------
//

EXPORT_C void CSVGTAppObserverUtil::SetDownloadMenu( TBool aAddDownloadMenu ) 
    {
    iDownloadMenuEnabled = aAddDownloadMenu;    
    }
    
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::SetDownloadMenu
// Sets/Unsets the iDownloadMenuEnabled
// -----------------------------------------------------------------------------
//    
EXPORT_C TBool CSVGTAppObserverUtil::CanShowDownload() const
    {
        if(iDownloadInProgress || iDownloadDone)
            {
            const CDownloadArray& array = iDownloadMgr.CurrentDownloads();
            return ( array.Count() ? ETrue : EFalse ); 
            }
            
        return EFalse;            
    }       

// --------------------------------------------------------------------
// void CSVGTAppObserverUtil::DisplayDownloadMenuL()
// Call back called by the UI dialog when the downloads menu is 
// selected. This function calls the DisplayDownloadsListL function
// provided by the download manager ui lib.
// --------------------------------------------------------------------
//    
EXPORT_C void CSVGTAppObserverUtil::DisplayDownloadMenuL()
    {   
#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED          
    if ( iDMgrDownloadsList )
        {
        TRAPD( error, iDMgrDownloadsList->DisplayDownloadsListL() );        
            {
            if ( error != KErrNone )
                {
#ifdef _DEBUG                        
                TBuf<64> buffer;
                _LIT(msg, "AP error : %d" );
                buffer.Format( msg, error );
                iEikEnv->InfoMsg( buffer );              
#endif                
                }
            }
        }   
#endif
    }
    
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::HandleDMgrEventL
// Called by Download manager when the image is being downloaded. 
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::HandleDMgrEventL( RHttpDownload& aDownload,
                                       THttpDownloadEvent aEvent ) 
    {
    
#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED       
           
#ifdef _DEBUG        
        TBuf<64> buffer;
        _LIT(msg, "download progressing : %d, %d" );
        buffer.Format( msg, aEvent.iDownloadState, aEvent.iProgressState );
        iEikEnv->InfoMsg( buffer );        
#endif        
        {
        
        if ( EHttpDlInprogress == aEvent.iDownloadState ) 
            {
            iDownloadInProgress = ETrue;
            iDownloadDone = EFalse;
            }
            
        if ( EHttpDlCompleted == aEvent.iDownloadState )
            {  
             
             iDownloadInProgress = EFalse;
             iDownloadDone = ETrue; 
                               
            // Get the download item from the list
            CSVGTViewerAppDownloads* downloadItem;   
            downloadItem = FindDownload( &aDownload );
            
#ifdef _DEBUG                    
                
           
            _LIT(downloadMsg, "download found" );
            iEikEnv->InfoMsg( downloadMsg );
#endif            
                    
                       
            if ( downloadItem )
                {
                // Get the file Name of the downloaded file
                TFileName fileName;
                aDownload.GetStringAttribute( EDlAttrDestFilename, fileName );                
#ifdef _DEBUG                       
                   
                _LIT(fileNameMsg, "got file name " );
                iEikEnv->InfoMsg( fileNameMsg );
#endif            

//##############################################
// For AssignImageData
                DoAssignImageDataL(downloadItem, aDownload);
//##############################################
                downloadItem->SetFileName( fileName );
        
                // Check whether the download happened in a synchronous way
                if ( downloadItem->SynchronousMode() )
                    {                    
                    downloadItem->SetDownloadCompleted( ETrue );
                    
                    // If wait note is being displayed, delete it
                    if ( iWaitNote )
                        {
                        // Delete the download wait note
                        TRAPD( err, iWaitNote->ProcessFinishedL() );
                        if ( err != KErrNone )
                            {
                            delete iWaitNote;
                            iWaitNote = NULL;    
                            }                        
                        }                 

                    // If synchronous stop the scheduler 
                    iWait.AsyncStop();  
                    }
                else    
                    {
                    // Otherwise check the embedded mode and open the file 
                    // using DocHandler
                    downloadItem->QueLink().Deque();
                    delete downloadItem;
                    }
                }
            }
        else 
            {
            // added for if user cancels the download.
             if ( EHttpDlDeleting == aEvent.iDownloadState && !iDownloadDone)   
                {
                 aDownload.Pause();
                aEvent.iDownloadState = EHttpDlFailed;
                }
                
            if ( EHttpDlFailed == aEvent.iDownloadState )
                {       
#ifdef _DEBUG                              
              
                _LIT(failMsg, " downloading failed" );
                iEikEnv->InfoMsg( failMsg );    
#endif               
                                
                TInt32 error = 0;
                TInt32 globalError = 0;
                aDownload.GetIntAttribute( EDlAttrErrorId, error );
                aDownload.GetIntAttribute( EDlAttrGlobalErrorId, globalError );
                
#ifdef _DEBUG                  
                TBuf<64> errorBuffer;
                _LIT(failedMsg, "downloading failed : %d, %d" );
                buffer.Format( failedMsg, error, globalError );
                iEikEnv->InfoMsg( errorBuffer );
#endif               
             
                // Get the download item from the list             
                CSVGTViewerAppDownloads* downloadItem;      
                downloadItem = FindDownload( &aDownload );
                // Check whether the download was started synchronously
               // if( !downloadItem->ImageLinkFlag())
                    {
                    HBufC* lUriBuf = HBufC::NewLC( KMaxUrlLength );
                    TPtr lUriPtr(lUriBuf->Des());
                    aDownload.GetStringAttribute( EDlAttrReqUrl, lUriPtr ); 
                    iAppDialog->AssignImageData(lUriPtr , NULL );
                    CleanupStack::PopAndDestroy( lUriBuf );
                    }
                    
                if ( downloadItem )
                    {
                    if ( downloadItem->SynchronousMode() )
                        {
                        
                        downloadItem->SetDownloadCompleted( EFalse );
                        if ( iWaitNote )
                            {
                            // Delete the download wait note
                            TRAPD( err, iWaitNote->ProcessFinishedL() );
                            if ( err != KErrNone )
                                {
                                delete iWaitNote;
                                iWaitNote = NULL;    
                                }
                            }                 

                        // If synchronous, stop the scheduler started.
                        iWait.AsyncStop();
                        }   
                    else
                        {
                        // Otherwise delete the download item from the list
                        downloadItem->QueLink().Deque();
                        delete downloadItem;
                        }
                    }
                }
            } 
        }
#endif        
    }    
    
// ---------------------------------------------------------------------------
// CSVGTAppObserverUtil::LinkActivatedL
// Leave variant of LinkActivated
// Launches the browser in embedded mode and passes the uri to the browser
// if the uri is a web page, if the uri refers to a local image file, or a 
// remote image file, launches the file using doc handler.
// ---------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::LinkActivatedL( const TDesC& aUri )
    {
    
    TDownloadNeeded dnloadNeeded = IsDownloadNeededL( aUri );
    
    if ( dnloadNeeded == ENotAnImageFile )
        {           

        // (2 = length of the first parameter + space)            
        HBufC* parameters = NULL;
        parameters = HBufC::NewLC( aUri.Length() + 2 );
    
        TPtr parametersPtr = parameters->Des();
        parametersPtr.Copy( KBrowserParameter );
        parametersPtr.Append( KSpace );
        parametersPtr.Append( aUri );             
       
        // Create BrowserLauncher
        CBrowserLauncher* launcher = NULL;
    
        launcher = CBrowserLauncher::NewLC();        
        
        TBrowserOverriddenSettings overriddenSettings;
        launcher->LaunchBrowserSyncEmbeddedL( *parameters, NULL, &overriddenSettings );     
        
        //launcher, parameters   
        CleanupStack::PopAndDestroy( 2, parameters );
        }
    else
        {
        if ( dnloadNeeded == ERemoteImageFile )
            {
            RFile fileHandle; // param not going to be used
            RFs fileSession;  // param not going to be used
            StartDownloadingImageL( aUri,
                                    fileSession, 
                                    fileHandle,
                                    EFalse, /* aSynchronous mode */
                                    ETrue,
                                    ETrue /* Embedded mode */ );                                     
            }
        else if ( dnloadNeeded == ELocalImageFile )
            {
            
            TFileName fileName;
            TInt error = KErrNone;            
            
            if ( GetLocalFile( aUri, fileName, *iSvgFileName ) )
                {
                TRAP( error, LaunchDownloadedFileL( fileName, ETrue, EFalse ) );
                } 
            else
                {
                error = KErrNotFound;               
                }
                
            if ( error != KErrNone )
                {
                TInt resourceId;
                resourceId = R_SVGT_FILE_NOT_FOUND;
                // Show information note
                HBufC* prompt = StringLoader::LoadLC( resourceId );
                CAknInformationNote* note = 
                        new ( ELeave ) CAknInformationNote( ETrue );
                note->ExecuteLD( *prompt );

                CleanupStack::PopAndDestroy( prompt );
                }
            }
        else        
            {           
            return;
            }
        }  
    }
      
    
// --------------------------------------------------------------------
// CSVGTAppObserverUtil::LinkActivatedWithShowL
// Called by LinkActivatedWithShow 
// Calls LinkActivatedL
// --------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::LinkActivatedWithShowL( 
    const TDesC& aUri ,
    const TDesC& /*aShow*/ )
    {
    LinkActivatedL( aUri );
    }  
// ---------------------------------------------------------------------------
// CSVGTAppObserverUtil::StartDownloadingImageL    
// Called by FetchImage, LinkActivated, LinkActivatedWithShow to download the
// image. This function calls the download manager client Api's to start the 
// download. The download completion will be indicated by the call back 
// HandleDownloadMgrEventL. As FetchImage is a synchronous function
// a CActiveSchedulerWait loop is started, after starting the http download.
// the wait loop is stopped when the call back HandleDownloadMgrEventL is 
// called. 
// 1. CreateDownloadAndSetAttributes
// 2. check result
//    If result no
//          check the status of the download
//          if Completed 
//               return the filehandle
//          if in progress 
//               continue downloading
//          if failed
//               return 
//    else
//        Start Download
// 3. If Synchronous
//        Start DownloadWaitNote
//        Start ActiveSchedulerWait.
// 4. Return
//              
// parameters:
// aUri: the hyperlink that needs to be downloaded.
// aSession: File session in which the file handle is opened.
// aFileHandle: Handle of the downloaded file
// aIsSynchronous: flag indicating whether download is synchronous or not
// aLinkFlag: Image flag to distinguish the Embeded & hyperlink.
// aIsEmbedded: flag indicating whether downloaded file has to opened in 
//              stand alone mode or embedded mode.
// ---------------------------------------------------------------------------
//
TInt CSVGTAppObserverUtil::StartDownloadingImageL(
                    const TDesC& aUri, 
                    RFs& aSession, 
                    RFile& aFileHandle,     
                    TBool aIsSynchronous,
                    TBool aLinkFlag,
                    TBool aIsEmbedded )        
    {  
                                  
#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED           

     
#if defined(__WINS__) || defined(__WINSCW__)
    //do nothing, hard code later on
#else

    
    TUint32 Iapid( 0 );
    if ( !GetDefaultInetAccessPointL( Iapid ) )       
        {            
        return KErrNotFound;    
        }
        
    
    iDownloadMgr.SetIntAttribute( EDlMgrIap, (TInt32)Iapid );
#endif // __wins__|| __winscw__


#ifdef _DEBUG     
    _LIT( setAP, "Access point set" );
    iEikEnv->InfoMsg( setAP ); 
#endif

   
    TInt result = ETrue;     
    // Add lDownload to cleanup stack and pop it after CreateDownloadL.
    CSVGTViewerAppDownloads *lDownload = CSVGTViewerAppDownloads::NewLC( 
            aIsEmbedded, aIsSynchronous, aLinkFlag );  
            
    HBufC8* url8 = NULL;     
    url8 = HBufC8::NewLC( aUri.Length() );    
    url8->Des().Copy( aUri );                      
              
    lDownload->SetDownloadHandle( 
                    &iDownloadMgr.CreateDownloadL( *url8 , result ) );
                    
    CleanupStack::PopAndDestroy( url8 ); // url8                                                                            
        
#ifdef _DEBUG    
    _LIT(dlCreated, "download created" );
    iEikEnv->InfoMsg( dlCreated ); 
#endif    
    
            
    // Add Downloads to the Menu Option
    if ( !iDownloadMenuEnabled && !aIsSynchronous )
        {
        SetDownloadMenu( ETrue );        
        }
            
    if ( !result )                           
        {  
          
        // url is already being downloaded or completed
        // Get the download State
        TInt32 downloadState = 0;
        TInt retval = KErrNone;
        lDownload->DownloadHandle()->GetIntAttribute( EDlAttrState,
                        downloadState );       
       
        if ( downloadState == EHttpDlCompleted )
            {
            TFileName fileName;
            lDownload->DownloadHandle()->GetStringAttribute( EDlAttrDestFilename
                            , fileName );
            if ( aIsSynchronous )
                {
                retval = aFileHandle.Open( aSession, fileName, EFileShareAny );
                }            
            else    
                {
                if(aLinkFlag)
                    {
                    // Handle Launching downloaded file in asynchronous mode                   
                    LaunchDownloadedFileL( fileName, aIsEmbedded , ETrue);    
                    }
                else
                    {
                    DoAssignImageDataL(lDownload, *(lDownload->DownloadHandle()));
                    }
                       
                }                     
            
            CleanupStack::PopAndDestroy( lDownload ); // lDownload     
            return retval;
            }               
        else
            {
            if ( downloadState == EHttpDlFailed )
                { 
                // Retry
                // Add to the linked list 
                iDownloads.AddFirst( *lDownload );               
                CleanupStack::Pop( lDownload ); // lDownload                     
                lDownload->DownloadHandle()->SetBoolAttribute( 
                                   EDlAttrNoContentTypeCheck, ETrue );              
                // Start the download       
                lDownload->DownloadHandle()->Start();
                
                return KErrNone;
                }
            else 
                {                             
                                                                                  
                if ( !FindDownload( lDownload->DownloadHandle() ) )
                    {
                    // Add to the linked list 
                    iDownloads.AddFirst( *lDownload );  
                    CleanupStack::Pop( lDownload ); // lDownload 
                    }
                else
                    {
                    CleanupStack::PopAndDestroy( lDownload ); // lDownload 
                    }
                }                   
            }
        }
    else
        { 
        if ( aIsSynchronous )
            {            
            TFileName fileName( KEmptyString );
            GetFileNameFromUri( aUri, fileName );               
            LaunchWaitNoteL( R_SVGT_DOWNLOAD_WAIT_NOTE, ETrue, fileName );                        
            }
       if ( !aIsSynchronous )
            {            
            lDownload->DownloadHandle()->SetIntAttribute( EDlAttrAction, 
                            lDownload->ImageLinkFlag() ? ELaunch : EDoNothing );             
//                iIsFetchImageCall ? EDoNothing : ELaunch );           
            }
       else
            {
            lDownload->DownloadHandle()->SetIntAttribute( EDlAttrAction,
                                                          EDoNothing );
            lDownload->DownloadHandle()->SetBoolAttribute( EDlAttrSilent, 
                                          ETrue );
            }
            
       lDownload->DownloadHandle()->SetBoolAttribute( 
                                   EDlAttrNoContentTypeCheck, ETrue );
            
                
#ifdef _DEBUG                 
        _LIT(dlCreateMsg, "result true dl created" );
        iEikEnv->InfoMsg( dlCreateMsg ); 
#endif        
       
       // Add to the linked list 
       iDownloads.AddFirst( *lDownload ); 
              
       CleanupStack::Pop( lDownload ); // lDownload     
              
       // Start the download       
       lDownload->DownloadHandle()->Start();
       }
        
    if ( aIsSynchronous )
        {  
        // Start the Scheduler Wait
        if ( !iWait.IsStarted() )
            {
            iWait.Start();
            }        
        
#ifdef _DEBUG                         
        _LIT(waitLoopMsg, "out of wait loop" );        
        iEikEnv->InfoMsg( waitLoopMsg ); 
#endif        
        if ( iSynchronousDownloadStopped )
            {
            // Download dismissed
            // Cancel the download
            lDownload->DownloadHandle()->Delete();                    
            return KErrCancel;     
            }      

        CSVGTViewerAppDownloads* downloadItem = NULL;
        downloadItem = FindDownload( lDownload->DownloadHandle() );
       
        if ( downloadItem )
            {
            TInt retval = KErrNone;
            TBool downloadCompleted = downloadItem->DownloadCompleted();                       
            
            if ( downloadCompleted )
                {                
                retval = aFileHandle.Open( aSession, 
                                           downloadItem->FileName(), 
                                           EFileShareAny );
                }
            else
                {
                retval = KErrNotFound;
                }
            
            downloadItem->QueLink().Deque();                    
            delete downloadItem;
            return retval;      
            }
        else
            {
            return KErrNotFound;     
            }
        }
    else
        {      
        return KErrNone;           
        }
        
#else
        return KErrNotFound;
#endif      
        
    }   

// ---------------------------------------------------------------------------
// CSVGTAppObserverUtil::GetDefaultInetAccessPointL
// if the return value is true, aIapId has the default Iap id.
// ---------------------------------------------------------------------------
EXPORT_C TBool CSVGTAppObserverUtil::GetDefaultInetAccessPointL
                              ( TUint32& aIapId ) const
    {

    // Try to find the default AP.
    // The Browser may stores the default AP in the shared data.
    CRepository* repository = CRepository::NewL( KCRUidBrowser );
    CleanupStack::PushL( repository );

    TInt wapApId = 0;
    TRAPD( error, repository->Get( KBrowserDefaultAccessPoint, wapApId ) );
    if ( error != KErrNone )
        {
#ifdef _DEBUG                         
        TBuf<64> buffer;
        _LIT(msg, "AP error : %d" );
        buffer.Format( msg, error );
        iEikEnv->InfoMsg( buffer );
        User::After(1000000);
#endif      
        return EFalse;   
        }
    TUint32 wapApId32( wapApId );
    TUint32 iapId32( 0 );
    
    CleanupStack::PopAndDestroy( repository );

    CCommsDatabase* commsDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
    CleanupStack::PushL( commsDb );
    CApDataHandler* apDataHandler = CApDataHandler::NewLC( *commsDb );
    CApAccessPointItem* apItem = CApAccessPointItem::NewLC();
    TRAPD( err, apDataHandler->AccessPointDataL( wapApId32, *apItem ) );
    CApUtils* apUtils = CApUtils::NewLC( *commsDb );

    TBool wapApFound( EFalse );
    TBool iapFound( EFalse );
    
    if ( err == KErrNone )
        {
        wapApFound = ETrue;
        }
    else 
        {
        // The first valid access point has to be selected if exists
        CApSelect* apSelect = CApSelect::NewLC
            (
                *commsDb, 
                KEApIspTypeAll, 
                EApBearerTypeAll, 
                KEApSortNameAscending
            );
        if ( apSelect->MoveToFirst() )
            {
            wapApFound = ETrue;
            wapApId32 = apSelect->Uid();
            }
        CleanupStack::PopAndDestroy( apSelect ); // apSelect
        }

    if ( wapApFound )
        {
        // Get the IAP ID from the WAP AP ID.
#ifndef __SERIES60_WSP
        iapId32 = 0;
        TRAPD( errTransl, iapId32 = apUtils->IapIdFromWapIdL( wapApId32 ) );
       
        if ( errTransl == KErrNone )
            {
            iapFound = ETrue;
            }
#else
        iapId32 = wapApId32;
        iapFound = ETrue;
#endif // __SERIES60_WSP
        }

    CleanupStack::PopAndDestroy( 4 ); // apUtils, apItem, 
                                      // apDataHandler, commsDb

    if ( iapFound )
        {
        aIapId = iapId32;
        }
    return iapFound;
    }
    
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::LaunchDownloadedFileL
// Called by Download manager when the image is downloaded.
// This function uses the document handler to launch the downloaded file.
// -----------------------------------------------------------------------------
//   
EXPORT_C void CSVGTAppObserverUtil::LaunchDownloadedFileL( const TDesC& aFileName, 
                                        TBool /* aIsEmbedded */,  
                                        TBool aAllowSave ) const
    {            
   
    TDataType dataType = TDataType();
    RFile fileHandle;          
    // Creates a sharable file handle
    iDocHandler->OpenTempFileL( aFileName, fileHandle );                
    CleanupClosePushL( fileHandle );        
    if ( aAllowSave )
        {      
        CAiwGenericParamList& paramList = iDocHandler->InParamListL();                
        // To show/hide save confirmation dlg
        TAiwVariant allowSaveVariant(aAllowSave);
        TAiwGenericParam genericParamAllowSave( EGenericParamAllowSave, 
                                                allowSaveVariant );
        paramList.AppendL( genericParamAllowSave );
        
        iDocHandler->OpenFileEmbeddedL( fileHandle, dataType, paramList );    
        }
    else
        {
        _LIT( KSvgFile, ".svg");
        _LIT( KSvgzFile, ".svgz");
        _LIT( KSvgbFile, ".svgb");
        if(aFileName.Right(4) == KSvgFile || aFileName.Right(5) == KSvgzFile || aFileName.Right(5) == KSvgbFile)
            {
            iDocHandler->OpenFileEmbeddedL( fileHandle, dataType );  
            }
        else
            {
            iDocHandler->OpenFileL( fileHandle, dataType );           
            }
        }
    CleanupStack::PopAndDestroy( &fileHandle ); // fileHandle        
    } 
    
// ---------------------------------------------------------------------------
// CSVGTAppObserverUtil::IsDownloadNeededL
// Called by link activated(WithShow), FetchImage 
// to check whether the uri needs a download
// If the filename is present in the uri and if it is a html, 
// returns ENotAnImageFile
// If the host is present & filename is not present, 
// returns ENotAnImageFile
// If the host is present & filename is present, 
// returns ERemoteImageFile
// If the uri doesnt need a download and if it is locally available, 
// returns ELocalImageFile 
// ---------------------------------------------------------------------------
//    
EXPORT_C CSVGTAppObserverUtil::TDownloadNeeded 
      CSVGTAppObserverUtil::IsDownloadNeededL( 
        const TDesC& aUri )
    {

    TUriParser UriParser;
    
    TBuf<KMaxFileName> localUri;
    // url starts with www so append http://
    if ( !aUri.Left( 4 ).Compare( KWww ) )     
        {
        localUri.Append( KHttp );
        localUri.Append( aUri );
        User::LeaveIfError( UriParser.Parse( localUri ) );   
        }
    else
        {
        User::LeaveIfError( UriParser.Parse( aUri ) );   
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

// ---------------------------------------------------------------------------
// CSVGTAppObserverUtil::GetFileNameFromUri
// This function gets the filename from the uri, 
// Returns true if the file name is retrieved
// Returns False if the file name could not be retrieved.
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CSVGTAppObserverUtil::GetFileNameFromUri( const TDesC& aUri, 
        TDes& aFileName ) const
    {
    
    const TChar KBSlash = '/';
    TUriParser UriParser;
    if ( UriParser.Parse( aUri ) != KErrNone )
        {
        return EFalse;
        }
    
    if ( UriParser.IsPresent( EUriPath ) )
        {
        // Extract the Path, filename and extension from the uri
        TFileName filename = UriParser.Extract( EUriPath );        
        
        TParse fileParser;
        fileParser.Set( filename, NULL, NULL );
        
        TInt slashPos = filename.LocateReverse( KBSlash );
        if ( slashPos != KErrNotFound )
            {
            aFileName = filename.Right( filename.Length() - (slashPos + 1));
            return ETrue;               
            }
        else
            {
            return EFalse;      
            }      
        
        }
    else
        {
        return EFalse;
        }    
    
    }
// ---------------------------------------------------------------------------
// CSVGTAppObserverUtil::GetLocalFile
// This function gets the filename  with full path from the uri, 
// Returns true if the file name is retrieved
// Returns False if the file name could not be retrieved.
// ---------------------------------------------------------------------------
//    
EXPORT_C TBool CSVGTAppObserverUtil::GetLocalFile( 
        const TDesC& aUri, 
        TDes& aFileName ,
        const TDesC& aSvgFileName )
    {
    
    TBuf<KMaxFileName> copyUri = aUri;
    // Before giving the uri to the file parser,
    // convert '/' to '\'.          
    TInt pos;
    while( KErrNotFound != ( pos = copyUri.FindF( KBSlashStr ) ) )
        {
        copyUri.Replace(pos,1,KSlashStr );
        }

    // if the uri starts with ".\" remove ".\"
    if ( copyUri.Left( 2 ).Compare( KDotSlashStr ) == 0 ) 
        {
        copyUri = copyUri.Right( copyUri.Length() - KDotSlashStr().Length() );
        }   
        
    // If the second letter is not ':' 
    // and if it is not starting with '\',
    // and if the path has '/',
    // then prepend slash to it.
    if ( copyUri.Length() >= 2 && 
         copyUri.Mid( 1, 1 ).Compare( KColonStr ) != 0 && 
         copyUri.Mid( 0, 1 ).Compare( KSlashStr ) != 0 &&
         ( copyUri.Find( KSlashStr ) != KErrNotFound ) )
        {
        copyUri.Insert( 0, KSlashStr );
        }       
    
    TParse fileParser;
    if( fileParser.Set( copyUri, NULL, NULL ) != KErrNone )
        {
        return EFalse;
        } 
            
    if ( fileParser.DrivePresent() )
        {
        // The uri is an absolute path
        aFileName.Append( fileParser.Drive() ); 
        // Append path, file name and extension from the uri.
        if ( fileParser.PathPresent() )
            {
            aFileName.Append( fileParser.Path() );              
            }
        if ( fileParser.NamePresent() )
            {
            aFileName.Append( fileParser.Name() );              
            }
        if ( fileParser.ExtPresent() )
            {
            aFileName.Append( fileParser.Ext() );
            }
        RFs session;
        TInt connectError=session.Connect();
        if(connectError==KErrNone)
            { 
            RFile fileHandle;
            TInt openError  = fileHandle.Open( session, aFileName, EFileShareReadersOnly);    
            fileHandle.Close();
            session.Close();
            if ( openError != KErrNone  )
               {
               return EFalse;
               } 
            return ETrue;
            }
        session.Close();
        return EFalse;         
        }
    else
        {
        
        TParse svgFilenameParser;
        if ( svgFilenameParser.Set( aSvgFileName,NULL,NULL ) != KErrNone )
            {
            return EFalse;
            }
        
        if ( svgFilenameParser.DrivePresent() )
            {
            aFileName.Append( svgFilenameParser.Drive() );               
            }
        
        TInt dotdotPos = copyUri.Find( KDotDot );
        TInt rightPos = 0;        
        
        if ( dotdotPos != KErrNotFound )
            {
            // If ".." is present collapse it with the parent path
            if ( svgFilenameParser.PathPresent() )
                {                  
                do   
                    {
                    if ( dotdotPos > 1 )
                        {
                        svgFilenameParser.AddDir( copyUri.Left(dotdotPos - 1) );
                        }
                                                          
                    if ( svgFilenameParser.PopDir() != KErrNone )
                        {
                        return EFalse;
                        }               
                        
                    rightPos = (copyUri.Length()) - (dotdotPos + 3);
                    
                    if ( rightPos > 0 )
                        {                       
                        copyUri = copyUri.Right( rightPos );    
                        }
                                                                                                   
                    }while ( (dotdotPos  = copyUri.Find( KDotDot ) ) 
                                   != KErrNotFound ); 
                                   
                aFileName.Append( svgFilenameParser.Path() );
                aFileName.Append( copyUri );             
                }
            else
                {
                return EFalse;
                }                       
            }
        else
            {
            if ( svgFilenameParser.PathPresent() )
                {
                aFileName.Append( svgFilenameParser.Path() );
                }           
        
            // Append path, file name and extension from the uri.    
            if ( fileParser.PathPresent() )
                {
                //deletes an extra slash in aFileName
                TInt strlen = aFileName.Length();
                if(aFileName.Mid(strlen-1).Compare(KSlashStr) == 0)
                    {
                    aFileName.Delete(strlen-1, 1);
                    }
                aFileName.Append( fileParser.Path() );              
                }
                                
            if ( fileParser.NamePresent() )
                {
                aFileName.Append( fileParser.Name() );              
                }
            if ( fileParser.ExtPresent() )
                {
                aFileName.Append( fileParser.Ext() );
                }
            } 
        RFs session;
        // The error is caught to avoid any code scanner error
        TInt connectError=session.Connect();
        if(connectError==KErrNone)
            {
            RFile fileHandle;
            TInt openError  = fileHandle.Open(session, aFileName, EFileShareReadersOnly );    
            fileHandle.Close();
            session.Close();
            if ( openError != KErrNone  )
                {
                return EFalse;
                }      
            return ETrue;
            }
        session.Close();
        return EFalse;                
        }                                  
    }
               
    
#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED       
// ---------------------------------------------------------------------------
// CSVGTAppObserverUtil::FindDownload
// This iterates through the linked list and return the link 
// corresponding to the given http download handle
// If a link is not found returns NULL.
// ---------------------------------------------------------------------------
//    
CSVGTViewerAppDownloads* CSVGTAppObserverUtil::FindDownload(
                                         const RHttpDownload* aHttpDownload )
    {
    CSVGTViewerAppDownloads* downloadItem;
    TDblQueIter<CSVGTViewerAppDownloads> Iter( iDownloads );    
    if ( !iDownloads.IsEmpty() )
        {
            Iter.SetToFirst();
            while ( ( downloadItem = Iter ) != NULL )
                {
                if ( downloadItem->DownloadHandle() == aHttpDownload )
                    {
                    return downloadItem;
                    }  
                Iter++;
                }            
        }
    return NULL;     
    }
#endif    

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::CSVGTAppObserverUtil
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CSVGTAppObserverUtil::CSVGTAppObserverUtil( 
            CSVGTUIDialog* aAppDialog ) :
#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED
             iDownloads( CSVGTViewerAppDownloads::LinkOffset() ) ,
             iDownloadIter( iDownloads ),
             iDownloadMenuEnabled( EFalse ), // construct header & iterator
             iSynchronousDownloadStopped( EFalse ),
#endif 
              iAppDialog( aAppDialog ),
              iSavingDone( EFalse ),
              iCommandId( 0 ),
//              iIsFetchImageCall(EFalse),
              iTempFileUsed( EFalse ),
              iDownloadDone( EFalse ),
              iDownloadInProgress( EFalse )

    {
    }
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::ConstructL()
    { 
  
    // Get the CEikonEnv pointer
    iEikEnv = CEikonEnv::Static(); 
            
    // Find which drive this DLL is installed.
    TFileName fileName;

    // Get the full resource file
    TParse fp;

    // Add the resource dir
    fp.Set( KResourceFileName, &KDC_RESOURCE_FILES_DIR, NULL);

    // Get the filename with full path
    fileName = fp.FullName();

    BaflUtils::NearestLanguageFile( iEikEnv->FsSession(), fileName ); //for
                                                               // localization
    iResourceOffset = iEikEnv->AddResourceFileL( fileName );


    // File manager utility needed for saving/moving files.
    iFileManager = CSVGTFileManager::NewL();
    
    iDocHandler = CDocumentHandler::NewL(); 

#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED
    TUid hostUid = iEikEnv->EikAppUi()->Application()->AppDllUid();
       
    //creates a download manager session
    //parameters - uid, mhttpdownloadmgrobserver
    //embedded mode on - master flag True has to be passed when the app is 
    //in stand alone mode, false in case of embedded mode
    iDownloadMgr.ConnectL( hostUid ,*this , EFalse /* Embedded mode*/);
    // When SVGT Viewer exits, all the downloads are 
    // cancelled by the download manager
    iDownloadMgr.SetIntAttribute( EDlMgrExitAction , EExitDelete );

    iDMgrUiReg  = CDownloadMgrUiLibRegistry::NewL( iDownloadMgr );
    iDMgrDownloadsList = &iDMgrUiReg->RegisterDownloadsListL();
    iDMgrUserInteractions = &iDMgrUiReg->RegisterUserInteractionsL();
    iDMgrUserInteractions->SetBoolAttributeL( CDownloadMgrUiUserInteractions::EAttrSuppressDownloadConfirmation,
                                              ETrue );
#endif  
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::ConstructL(
                                            RFile* aFileHdlPtr,
                                            const TDesC& aFilename, 
                                            TBool aIsFileFullPathAvailable,
                                            TBool aCanSaveContent,
                                            TBool aShouldMoveContent )
    {
    iIsFullFilePathValid = aIsFileFullPathAvailable;
    iCanSaveContent = aCanSaveContent;
    iShouldMoveContent = aShouldMoveContent;
    iSvgFileId = aFileHdlPtr;
    if ( aIsFileFullPathAvailable )
        {
        iSvgFileName = aFilename.AllocL();
        }
    else
        {
        TFileName fullPathName;
        iSvgFileId->FullName( fullPathName );
        iSvgFileName = fullPathName.AllocL();
        }
    ConstructL();
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::ConstructL(
                                            RFile* aFileHdlPtr
                                            )
    {
    TBuf<1> lDummyFileName; /* This is needed by ConstructL, 
                               but will be ignored */
    
    ConstructL( aFileHdlPtr, 
                lDummyFileName, 
                EFalse /* IsFileFullPathAvailable */, 
                ETrue  /* CanSaveContent */,
                EFalse /* aShouldMoveContent */
              );
    }
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::CanShowSave
// Tells the SVG-T dialog if the save option should be displayed.
// e.g. If the dialog is launched from SVG-T viewer and which is launched
// embedded from Browser/Messaging then Save option would be displayed.
// Also when MMS launches the dialog.
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CSVGTAppObserverUtil::CanShowSave()
{
    return iCanSaveContent;
}

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::CanShowSave
// Tells the SVG-T dialog if saving of the content is done.
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CSVGTAppObserverUtil::IsSavingDone()
{
    return iSavingDone;
}
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::DoSaveL
// Performs the save operation on the content opened. 
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::DoSaveL( TInt aCommandId )
    {
    iCommandId = aCommandId;
    // Get filename.
    // Run Dialog.
    // Need to pause content
    iAppDialog->ProcessCommandL( ESvgtDlgPauseCommand );

    // Get the currently playing svg file's name and extension.    
    HBufC* lFileNameBuf = HBufC::NewLC( KMaxFileName );
    TPtr lFileNamePtr = lFileNameBuf->Des();
    GetFileNameAndExt( lFileNamePtr );
   
 #ifndef RD_MULTIPLE_DRIVE    
    // Get User's choice of name and folder for target file.
    TBool retval = AknCommonDialogs::RunSaveDlgNoDirectorySelectionLD(
        lFileNamePtr,
        R_SVGT_SAVE_LOCATIONS );
#else
      
    TBool retval = AknCommonDialogsDynMem::RunSaveDlgNoDirectorySelectionLD( 
                                         AknCommonDialogsDynMem::EMemoryTypeRemote | 
                                         AknCommonDialogsDynMem::EMemoryTypePhone | 
                                         AknCommonDialogsDynMem::EMemoryTypeMMC, 
                                         lFileNamePtr, R_SVGT_SAVE_LOCATIONS);
#endif
    
    if ( retval )
        {
        // User chose a valid file name        
        TBool lMoveFlag = EFalse;
        TBuf<1> lDummyFileName;
        RFile lDummyFile;
        TInt errSaveContent = KErrNone;
        TInt errSaveText = KErrNone;
        
        // Assume it is not the overwrite case by default
        iIsOverwriteCase = EFalse; 

        // Get the currently playing svg file's full path
        HBufC* lFullFilePathBuf = HBufC::NewLC( KMaxFileName );
        TPtr lFullFilePathPtr = lFullFilePathBuf->Des();
        
        if ( iSvgFileId )
            {
            iSvgFileId->FullName( lFullFilePathPtr );
            }
        // Check if file name is same as orig file -> overwrite case
        if ( !( lFileNamePtr.CompareF( lFullFilePathPtr ) ) )
            {
            iIsOverwriteCase = ETrue;
            }
        CleanupStack::PopAndDestroy( lFullFilePathBuf );
        lFullFilePathBuf = NULL;
               
        // Check if text edit happened
        if ( iAppDialog->IsTextContentChanged() )
            {
            if ( iTempFileName )
                {
                delete iTempFileName;
                iTempFileName = NULL;
                }
            iTempFileName = HBufC::NewL( KMaxFileName );
            TPtr lTempFilePtr = iTempFileName->Des();
            
            // Indicate that cleanup is to be performed in SVGTSavingDoneL        
            iTempFileUsed = ETrue;        
            
            TRAP( errSaveText, SaveSvgTextContentL( *iSvgFileId, lTempFilePtr ) );
            
            if ( !errSaveText )
                {
                // Temp file should be moved instead of
                // original file
                lMoveFlag = ETrue;

                if ( IsAllowMove() || iIsOverwriteCase )
                    {
                    // Based on Move option/self overwrite case, need to close 
                    // the content being played.           
                    // Need to close the currently running file-handle, 
                    // need to inform dialog to stop using it.
                    iAppDialog->StopUsingContentFileHandle();
                    iSvgFileId->Close(); 
                    }
                
                if ( IsAllowMove() )
                    {
                    // Remember that the original file should 
                    // be deleted if file save is successful.
                    iDeleteOrigFile = ETrue;    
                    }
                    

                TRAP( errSaveContent, SaveContentL( 
                      lDummyFile, 
                      lTempFilePtr,
                      lFileNamePtr, 
                      lMoveFlag ));            
                }
            }
            else // No text editing happened
                {

                if ( IsAllowMove() || iIsOverwriteCase )
                    {
                    // Based on Move option/self overwrite case, need to close 
                    // the content being played.           
                    // Need to close the currently running file-handle, 
                    // need to inform dialog to stop using it.
                    iAppDialog->StopUsingContentFileHandle();
                    iSvgFileId->Close(); 
                    }
                
                if ( IsAllowMove() )
                    {
                    lMoveFlag = ETrue;
                    TRAP( errSaveContent, SaveContentL( 
                        lDummyFile, 
                        *iSvgFileName,
                        lFileNamePtr, 
                        lMoveFlag ));
                    }
                    else
                        {
                        TRAP( errSaveContent, SaveContentL( 
                            *iSvgFileId, 
                            lDummyFileName,
                            lFileNamePtr, 
                            lMoveFlag ));
                        }
                }

        CleanupStack::PopAndDestroy( lFileNameBuf );
            
            
        if ( errSaveText || errSaveContent )
            {
            // If disk full problem, display note.
            if ( errSaveContent == KErrDiskFull || errSaveText == KErrDiskFull )
                {
                // mmc full note
                HBufC* text = StringLoader::LoadLC( R_SVGT_MMC_FULL );
                CAknGlobalNote* note = CAknGlobalNote::NewLC();
                note->SetSoftkeys( R_AVKON_SOFTKEYS_OK_EMPTY__OK );
                note->ShowNoteL( EAknGlobalErrorNote, *text );
                CleanupStack::PopAndDestroy(2);// note, text
                }

            // If temporary file was used, perform cleanup
            if ( iTempFileUsed )
                {
                if ( iTempFileName )
                    {
                    // Try and delete the temp file, ignore the error.
                    iEikEnv->FsSession().Delete( iTempFileName->Des() );
                    delete iTempFileName;
                    iTempFileName = NULL;
                    }
                iTempFileUsed = EFalse;
                }
                        
            // If the File Couldn't be Moved, then Open the original
            // file again and Set it back to dialog.
            if ( IsAllowMove() )
                {
                if( iSvgFileId->Open(iEikEnv->FsSession(),
                        iSvgFileName->Des(), EFileShareAny) != KErrNone )
                    {
                    // This should not happen, Couldn't reopen back the
                    // Original content!, No Point in showing save.
                    iCanSaveContent = EFalse; 
                    return;
                    }
                else
                    {
                    iAppDialog->StartUsingContentFileHandle();
                    }
                }
            }
            else
                {
                LaunchWaitNoteL( R_SVGT_SAVING_WAIT_NOTE, ETrue, lDummyFileName );
                }
        }
    else
        {
        // User pressed cancel in save dialog - cleanup
        CleanupStack::PopAndDestroy( lFileNameBuf );
        }
    }

EXPORT_C TInt CSVGTAppObserverUtil::NewFetchImageData( const TDesC& aUri )
    {
//    iIsFetchImageCall = ETrue
    TInt ret = KErrNone;
    
    TDownloadNeeded downloadNeeded = ELocalImageFile;
    TRAPD( error, downloadNeeded = IsDownloadNeededL( aUri ) );
    
    if ( error != KErrNone )
        {
        return KErrNotFound;
        }
    
    if ( downloadNeeded == ELocalImageFile )
        {
        TFileName fileName;
        if ( GetLocalFile( aUri, fileName, *iSvgFileName ) )
            {
            TRAPD(error, DoAssignLocalFileDataL( aUri, fileName ));
            return error;
            }
        else
            {
            iAppDialog->AssignImageData(aUri , NULL );
            return KErrNotFound;
            }        
        }
    else if ( downloadNeeded == ERemoteImageFile )
        {
        //creatred for Dummy purpose 
        RFs fs; 
        RFile file;
        
        TRAPD( err,ret = StartDownloadingImageL( aUri, fs, file,
                                               EFalse,
                                               EFalse /* Asynchronous mode */) ); 
        if ( err != KErrNone )
            {
            return KErrNotFound;
            }
        else
            {
            return ret;         
            }
        
        }
    else
        {
        return KErrNotFound;
        }   
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::SaveContentL
// Performs the save operation on the content opened given 
// source file handle, src/dst filename and whether to move/copy. 
// -----------------------------------------------------------------------------
//
void CSVGTAppObserverUtil::SaveContentL( RFile& aSrcFileHandle, // Used in copy
                                         const TDesC& aSrcFileName,   // Used in move
                                         TDes& aDstFileName,    // Dest File name
                                         TBool aMoveFlag )      // ETrue = Move
    {
    TInt ret = KErrNone;        
    // Start Saving Operation.        
    // Create directory 
    TInt error = iEikEnv->FsSession().MkDirAll( aDstFileName );
    if ( error != KErrNone && error != KErrAlreadyExists )
        {
        User::Leave( error );
        }
  
    // Check if the content should be moved. e.g. In Browser case.
    iFileManager->SetAllowMove( aMoveFlag );

    // Ask FileManager to start saving. ObserverUtil acts as
    // observer for filemanager operations. Based on whether
    // the Move operation is required or not,
    // there are two versions of filecopy operations.
    // In Move Full filepath is required
    if ( aMoveFlag )
        {
        if ( IsFileFullPathAvailable() )
            {
            // Keep the Destination file reference.
            if ( iSvgDstFileName )
                {
                delete iSvgDstFileName;
                iSvgDstFileName = NULL;
                }
            iSvgDstFileName = aDstFileName.AllocL();

            // Start the Move Now.
            ret = iFileManager->StartCopyOperationL( 
                this, aSrcFileName, aDstFileName );
            }
        else
            {
            // Shouldn't happen. If Move operation is required,
            // Full file path must be there.
            ret = KErrPathNotFound;  
            }
        }
    else
        {
        ret = iFileManager->StartCopyOperationL(
            this, aSrcFileHandle, aDstFileName );
        }
    User::LeaveIfError( ret );
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::GetUniqueFileName
// Generates a unique filename in the private directory
// -----------------------------------------------------------------------------
//
void CSVGTAppObserverUtil::GetUniqueFileName( TDes& aName ) const
    {
    TFileName lPpath;
    iEikEnv->FsSession().PrivatePath( lPpath );
    RThread lThread;
    _LIT( KFNameFormatStr,"%u");
    TBuf<KSvgMaxThreadIdStr> lThreadIdStr;
    lThreadIdStr.Format( KFNameFormatStr, (TUint)(lThread.Id()) );
    aName.Append( lPpath );
    aName.Append( lThreadIdStr );
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::IsGzipContent
// Checks whether the file is a GZipped content
// -----------------------------------------------------------------------------
//
TBool CSVGTAppObserverUtil::IsGzipContent( const RFile& aFileHandle ) const
{
    TUint8 ids[2];
    TPtr8 des( ids, 0, sizeof( TUint8 ) * 2 );
    TInt zero = 0;    
    
    // Reset to the beginning
    aFileHandle.Seek( ESeekStart, zero );
    
    // Read the first two bytes
    if ( aFileHandle.Read( des ) != KErrNone )
        {
        return EFalse;
        }

    // reset to start of file
    aFileHandle.Seek( ESeekStart, zero );
    return ( ids[0] == 31 && ids[1] == 139 );
//    return ( ids[0] == EZGZipFile::ID1 && ids[1] == EZGZipFile::ID2 );
}

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::SaveSvgContent
// Saves the SVG content to a temporary file
// -----------------------------------------------------------------------------
//
void CSVGTAppObserverUtil::SaveSvgTextContentL( const RFile& aSrcFileHandle,  
    TDes& aTempFileName )
    {
    TFileName lTempFileName;

    // Save the content path
    HBufC* lOrigFileNameBuf = HBufC::NewLC( KMaxFileName ); 
    TPtr lOrigFileNamePtr = lOrigFileNameBuf->Des();
    aSrcFileHandle.FullName( lOrigFileNamePtr );
    
    // Create the temp file in the same drive as the content file
    TParsePtrC lOrigPath( lOrigFileNamePtr );
    
    // Put the drive into the path
    lTempFileName.Append( lOrigPath.Drive() );
    
    // lOrigFileNameBuf no longer needed. No more references to
    // lOrigFileNamePtr.
//    CleanupStack::PopAndDestroy( lOrigFileNameBuf ); 
    
    // Generate a unique file name
    GetUniqueFileName( lTempFileName );
    
    TInt lOrigFileSize = 0;
    TInt lFileSizeErr = aSrcFileHandle.Size( lOrigFileSize );
    if ( lFileSizeErr )
        {
        User::Leave( lFileSizeErr );
        }
        
    // Check whether freespace available
    // Assume that the new file occupies same amount of space as the original 
    // file. Not so in the case of svgz file. 
#ifndef RD_MULTIPLE_DRIVE
    if( lOrigPath.Drive().CompareF( PathInfo::MemoryCardRootPath().Left(2) ) 
        == 0 )
        {
        if (SysUtil::MMCSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),
            lOrigFileSize ))
            {            
            User::Leave( KErrDiskFull ); // MMC full, Avkon shows note
            }
        }
    else
        {
        if (SysUtil::FFSSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),
            lOrigFileSize ))
            {
            User::Leave( KErrDiskFull ); // Phone memory full, Avkon shows 
                                         // note
            }
        }  
#else
//******************* Added to support multiple drives    *************************
    TInt intDrive;
    TChar ch = lOrigPath.Drive()[0];
    
    User::LeaveIfError( RFs::CharToDrive(ch,intDrive) );
    
    if( SysUtil::DiskSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),
                                lOrigFileSize, intDrive ))
        {
        User::Leave( KErrDiskFull ); // MMC full, Avkon shows note
        }

//*********************************************************************************
#endif
    // lOrigFileNameBuf no longer needed. No more references to
    // lOrigFileNamePtr.
    CleanupStack::PopAndDestroy( lOrigFileNameBuf ); 

    // Create the directory to the temp filename
    TInt lDirCreateError = iEikEnv->FsSession().MkDirAll( lTempFileName  );
    
    // Ignore error if the directory already exists
    if ( lDirCreateError  != KErrNone && lDirCreateError != KErrAlreadyExists )
        {
        User::Leave( lDirCreateError );
        }
    
    // Save the changed SVG content to that file name
    iAppDialog->SaveSvgDom( lTempFileName );
    
    // Check if content is gzipped
    if ( IsGzipContent( aSrcFileHandle ) )
        {
        // Zip the content
        }
    aTempFileName = lTempFileName;
    }
    
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::GetFileNameAndExt
// -----------------------------------------------------------------------------
// 
EXPORT_C void CSVGTAppObserverUtil::GetFileNameAndExt(TDes& filename) const
{

    // If FileName is valid use that, else check filehandle
    // and give back file-name from that.
    if ( iIsFullFilePathValid )
        {
            if ( iSvgFileName )
                {
                TParsePtrC p(*iSvgFileName);
                TPtrC ptr(p.NameAndExt());
                filename = ptr;
                }
        }
    else
        {
            if ( iSvgFileId )
                {
                iSvgFileId->Name( filename );
                }
        }

}

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::IsAllowMove
// -----------------------------------------------------------------------------
EXPORT_C TBool CSVGTAppObserverUtil::IsAllowMove() const
{
    return iShouldMoveContent;
}

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::IsFileFullPathAvailable
// -----------------------------------------------------------------------------
EXPORT_C TBool CSVGTAppObserverUtil::IsFileFullPathAvailable() const
    {
    return 1;
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::LauncWaitNoteL
// -----------------------------------------------------------------------------
//
void CSVGTAppObserverUtil::LaunchWaitNoteL(TInt aResourceId, 
        TBool aVisibilityDelayOff, const TDesC& aFileName )
    {
  
    delete iWaitNote;
    iWaitNote = NULL;
    iProgressInfo = NULL;
    
    if ( aResourceId == R_SVGT_DOWNLOAD_WAIT_NOTE )
        {
        iWaitNote = new (ELeave) CAknWaitDialog( (CEikDialog**)&iWaitNote, 
                                                      aVisibilityDelayOff );
        iWaitNote->PrepareLC(aResourceId);  
        
        TInt resourceId = R_SVGT_DOWNLOADING_FILE;
        HBufC* downloadMsg = StringLoader::LoadLC( resourceId );
        
        TBuf<KMaxFileName> titleString;                
        titleString.Append( downloadMsg->Des() );
        
        CleanupStack::PopAndDestroy( downloadMsg ); // downloadMsg
        
        // Add a force line feed to the note: 
        // qtn_svgt_waiting_downloading [ Downloading file ],
        // and the note will read like this:
        //     Downloading file
        //     filename
        titleString.Append( KNewLineStr );
       
        // Append the filename
        titleString.Append( aFileName );
        
        iWaitNote->SetTextL( titleString );
        
        iWaitNoteState = EDownloadImageWait;                                              
        }
    else if ( aResourceId == R_SVGT_SAVING_WAIT_NOTE )
        {
        iWaitNote = new (ELeave) CAknProgressDialog( (CEikDialog**)&iWaitNote, 
                                                      aVisibilityDelayOff );
        iWaitNote->PrepareLC(aResourceId);
        iProgressInfo = iWaitNote->GetProgressInfoL();
        iProgressInfo->SetFinalValue( KSVGTPercent );
        iWaitNoteState = ESaveFileWait;                                              
        }
    else
        {
        return;
        }
  
    if ( iWaitNote ) 
        {
        iWaitNote->SetCallback(this);
        iWaitNote->RunLD();
        }    
    else
        {
        return;
        }
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::DialogDismissedL
// This is for CAknWaitDialog callback
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::DialogDismissedL( TInt aButtonId )
    {
    
    switch (aButtonId)
        {
        case EAknSoftkeyQuit:
            {
            if ( iWaitNoteState == ESaveFileWait )
                {
                // Handle Save Cancel
                iFileManager->CancelCopy();            
                // Resume the presentation
                iAppDialog->ProcessCommandL( iCommandId );      
                }
            else if ( iWaitNoteState == EDownloadImageWait )
                {
                if ( iWait.IsStarted() )
                    {
                    iSynchronousDownloadStopped = ETrue;
                    iWait.AsyncStop();                              
                    }    
                }
                else
                    {
                    return;
                    }
            break;          
            }
        default:
            break;
        }                  
    }


// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::SVGTSavingDoneL
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::SVGTSavingDoneL( TInt aError )
    {
#ifdef SVGTVIEWERAPP_DBG_FLAG    
    TBuf<40> msg;
    _LIT( errorMsg, "Save Done Err %d" );
    msg.Format(errorMsg, aError );
    PrintDebugMsg( msg );
#endif       
    // If temporary file was used then, perform cleanup
    if ( iTempFileUsed )
        {
        if ( iTempFileName )
            {
            iEikEnv->FsSession().Delete( iTempFileName->Des() );
            delete iTempFileName;
            iTempFileName = NULL;
            }
        iTempFileUsed = EFalse;            
        }
    
    // Indicate to the dialog about the save completion
    iAppDialog->SaveComplete( aError );
    
    if ( !aError )
        {
        iSavingDone = ETrue;
        }
        
    if( iWaitNote )
        {
        if ( !aError && iProgressInfo )
            {
            iProgressInfo->SetAndDraw( KSVGTPercent );
            }
        iWaitNote->ProcessFinishedL();      
        delete iWaitNote;
        iWaitNote = NULL;    
    
#ifdef SVGTVIEWERAPP_DBG_FLAG        
        _LIT( errorMsg, "Proc Finished called" );        
        PrintDebugMsg( errorMsg() );  
#endif          
          
        }
           
    if ( !aError )
        {

        // "saved" note
        HBufC* text = StringLoader::LoadLC(R_SVGT_CLIP_SAVED);
        CAknInformationNote* dlg = new (ELeave) CAknInformationNote( ETrue );
        dlg->ExecuteLD(*text);
        CleanupStack::PopAndDestroy( text );  // text   

        
        }
    else
        {
        iEikEnv->HandleError( aError );
        }
    
    // If move allowed or it is the overwriting self case then             
    if ( IsAllowMove() || iIsOverwriteCase )
        {
        if ( !aError )
            {
            // No error occurred
            
            // File has been moved and source
            // deleted hence don't show save now.    
            iCanSaveContent = EFalse; 
            // If It was Move operation, then Open the new file
            // and give the handle to dialog.
            if ( iSvgFileId->Open(iEikEnv->FsSession(),
                 iSvgDstFileName->Des(), EFileShareAny) != KErrNone )
                {
                // Shouldn't happen. 
                return;
                }
            else
                {
                iAppDialog->StartUsingContentFileHandle();
                }
            }
        else // Error occurred while saving, try to recover
            {
            // If it was Move Operation, and failed, then Open
            // Original Source file again and give the handle to
            // dialog. Note this could fail potentially as the
            // original file could have been deleted.
            if ( iSvgFileId->Open(iEikEnv->FsSession(),
                 iSvgFileName->Des(), EFileShareAny) != KErrNone )
                {
                //Inconsistent state, can't show save also.
                iCanSaveContent = EFalse; 
                return;
                }
            else
                {
                // Can keep showing the save option still.
                iAppDialog->StartUsingContentFileHandle();
                }
            }
        }
    
    if ( !aError )
        {
        if ( iDeleteOrigFile )
            {
            iEikEnv->FsSession().Delete( iSvgFileName->Des() );
            iSvgFileName->Des().Zero();
            }
        // Pass the command id back to UiDialog
        iAppDialog->ProcessCommandL( iCommandId );  
        }
    }

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::SVGTPositionChanged
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTAppObserverUtil::SVGTPositionChanged(TInt aPosition)
    {
    if (iWaitNote && iProgressInfo)
        {
        iProgressInfo->SetAndDraw(aPosition);
        }
    /*
    else if (iFullScreen)
        {
        TRAPD(ignore, iContainer->UpdateBufferingL(aPosition) ); 
        } // (L961)
        */
    }
   
#ifdef SVGTVIEWERAPP_DBG_FLAG    
// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::PrintDebugMsg
// -----------------------------------------------------------------------------
//
void CSVGTAppObserverUtil::PrintDebugMsg( const TDesC&  aMsg )
    {
#ifdef _DEBUG
    RFileLogger::Write( KFileLoggingDir, KFileLog, 
                        EFileLoggingModeAppend, aMsg );
#endif    
    }
#endif

// -----------------------------------------------------------------------------
// CSVGTAppObserverUtil::DoAssignImageDataL
// Reads the downloaded file & gives data to the svg engine.
// -----------------------------------------------------------------------------
//
void CSVGTAppObserverUtil::DoAssignImageDataL(const CSVGTViewerAppDownloads* aDownloadItem,
                                                RHttpDownload& aDownload)
    {
    if( aDownloadItem && !aDownloadItem->ImageLinkFlag() )
        {
        TFileName lFileName;
        // Get the file path of the downloaded file
        aDownload.GetStringAttribute( EDlAttrDestFilename, lFileName );

        TInt lFileSize = 0;
        RFile lSvgFile;
        // Attempt to open the file in read mode
        User::LeaveIfError( lSvgFile.Open(iEikEnv->FsSession(),lFileName,EFileRead ) );
        
        // Save on cleanup stack
        CleanupClosePushL( lSvgFile );
        
        // Get the size of the data to create read buffer
        User::LeaveIfError( lSvgFile.Size(lFileSize) );
        
        // Create buffer that will contain the file data
        HBufC8* lFileData = HBufC8::NewLC(lFileSize);
        TPtr8 lFileDataPtr(lFileData->Des());
        
        // Read from the file
        User::LeaveIfError( lSvgFile.Read(lFileDataPtr) );
        
        // Create buffer to store the URI of the file
        HBufC* lUriBuf = HBufC::NewLC( KMaxUrlLength );
        TPtr lUriPtr(lUriBuf->Des());
        
        // Get the URI from data structure
        aDownload.GetStringAttribute( EDlAttrReqUrl, lUriPtr ); 
        
        // Call the AssignImageData function
        // Mapping is maintained between URI <-> FileData
        iAppDialog->AssignImageData(lUriPtr , lFileData );
        
        CleanupStack::PopAndDestroy( lUriBuf );
        CleanupStack::Pop( lFileData );
        CleanupStack::PopAndDestroy(&lSvgFile); // lSvgFile.Close()
        }
    }

void CSVGTAppObserverUtil::DoAssignLocalFileDataL( const TDesC& aOriginalUri, 
                                                   const TDesC& aNewUri ) const
    {
    TInt lFileSize = 0;
    TBuf<KMaxPath>lFileName;
    lFileName.Copy(aNewUri);
    RFile lSvgFile;
    // Attempt to open the file in read mode
    User::LeaveIfError( lSvgFile.Open(iEikEnv->FsSession(),lFileName,EFileRead ) );
    
    // Save on cleanup stack
    CleanupClosePushL( lSvgFile );
    
    // Get the size of the data to create read buffer
    User::LeaveIfError( lSvgFile.Size(lFileSize) );
    // Create buffer that will contain the file data
    HBufC8* lFileData = HBufC8::NewLC(lFileSize);
    TPtr8 lFileDataPtr(lFileData->Des());
    // Read from the file
    User::LeaveIfError( lSvgFile.Read(lFileDataPtr) );
    
    // Call the AssignImageData function
    // Mapping is maintained between URI <-> FileData
    iAppDialog->AssignImageData(aOriginalUri , lFileData );
    
    CleanupStack::Pop( lFileData );
    CleanupStack::PopAndDestroy(&lSvgFile); // lSvgFile.Close()
    }

EXPORT_C void CSVGTAppObserverUtil::AssignEmbededDataL( const TDesC& aUri )
    {
    HBufC8* decoded = DoGetProcessedEmbededDataL( aUri );
    if ( decoded )
        {
        iAppDialog->AssignImageData( aUri, decoded );
        }
    }
    
HBufC8* CSVGTAppObserverUtil::DoGetProcessedEmbededDataL( const TDesC& aXlinkHrefValue ) const
    {
    HBufC8* retPtr = NULL;
    // find positions for ';' and ',' to determine encoding, mimetype
    TInt startIndex = aXlinkHrefValue.Locate( ';' );
    TInt endIndex = aXlinkHrefValue.Locate( ',' );

    // mimetype is after 'data:' : xlink:href="data:image/png;base64,
    // ignore mimetype extraction, decode will detect mimetype from decoded data.

    if (    startIndex != KErrNotFound && 
            endIndex != KErrNotFound &&
            startIndex < endIndex )
        {
        // extract encoding type
        TPtrC encoding( aXlinkHrefValue.Left( endIndex ).Right( endIndex - startIndex - 1 ) );
        // handle Base64 encoding
        _LIT( KEncodingBase64, "base64" );
        if ( encoding == KEncodingBase64 )
            {
            // find index of first character after white-space
            TInt index = endIndex + 1;
            while ( index < aXlinkHrefValue.Length() && TChar( aXlinkHrefValue[index] ).IsSpace() )
            index++;

            // must be 8-bit
            TInt length = aXlinkHrefValue.Length() - index;
            HBufC8* encoded = HBufC8::NewLC( length );
            encoded->Des().Copy( aXlinkHrefValue.Right( length ) );

            HBufC8* decoded = HBufC8::NewLC( length );
            TPtr8 decodedDes = decoded->Des();

            // decode
            TImCodecB64 encodingBase64;
            TInt decodeError = encodingBase64.Decode( *encoded, decodedDes );

            // Cleanup
            CleanupStack::Pop( decoded ); // encoded, decoded
            CleanupStack::PopAndDestroy( encoded ); 
            //delete encoded;

            // Check for decoding error
            if ( decodeError != KErrNone )
                {
                delete decoded;
                }
            else
                {
                retPtr = decoded;
                }
            }
        }
    return retPtr;
    }
    TBool CSVGTAppObserverUtil::ExitWhenOrientationChange()
    {
    	return EFalse;
    }

//  End of File     
    
