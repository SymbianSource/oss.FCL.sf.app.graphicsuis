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
* Description:  SVGT UI Dialog implements a custom dialog used to display
*                SVGT content.
*
*/


#ifndef __SVGTUIDIALOG_H__
#define __SVGTUIDIALOG_H__
#include <akndialog.h>
#include <aknnavide.h> 
#include <SVGTCustControl.h> 
#include <SvgtApplicationExitObserver.h>

// FORWARD DECLARATIONS
class CSVGTCustControl;
class MSvgtAppObserver;
class MSVGTProgressBarDrawImpl;
class TCoeHelpContext;

/**
*  Provides the dialog functionality for viewing SVGT files.
*  @lib SVGTUIControl.lib
*  @since 3.0
*/
 
class CSVGTUIDialog : public CAknDialog, 
                      public MSVGTProgressBarDrawImpl,
                      public MSVGTMSKImpl,		//Added since 3.2 interface for MSK label impl
                      public MSvgtApplicationExitObserver,
                      public MSvgAudioMuteListener,
                      public MCustControlPointerEventHandler
    {
    public:  // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CSVGTUIDialog* NewL();

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CSVGTUIDialog* NewLC();
        
        

        // Destructor
        virtual ~CSVGTUIDialog();
    public: // New functions
    public: // Functions from base classes
        /**
        * From CAknDialog Executes the dialog and deletes the dialog on exit.
        * @since 3.0
        * @param aFileHandle File handle to the SVGT content.
        * @param aAppObs Implementation Class provided by application that
        *   uses this dialog
        * Note: The ownership of the file handle is not transferred. 
        *       It is the responsibility of the caller to close this file 
        *       after deleting this dialog.        
        * @return Integer - Indicates Exit Reason.
        */
        virtual TInt ExecuteLD( RFile& aFileHandle,
                                MSvgtAppObserver  *ptr );
                                
        /**
        * From CAknDialog - Called by framework to process commands invoked by
        *  the user.
        */
        virtual void ProcessCommandL( TInt aCommandId );
                                
    public: //From MSvgtApplicationExitObserver
        void HandleApplicationExitL( TInt aExitCommand );
        IMPORT_C void AssignImageData( const TDesC& aUri, HBufC8* aData );
        
    public:// From MSvgAudioMuteListener
    		/**
        *  This method is to remove the muted icon
        * @since 5.0
        * @param aIsVolumeMuted: indicates if volume is to be muted
        * @return none.
        */
        void VolumeMuted(TBool aIsVolumeMuted);
    public:                                           
               
        
        /**
        * This function is called by the application to check whether the
        * loading of the svg content is done.
        * @since 3.0
        * @return TBool iIsLoadingDone
        */                                       
        virtual TBool IsLoadingDone();   
        /**
        * Function used by the application to handle background/foreground
        * events. When the application goes to background the presentation
        * needs to be paused. When application comes back to foreground,
        * the user would need to start the paused presentation. 
        * @param aForeground: ETrue: Foreground event, 
        *                     EFalse: Background event.
        * @since 3.0
        */
        virtual void HandleApplicationForegroundEvent( TBool aForeground );

        /**
        * Function used by the client to indicate the filehandle is
        * no longer going to be valid. e.g. In Move operation of the
        * file, client could close old file and open new one after
        * move is over.
        * @since 3.0
        */
        virtual void StopUsingContentFileHandle();
        /**
        * Function used by the client to indicate the filehandle is
        * going to be valid now. e.g. In Move operation of the
        * file, client could close old file and open new one after
        * move is over.
        * @since 3.0
        */
        virtual void StartUsingContentFileHandle();

        /**
        * Function used by the client to check if text in the content
        * was modified, to check if save is needed.
        * @since 3.1
        */
        virtual TBool IsTextContentChanged();
        
        /**
        * Function used by the client to save the modified svg content
        * @since 3.1
        */
        virtual TInt SaveSvgDom( const TDesC& aFileName );

        /**
        * Function used by the client to notify dialog that save is complete
        * @since 3.1
        */
        virtual void SaveComplete( TInt aError );

    protected: // New functions
        /**
        * Convert a time given in milliseconds to a descriptor.
        * The format depends on how big the given duration is.
        * Durations of less than an hour are formatted using
        * the short format (02:24). Longer durations are formatted
        * using the long format (1:02:04). Maximum duration is 10 hours.
        *
        * @since 3.0
        * @param aDuration duration in seconds
        * @param aBuf      descriptor to hold the converted string
        *
        */
        void FormatTimeL( const TUint& aDuration,
                          TDes& aBuf ) const;
        
        /**
        * This function is called when the user presses back or Exit.
        * prompts a confirmation query to the user and then calls DoSaveL
        * @param aButtonId  
        * @since 3.0
        */                          
        TBool SaveNeededL( TInt aButtonId );
        
        /**
        * This function prompts a save confirmation query to the user     
        * @since 3.0
        */  
        TBool LaunchSaveQueryDialogL() const;        
        
        /**
        * This function changes the layout of the dialog to full screen mode     
        * @since 3.0
        */  
        void SwitchDialogLayoutL( TBool aIsFullScreen );        
        
    protected: // Functions from base classes
        // From CEikDialog
        // From CAknDialog

        /**
        * From CAknDialog - Called by framework to dynamically manipulate
        *  the menu for the dialog.
        */
        void  DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );
        
        /**
        * From CAknDialog - Framework method to determine if it OK
        *  to exit the dialog.
        */
        TBool OkToExitL( TInt aButtonId );
        
        /**
        * From CAknDialog - Called by framework to add custom control to the
        *  dialog when specified in resource.
        */
        SEikControlInfo CreateCustomControlL( TInt aControlType );
       
        /**
        * From CAknDialog - Called by framework to notify resize of dialog.
        */
        void SizeChanged();

        /**
        * From CCoeControl Key Handling Method for control.
        * @since 3.0
        * @param aKeyEvent Key Event.
        * @param aType Type of Key Event.
        * @return TKeyResponse - EKeyWasConsumed/EKeyWasNotConsumed.
        */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                     TEventCode aType );
      
