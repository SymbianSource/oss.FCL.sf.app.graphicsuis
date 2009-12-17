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
* Description:  AppUI class for the SVGT Viewer.
*
*/


#ifndef SVGTVIEWERAPPAPPUI_H
#define SVGTVIEWERAPPAPPUI_H

#include <SVGTAppObserver.h>
#include <DownloadMgrClient.h>

// FORWARD DECLARATIONS
class CAknAppUi;
class CSVGTViewerAppAppView;
class CSVGTUIDialog;
class CDownloadMgrUiUserInteractions;       
class CDownloadMgrUiDownloadsList;      
class CDownloadMgrUiLibRegistry;
class CSVGTViewerAppDownloads;
class CSVGTAppObserverUtil;

        
class CSvgtViewerAppView;
// CLASS DECLARATION

/**
*  CSVGTViewerAppAppUi is the AppUI class for the SVGT Viewer
*  @since 3.0
*/
class CSVGTViewerAppAppUi : public CAknAppUi
    {
    public: // Constructors and destructor
        /**
        * C++ default constructor.
        */
        CSVGTViewerAppAppUi();

        /**
        * Second Phase constructor.
        */
        void ConstructL();

        // Destructor
        virtual ~CSVGTViewerAppAppUi();

    public: // New functions
        /**
        * Handle open command from document class.
        * @since 3.0
        * @param  aFileHandle - File Fandle to the SVGT file.
        */
        virtual void OpenFileL( RFile& aFileHandle );

        /**
        * This function is used to set the move option, the
        * Move allowed or not decision is done by the document
        * class and it calls this function to inform the UI class
        * about the decision.
        * @param: aMoveAllowed, ETrue: Move allowed, EFalse: 
        *   Move Not allowed.
        * @since 3.0
        */
        void SetAllowMove(TInt32 aMoveAllowed);

    public: // Functions from base classes
        /**
        * From CAknAppUi
        * @see CAknAppUi::HandleCommandL
        */
        virtual void HandleCommandL( TInt aCommand );
        
        /**
        * From CAknAppUi
        * @see CAknAppUi::OpenFileL
        */
        virtual void OpenFileL( const TDesC& aFilename );
        
        /**
        * From CAknAppUi
        * @see CAknAppUi::ProcessCommandParametersL
        */
        virtual TBool ProcessCommandParametersL(TApaCommand aCommand,
            TFileName& aDocumentName, const TDesC8& aTail );
        
        /**
        * From CAknAppUi
        * @see CAknAppUi::HandleScreenDeviceChangedL
        */
        void HandleScreenDeviceChangedL();
        
        /**
        * From CAknAppUi
        * @see CAknAppUi::HandleResourceChangeL
        */
        void HandleResourceChangeL( TInt aType );
    private: 
        /**
        * From CEikAppUi, called when event occurs of type EEventFocusLost
        * or EEventFocusGained.
        * @param aForeground ETrue if the application is in the foreground,
        * otherwise EFalse
        */
        void HandleForegroundEventL(TBool aForeground);                           
    
        /**
        * Used to find out if video call is going on.
        * @since 3.0
        */   
        TBool IsVideoCall() const;
        /**
        * Used to check if the application has started any embedded app.
        * @since 3.0
        * return: ETrue: If there is an embedded app, EFalse otherwise.
        */ 
        TBool IsEmbeddedAppRunning() const;
        /**
        * Used to set internal state if the application has started 
        * any embedded app.
        * @since 3.0
        */
        void SetEmbeddedAppRunning();
        /**
        * Used to set internal state if the embedded app started 
        * previously gets closed.
        * @since 3.0
        */
        void ResetEmbeddedAppRunning();
        
        /**
        * Used to callback by the idle timer in order to launch the dialog
        * @since 3.0
        */
        static TInt CallBackFuncL(TAny *aPtr);
        
        /**
        * This function accesses the document and launches the content
        * using the UI Dialog.
        * return: ETrue When file handle is not valid
        *         EFalse When file handle is valid
        * @since 3.0
        */
        TBool LaunchDialogL();

        /**
        * This function is used to detect how we are launched. 
        * Embedded or Standalone.
        * return: ETrue: Embedded mode, EFalse: Standalone mode.
        * @since 3.0
        */
        TBool WeAreEmbedded() const;

        /**
        * This function is used to decide if the "save" option
        * should be shown in the UI. "Save" is shown if we
        * are launched from messaging/mms/mail/browser.
        * return: ETrue: Show save, EFalse: Do not show save.
        * @since 3.0
        */
        TBool CanShowSave() const;

        /**
        * This function is used to decide if the Save operation
        * should actually do a move. This is required if we
        * are launched from browser/downloadmgr and file
        * needs to be moved instead of copying to save disk
        * space. 
        * @since 3.0
        */
        TBool ShouldMoveContent() const;

        /**
        * This function checks whether the file is local or remote
        * @since 3.1
        */
        TBool IsLocalFile() const;
        
        /**
        * This function checks whether the filepath of the content 
        * opened is in the PHONE_MEMORY_IMAGES_PATH or 
        * MEMORYCARD_IMAGES_PATH
        * @param aFileHandle  Filehandle to check the path
        * @since 3.1
        */
        TBool IsLocalFileL( RFile& aFileHandle ) const;

    private:    // Data

        CSvgtViewerAppView* iContainer;
        
        // Pointer to the SVGT Dialog
        CSVGTUIDialog* iAppDialog;        
        
        // Pointer to Hyperlink Handler
        CSVGTAppObserverUtil*  iAppObserver;    

        TBool iEmbeddedAppRunning;
        CPeriodic* iIdleTimer;
        TBool iMoveAllowed;
    };


#endif // __SVGTVIEWERAPPAPPUI_H__

//End of File
