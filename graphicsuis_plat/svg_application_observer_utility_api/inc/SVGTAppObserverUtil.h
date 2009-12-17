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
* Description:  This class provides a reference implementation for the SVGT 
*                UI Control Application Observer. 
*
*/


#ifndef SVGTAPPOBSERVERUTIL_H
#define SVGTAPPOBSERVERUTIL_H

#include <e32base.h>
#include <e32std.h>
#include <aknprogressdialog.h>
#include <CDownloadMgrUiUserInteractions.h>
#include <CDownloadMgrUiDownloadsList.h>
#include <CDownloadMgrUiLibRegistry.h>

#include <SVGTAppObserver.h>
#include <DownloadMgrClient.h>
#include <SVGTUISaveListener.h>
#include <SVGTFileManager.h>
#include <SVGTViewerAppDownloads.h>
#include <SVGTViewerAppDbgFlags.hrh>

    
// Forward Declarations
class CSVGTUIDialog;
class CEikProgressInfo;
class CDocumentHandler;

// CLASS DECLARATION

/**
*  This class is used as a utility class for implementing observer for the 
*  SVGT UI Control.
*  It provides helper function/reference implementation for following 
*  functionality: 
*           a. Hyperlink handling 
*           b. Download manager + UI Usage
*           c. Save Option Implementation
*           d. Misc (GetSmilFitValue)
*  @lib SVGTAppObserverUtil
*  @since 3.0
*/