#ifdef __SERIES60_HELP        
        /**
        * From CCoeControl Context Sensitive Help ID retrieving function.
        * @since 3.0
        * @see CCoeControl
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;
#endif        
        /**
        * From CCoeControl 
        * @see CCoeControl::HandleResourceChange
        */
        void HandleResourceChange( TInt aType );
        
        /**
        * From MSVGTProgressBarDrawImpl. This function is called in 
        * order to render the progress indicator.
        * @since 3.0
        * @param aTimeElapsed - Contains the time elapsed in seconds.
        * @param aTotalTime - Contains the total time in seconds
        */
        void DrawProgressBarL( TUint aTimeElapsed, TUint aTotalTime );
        
        /**
        * Sets the mute icon & text to the navi pane .
        * @since 3.2
        * @param aIsVolumeMuted whether to show the the mute icon.
        * @param aText The text show the progress info.
        */
        void SetNaviPaneInfoL(TBool aIsVolumeMuted, const TDesC& aText);
        
    private:
    	
    	static TInt ButtonContainerTimerCallBack(TAny* aAny);
        virtual TBool HandleCustControlPointerEventL(const TPointerEvent& aPointerEvent);

    	/**
    	* From MSVGTMSKImpl. This function is called to draw the MSK label by
    	* pushing the command to position button's stack.
	    * @since 3.2
	    * @param aResourceID - The MSK label resourceID
	    * @param aCommandID	- The command associated with that label
	    */
    	void DrawMSKLabelL(TInt aResourceID,TInt aCommandID);
    	
    	/**
    	* From MSVGTMSKImpl. This function is called to remove MSK label from
    	* button group's stack
		* @since 3.2
		*/
    	void RemoveMSKLabel();
    	
        /**
        * C++ default constructor.
        */
        CSVGTUIDialog();

        /**
        * Second Phase constructor.
        */
        void ConstructL();

    private:    // Data
        CPeriodic* iBtnCntrTimer;
        
        TBool iIsButtonContainerVisible;
         
        TSize iBtnGrpSize;
        TPoint iBtnGrpPos;
        // Descriptor that contains the path to the SVGT file.
        RFile* iFileHandlePtr;

        // Is the Content file handle valid.
        TBool iIsFileHandleValid;

        // Integer storing resource offset to DLL resource file
        TInt iResourceOffset;

        // Pointer to Custom CCoeControl.
        CSVGTCustControl *iCustControl;

        // Pointer to application implementation of callbacks.
        MSvgtAppObserver *iAppObserver;

        // Boolean Indicating whether Save functionality is required.
        TBool iSaveEnable;
        
        // Boolean to store whether the content is preview or not
        TBool iPreview;
        
        // Navigation Pane - for progress indicator display
        CAknNavigationDecorator* iNaviDecorator;
        // Application Title Text
        HBufC* iAppTitleText;

        // Short Time format - initialised from resource
        HBufC* iTimeFormatShort;

        // Long Time format string - initialised from resource
        HBufC* iTimeFormatLong;        
        //Button group container - for MSK label updation 
        CEikButtonGroupContainer* iCbaGroup;
        
        HBufC* iProgressText;
        
        TBool iIsVolumeMuted;

        TInt iLastCommandID;
        TBool iQuietExitWhenOrientationChange;
    };

#endif // __SVGTUIDIALOG_H__
// End of File