class CSVGTAppObserverUtil : public CBase,
                             public MSvgtAppObserver,
                             public MHttpDownloadMgrObserver,
                             public MSVGTUISaveListener,
                             public MProgressDialogCallback
    {
    
    public:
    
    enum TDownloadNeeded 
        {
        // The uri refers to a local image file
        ELocalImageFile,
        // The uri refers to a remote image file
        ERemoteImageFile,
        // The uri doesnt refer to an image file
        ENotAnImageFile
        };
    
    enum TWaitNoteState
        {
        // WaitNote is used for file saving
        ESaveFileWait = 1,
        // WaitNote is used for downloading image synchronously
        EDownloadImageWait
        };
        
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * aAppDialog: Pointer to the SVGT Dialog
        * RFile* aFileHdlPtr: File handle to the content.
        * const TDesC& filename: Filepath.
        * TBool aIsFileFullPathAvailable: Flag to indicate if content's 
        *   fullpath is available.
        * TBool aCanSaveContent,
        * TBool aShouldMoveContent.
        */
        IMPORT_C static CSVGTAppObserverUtil* NewL( CSVGTUIDialog* aAppDialog,
                        RFile* aFileHdlPtr,
                        const TDesC& aFilename, 
                        TBool aIsFileFullPathAvailable,
                        TBool aCanSaveContent,
                        TBool aShouldMoveContent );

        /**
        * Two-phased constructor.
        * aAppDialog: Pointer to the SVGT Dialog
        * RFile* aFileHdlPtr: File handle to the content.
        * const TDesC& aFilename: Filepath.
        * TBool aIsFileFullPathAvailable: Flag to indicate if content's 
        *   fullpath is available.
        * TBool aCanSaveContent,
        * TBool aShouldMoveContent.
        */
        static CSVGTAppObserverUtil* NewLC( CSVGTUIDialog* aAppDialog,
                        RFile* aFileHdlPtr,
                        const TDesC& aFilename, 
                        TBool aIsFileFullPathAvailable,
                        TBool aCanSaveContent,
                        TBool aShouldMoveContent); 

        /**
        * Two-phased constructor.
        * aAppDialog: Pointer to the SVGT Dialog
        * RFile* aFileHdlPtr: File handle to the content.
        * @since 3.0
        */
        IMPORT_C static CSVGTAppObserverUtil* NewL( CSVGTUIDialog* aAppDialog,
                        RFile* aFileHdlPtr );

        /**
        * Two-phased constructor.
        * aAppDialog: Pointer to the SVGT Dialog
        * RFile* aFileHdlPtr: File handle to the content.
        * @since 3.0
        */
        static CSVGTAppObserverUtil* NewLC( CSVGTUIDialog* aAppDialog,
                        RFile* aFileHdlPtr ); 
        /**
        * Destructor.
        */
        IMPORT_C virtual ~CSVGTAppObserverUtil();
        
        
        /**
        * This method is called by the svg engine when there is an image tag        
        * encountered in the svg file. This function parses the uri, checks
        * whether the uri is a local file or a remote file. If the file is 
        * a remote file, the file is downloaded. Fetched file is then opened
        * and the file handle returned to the engine.       
        * @since 3.0
        * @see MSvgRequestObserver::FetchImage
        */                                       
        IMPORT_C virtual TInt FetchImage( const TDesC& aUri, 
             RFs& aSession, RFile& aFileHandle );                                  
 
        TInt FetchFont( const TDesC& /* aUri */, 
                                    RFs& /* aSession */, RFile& /* aFileHandle */ );
	   
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::LinkActivated
        */
        IMPORT_C virtual TBool LinkActivated( const TDesC& aUri );

        
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::LinkActivatedWithShow
        */
        IMPORT_C virtual TBool LinkActivatedWithShow( const TDesC& aUri, 
                                             const TDesC& aShow );
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::GetSmilFitValue
        */
        IMPORT_C virtual void GetSmilFitValue( TDes& aSmilValue );
        
        /**
        * This function is called by the application to enable/disable the
        * downloads menu in the options menu
        * @since 3.0
        * @param aAddDownloadMenu enable/disable the downloads menu  
        */   
                                 
        IMPORT_C void SetDownloadMenu( TBool aAddDownloadMenu );
        
        /**
        * This function is called by the Ui Dialog to enable/disable the
        * downloads menu in the options menu
        * @since 3.0
        * @return iDownloadMenuEnabled enable/disable the downloads menu  
        */   
        IMPORT_C virtual TBool CanShowDownload() const;
        

        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::DisplayDownloadMenuL
        */ 
        IMPORT_C virtual void DisplayDownloadMenuL();   
                                                           
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::CanShowSave
        */
        IMPORT_C virtual TBool CanShowSave();
        
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::IsSavingDone
        */
        IMPORT_C virtual TBool IsSavingDone();

        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::DoSaveL
        */ 
        IMPORT_C virtual void DoSaveL( TInt aCommandId ); 
        
        IMPORT_C TInt NewFetchImageData( const TDesC& aUri );
        IMPORT_C void AssignEmbededDataL( const TDesC& aUri );
		TBool ExitWhenOrientationChange( );
        
        /**
        * Helper function for DoSaveL
        */
        void SaveContentL(  RFile& aSrcFileHandle, const TDesC& aSrcFileName,
                            TDes& aDstFileName, TBool aMoveFlag );
        /**
        * This function generates a unique file name in the private directory
        * @param aName On return contains the full path on return.
        */    
        void GetUniqueFileName( TDes& aName ) const;
        
        /**
        * Checks whether the file is a GZipped content
        * @param aFileHandle File handle to the SVG content
        */
        TBool IsGzipContent( const RFile& aFileHandle ) const;
                                    
        /**
        * This function saves the SVG content into the private directory
        */           
        void SaveSvgTextContentL( const RFile& aSrcFileHandle, 
            TDes& aTempFileName );
                                  
    public: // Callback from the MHttpDownloadMgrObserver                                    
        /**
        * This method is a call back function used by the download manager
        * to indicate the completion/failure of the download started.
        * LinkActivatedWithShow function
        * @since 3.0
        * @param : aDownload -- handle of the download
        * @param : aEvent -- download event      
        */
        IMPORT_C void HandleDMgrEventL( RHttpDownload& aDownload,
                                       THttpDownloadEvent aEvent );                                                
    public: // Callback from the MSVGTUISaveListener         
       /**
        * From MSVGTUISaveListener. This function is called 
        * when buffering or saving position is changed.
        * @since 3.0
        * @param  aPosition, New buffering/saving position (0-100)
        */
        IMPORT_C virtual void SVGTPositionChanged( TInt aPosition );

        /**
        * From MSVGTUISaveListener. Called when saving is finished.
        * @since 3.0
        * @param  aError, KErrNone or one of the system wide error codes.
        * @return void
        */
        IMPORT_C virtual void SVGTSavingDoneL( TInt aError ); 
         
    public: // Functions from MProgressDialogCallback
        
        /** @see MProgressDialogCallback::DialogDismissedL */
        IMPORT_C void DialogDismissedL( TInt aButtonId );

    public: // Leaving variants of linkActivated and linkActivatedWithShow
        
        /**
        * This method is called by LinkActivated.
        * If this function leaves, it is trapped by the linkActivated function
        * @since 3.0        
        * @param : aUri -- URI string of hyperlink        
        */
        IMPORT_C void LinkActivatedL( const TDesC& aUri );

        /**
        * This method is called by LinkActivatedWithShow.
        * If this function leaves, it is trapped by the
        * LinkActivatedWithShow function
        * @since 3.0        
        * @param : aUri -- URI string of hyperlink
        * @param : aShow -- xlink:show value        
        */
        IMPORT_C void LinkActivatedWithShowL( const TDesC& aUri , 
                                              const TDesC& aShow ); 
    public:                                        
        
        /**
        * This method is called by LinkActivated(WithShow), FetchImage
        * This checks whether the uri passed is a local image file or
        * a remote image file or not an image file.
        * @since 3.0
        * @param : aUri -- URI string of hyperlink                 
        * @return : TDownloadNeeded 
        *           ELocalImageFile - if the uri is a local file
        *           ENotAnImageFile - If the uri is not an image file
        *           ERemoteImageFile - If the uri is remote image file.
        */
        IMPORT_C static TDownloadNeeded IsDownloadNeededL( 
                                           const TDesC& aUri );
        
        /**
        * This method is called by LinkActivated(WithShow), FetchImage
        * This gets the filename from the uri and the 
        * @since 3.0
        * @param : aUri -- URI string of hyperlink                 
        * @return : ETrue if the local file name is retrieved
        *           EFalse if the file could not be retrieved
        */        
        IMPORT_C TBool GetFileNameFromUri( const TDesC& aUri, 
                                  TDes& aFileName ) const;
                                  
        /**
        * This method is called by LinkActivated(WithShow), FetchImage
        * This gets the filename with full path from the uri and the 
        * @since 3.0
        * @param : aUri -- URI string of hyperlink                 
        * @return : ETrue if the local file name is retrieved
        *           EFalse if the file could not be retrieved
        */
        IMPORT_C static TBool GetLocalFile( const TDesC& aUri,  
                                            TDes& aFileName,
                                            const TDesC& aSvgFileName );
                
        
    protected: //helper functions for fetchImage function    
           
        /**
        * This method is called by LinkActivated(WithShow),FetchImage functions
        * If this function leaves, it is trapped by the
        * Called function
        * @since 3.0        
        * @param : aUri -- URI string of hyperlink
        * @param : aSession - File server session in which the file is opened
        * @param : aFileHandle - handle of the file downloaded.   
        * @param : aIsSynchronous - Flag to indicate synchronous/asynchronous 
        *                           download
        * @param : aIsEmbedded - Flag to indicate whether the downloaded 
        *           file needs to be opened in embedded mode or not.
        * @return : KErrNone if successful
        *           KErrNotFound if not
        */
        TInt StartDownloadingImageL(                    
                    const TDesC& aUri, 
                    RFs& aSession, 
                    RFile& aFileHandle,     
                    TBool aIsSynchronous,
                    TBool aLinkFlag,
                    TBool aIsEmbedded = ETrue );                
                            
   
        /**
        * This method is called by the Fetch Image
        * before starting the download to get Access point id.
        * @since 3.0
        * @param : aIapId -- Access point id Found         
        * @return : True if Iap found else returns false.
        */
        IMPORT_C TBool GetDefaultInetAccessPointL
                              ( TUint32& aIapId ) const;
                              
        /**
        * This method is called by HandleDMgrEventL
        * This makes use of Document Handler to launch the file.
        * @since 3.0
        * @param : aFileName -- Name of the file to be launched. 
        * @param : aIsEmbedded -- Flag to indicate whether file has to 
        *          be opened in embedded mode or independent mode        
        */                     
        IMPORT_C void LaunchDownloadedFileL( const TDesC& aFileName, 
                                        TBool aIsEmbedded,  
                                        TBool aAllowSave ) const;  
                                        
   
#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED   
        /**
        * This method is called by HandleDMgrEventL
        * This iterates through the linked list and return the link 
        * corresponding to the given http download handle
        * @since 3.0
        * @param : aHttpDownload -- handle of hyperlink         
        * @return : CSVGTViewerAppDownloads * the link corresponding to 
        * the given httpDownload handle.
        */
        CSVGTViewerAppDownloads* FindDownload( 
                                 const RHttpDownload* aHttpDownload ); 
#endif     
        /**
        * This method is used to get back the filename of the currently
        * playing svg content in filname.ext format.
        * @since 3.0
        * @param : TDes& filename: Parameter where filename needs to be
        *   returned.
        * @return : void.
        */
        IMPORT_C void GetFileNameAndExt(TDes& filename) const;

        /**
        * This method is used in Save operation to decide whether
        * move action should be performed.
        * @since 3.0
        * @param : None.
        * @return : ETrue: Content needs to be moved. EFalse: Content
        *   shouldn't be moved.
        */
        IMPORT_C TBool IsAllowMove() const;

        /**
        * This method is used to launch wait dialog while save operation
        * is going on.
        * @since 3.0
        * @param : aResourceId: Resource to be used for wait-note.
        *   aVisibilityDelayOff: Parameter to the wait note.
        *   aFileName FileName to be displayed in WaitNote
        * @return : none.
        */
        void LaunchWaitNoteL(TInt aResourceId, 
                             TBool aVisibilityDelayOff, 
                             const TDesC& aFileName );

        /**
        * This method is used check whether content file's full-path
        * is available.
        * @since 3.0
        * @return : ETrue: Fullpath available, EFalse: Fullpath
        *   not available.
        */
        IMPORT_C TBool IsFileFullPathAvailable() const;
    
    protected:

        /**
        * C++ default constructor.
        * aAppDialog: Pointer to the SVGT Dialog
        */
        IMPORT_C CSVGTAppObserverUtil( CSVGTUIDialog* aAppDialog ); 

        /**
        * Default Symbian 2nd phase constructor.
        */
        IMPORT_C void ConstructL();

        /**
        * Default Symbian 2nd phase constructor.
        * RFile* aFileHdlPtr: File handle to the content.
        * const TDesC& filename: Filepath.
        * TBool aIsFileFullPathAvailable: Flag to indicate if content's 
        *   fullpath is available.
        * TBool aCanSaveContent,
        * TBool aShouldMoveContent.
        */
        IMPORT_C void ConstructL( RFile* aFileHdlPtr,
                        const TDesC& aFilename, 
                        TBool aIsFileFullPathAvailable,
                        TBool aCanSaveContent,
                        TBool aShouldMoveContent );  
        
        /**
        * Default Symbian 2nd phase constructor.
        * RFile* aFileHdlPtr: File handle to the content.
        */
        IMPORT_C void ConstructL( RFile* aFileHdlPtr );  

#ifdef SVGTVIEWERAPP_DBG_FLAG        
        void PrintDebugMsg( const TDesC&  aMsg );
#endif        

    private: //User methods
        /**
        * Reads the downloaded file & assign to the svg engine.
        * @since 3.1
        * @param CSVGTViewerAppDownloads aDownloadItem
        * @param RHttpDownload aDownload
        */
        void DoAssignImageDataL(const CSVGTViewerAppDownloads* aDownloadItem,
                                                RHttpDownload& aDownload);
        
        void DoAssignLocalFileDataL( const TDesC& aOriginalUri,  const TDesC& aNewUri ) const;
        HBufC8* DoGetProcessedEmbededDataL( const TDesC& aXlinkHrefValue ) const;
        
    private:    // Data            
  
#ifdef SVGTVIEWERAPP_DOWNLOADMGR_ENABLED   
        // To start a wait loop after initiating download
        CActiveSchedulerWait iWait; 
         
        // client side handle for the download manager        
        RHttpDownloadMgr iDownloadMgr;
        
        // Handle for download manager ui user interactions
        CDownloadMgrUiUserInteractions* iDMgrUserInteractions;
        
        // Handle for download manager ui downloads list
        CDownloadMgrUiDownloadsList* iDMgrDownloadsList;                        
        
        // Handle to register download manager UI
        CDownloadMgrUiLibRegistry* iDMgrUiReg;

        // Head of Doubly linked list of Downloads
        TDblQue<CSVGTViewerAppDownloads>  iDownloads;
        
        // Iterator for the doubly linked list
        TDblQueIter<CSVGTViewerAppDownloads>   iDownloadIter;
        
        // Flag to check whether downloads menu is enabled or not
        TBool iDownloadMenuEnabled;
        
        // Flag to check whether synchronous download is stopped
        TBool iSynchronousDownloadStopped;
#endif 
        // Pointer to the SVGT Dialog
        CSVGTUIDialog* iAppDialog;   
        
        //File Manager used for file save operations.
        CSVGTFileManager* iFileManager;

        TBool  iIsFullFilePathValid;
        TBool  iCanSaveContent;
        TBool  iShouldMoveContent;
        HBufC* iSvgFileName;
        HBufC* iSvgDstFileName;
        RFile* iSvgFileId;
        CAknProgressDialog* iWaitNote;
        
        // enumeration to store the state in which the wait note is used.
        TWaitNoteState iWaitNoteState;        
               
        CEikProgressInfo* iProgressInfo;  
                   
        // Integer storing resource offset to DLL resource file
        TInt iResourceOffset;
        
        // Boolean indicating save done
        TBool iSavingDone;
        
        // Integer to store the command id in context while saving
        TInt iCommandId;
        
        // DocHandler pointer to Open embedded files
        CDocumentHandler* iDocHandler;

        // pointer to  CEikonEnv
        CEikonEnv* iEikEnv;
        //TBool iIsFetchImageCall;
        
        // Boolean to remember whether original file is to be 
        // deleted after save
        TBool iDeleteOrigFile;

        // Boolean to remember whether temp file was used 
        // to perform cleanup
        TBool iTempFileUsed;

        // Used for storing the temporary file name for deletion
        HBufC* iTempFileName;
        // Used to indicate whether we are attempting to overwrite existing
        // file
        TBool iIsOverwriteCase;
        TBool iDownloadDone;
        
        // This variable tracks whether the download manager was initialised or not 
        TBool iDownloadInProgress;
        
    };

#endif // SVGTAPPOBSERVERUTIL_H
// End of File
