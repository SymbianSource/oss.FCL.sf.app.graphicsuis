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
* Description:  SVGT Custom Control Class implements the custom control used
*                to display SVGT content.
*
*/



#ifndef __SVGTCUSTCONTROL_H__
#define __SVGTCUSTCONTROL_H__

// INCLUDES

#include <e32property.h> 
#include <SVGAnimationListener.h>
#include <SVGEngineInterfaceImpl.h>
#include <SVGRequestObserver.h>
#include <AknUtils.h>

#include <SVGListener.h>
#include <SVGHyperlinkListener.h>

#include <SVGTUIControlDbgFlags.hrh>
#include <SVGTProgressBarTimer.h>
#include <SVGTPointerHideTimer.h>
#include <e32std.h>

#ifdef RD_SVGT_AUDIO_SUPPORT
#include<remconcoreapitargetobserver.h>    //for MRemConCoreApiTargetObserver
#endif //RD_SVGT_AUDIO_SUPPORT

// CONSTANTS
const TInt KFindTextQueryMaxLen = 50;
// MACROS

// DATA TYPES

// Enumeration for indicating animation Status of current SVGT Content
enum TSvgtAnimStatus {
        ESvgNoAnimationsPresent,
        ESvgAllAnimationsComplete,
        ESvgAnimationsNotComplete };

// Enumeration for indicating Animation State of the viewer
enum TSvgtViewerAnimStatus
        {
        ESvgAnimPausedState,
        ESvgAnimPlayingState,
        ESvgAnimStoppedState
        };

// Enumeration for indicating pointer types in the viewer
enum TSvgtViewerPointerTypes
            {
            ESvgPointerDefault,
            ESvgPointerHand,
            ESvgPointerText
            };        
            
        
enum TSvgCustomControlFlag
        {
        ESvgDisplayWaitNote   = 0x01,    // need to draw initial wait note
        ESvgDisplayErrorNotes = 0x02,    // Display Error Notes
        ESvgDisplayIcons      = 0x04,    // Display Icons
        ESvgWindowOwning      = 0x08,    // Window Owning Control
        ESvgDrawPointer       = 0x10,    // Pointer is drawn
        ESvgDrawPanIndicator  = 0x20,	 // Panning Indicator is drawn
        ESvgDrawPauseIndicator = 0x40,   // Pause Indicator is drawn
        ESvgDrawProgressBar    = 0x80,    // ProgressBar drawn
        ESvgUseBackLight       = 0x100    // Keep Backlight on while animating
        };        

enum TSvgCustomControlSelectCause
    {
    ESvgInvalidSelectCause,
    ESvgHyperLinkSelectCause,
    ESvgTextSelectCause,
    ESvgTextAreaSelectCause    
    };
        
// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class   CSvgEngineInterfaceImpl;
class   CSVGTHyperlinkHandler;
class   CCoeControl;
class   CSVGTProgressBarTimer;
class   MSvgtAppObserver;
class   CAknWaitDialog;
class   CSVGTDrmHelper;
class   CSvgtThreadController;
class   CAknInformationNote;
class   MSvgtApplicationExitObserver;
class   CSvgtEventHandlerAO;
class   CAknNavigationControlContainer;
class   CAknNavigationDecorator;

#ifdef RD_SVGT_AUDIO_SUPPORT
class   CAknVolumePopup;
class CRemConCoreApiTarget;
class CRemConInterfaceSelector;
#endif //RD_SVGT_AUDIO_SUPPORT

// CLASS DECLARATION

/**
*  Interface for the progress timer event listener.
*  @lib SVGTUIControl.lib
*  @since 3.0
*/

class MSVGTProgressBarDrawImpl 
    {
    public:
    
        /**
        * This function is called in order to render the progress indicator.
        * @since 3.0
        * @param aTimeElapsed - Contains the time elapsed in seconds.
        * @param aTotalTime - Contains the total time in seconds
        */
        virtual void DrawProgressBarL( TUint aTimeElapsed, 
            TUint aTotalTime ) = 0;
    };

/**
*  Interface for MSK labelling   
*  @lib SVGTUIControl.lib
*  @since 3.2
*/

class MSVGTMSKImpl
{
	public:
	
	/**
	* This function is called to draw the MSK label by pushing the command to
	* position button's stack
	* @since 3.2
	* @param aResourceID - The MSK label resourceID
	* @param aCommandID	- The command associated with that label
	*/ 							
	virtual void DrawMSKLabelL(TInt aResourceID,TInt aCommandID)=0;
    	
    /**
    * This function is called to remove MSK label from button group's stack
	* @since 3.2
	*/
	virtual void RemoveMSKLabel()=0;
	
};


class MCustControlPointerEventHandler
    {
public:
    virtual TBool HandleCustControlPointerEventL(const TPointerEvent& aPointerEvent) = 0;    
    };


class MSvgCustControlListener
{
};

enum TCustControlListener
	{
	EAudioListener = 0	
	};
#ifdef RD_SVGT_AUDIO_SUPPORT
class MSvgAudioMuteListener : public MSvgCustControlListener
{
	public:
	virtual void VolumeMuted(TBool aIsVolumeMuted) = 0;
};
#endif
/**
*  CSVGTCustControl is a CCoeControl used to display SVGT content.
*  It can be used independently or through dialog provided by
*  CSVGTUIDialog.
*
*  @lib SVGTUIControl.lib
*  @since 3.0
*/

class CSVGTCustControl : public CCoeControl,
                         public MSvgRequestObserver,
                         public MSvgAnimationListener,
                         public MSVGTProgressBarListener,
                         public MSVGTPointerInactivityListener,
                         public MSvgHyperlinkListener
#ifdef SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE
                         , public MSvgTextAreaListener,
                         public MSvgTextListener
#endif // SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE                         
                         , public MSvgLoadingListener
                         
#ifdef RD_SVGT_AUDIO_SUPPORT
                         , public MCoeControlObserver
                         , public MRemConCoreApiTargetObserver
#endif //RD_SVGT_AUDIO_SUPPORT
                         , public MSvgInteractiveElementListener
                         ,public MSvgViewPortListener
    {
    public:
        enum TFileType
            {
            // The uri refers to a local image file
            ELocalImageFile,
            // The uri refers to a remote image file
            ERemoteImageFile,
            // The uri doesnt refer to an image file
            ENotAnImageFile,
            // The uri is not well formed(parsing error)
            EInvalidFilePath
            };
    public: // Constructors and destructor

    MCustControlPointerEventHandler* iPntrObserver;
    void SetPointerObserver(MCustControlPointerEventHandler* aPntrObserver)
        {
        iPntrObserver = aPntrObserver;
        }
            
    
//TOUCH SUPPORT START    
#ifdef RD_SCALABLE_UI_V2
        //From CCoeControl
		void HandlePointerEventL(const TPointerEvent& aPointerEvent);
        void SetContainerWindowL(const CCoeControl &aContainer);
        
		enum TIndicatorType
		    {
		    ELeftIndicator,
		    ERightIndicator,
		    EUpIndicator,
		    EDownIndicator,
		    EPauseIndicator, //for future use
		    ENoIndicator
		    };
		    
		TIndicatorType ActiveIndicator(const TPoint aPosition ) const;
		TRect ActiveIndicatorRect( TIndicatorType aIndicator) const;
		
		void PanPresentation(const CSVGTCustControl::TIndicatorType aIndicatorType );
		
#endif  //RD_SCALABLE_UI_V2
		
#ifdef RD_SVGT_AUDIO_SUPPORT
        //From MCoeControlObserver
        virtual void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        
		void SetPresentationVolume( TInt aVolumeLevel );
		TInt PresentationVolume() const;
		
		TInt GetPrevVolume() const;
        void MrccatoCommand(TRemConCoreApiOperationId aOperationId, 
                             TRemConCoreApiButtonAction aButtonAct );
                             
        void FilterAndSendCommand(TInt aCommandId);
        
#endif	//RD_SVGT_AUDIO_SUPPORT
//TOUCH SUPPORT END

        /**
        * Two-phased constructor.
        * @since 3.0
        * @param aFileHandle RFile that indicates the SVGT content filehandle
        * @param aAppObs Implementation Class provided by application that
        *   uses this control
        * @param aPBDrawImpl Implementation for rendering progress Indicator
        * @param aMSKImpl Implementation for drawing MSK label 
        * Note: The ownership of the file handle is not transferred. 
        *       It is the responsibility of the caller to close this file 
        *       after deleting this control.
        * @return CSVGTCustControl*
        */

        IMPORT_C static CSVGTCustControl* NewL( RFile& aFileHandle,
                                       MSvgtAppObserver* aAppObs, 
                                       MSVGTProgressBarDrawImpl* aPBDrawImpl,
                                       MSVGTMSKImpl* aMSKImpl,
                                       MSvgtApplicationExitObserver* 
                                            aExitObserver = NULL,
                                       TBool aProgressiveRendering = EFalse,
                                       const CCoeControl* aParent = NULL,
                                       TInt aDisplayFlag = 
                                       ESvgDisplayWaitNote|
                                       ESvgDisplayErrorNotes|
                                       ESvgDisplayIcons|
                                       ESvgDrawPanIndicator|
                                       ESvgDrawPointer|
                                       ESvgDrawPauseIndicator|
                                       ESvgDrawProgressBar|
                                       ESvgUseBackLight
                                        );

        /**
        * Two-phased constructor which pushes created object onto
        * Cleanup Stack.
        * @since 3.0
        * @param aFileHandle RFile that indicates the SVGT content filehandle
        * @param aAppObs Implementation Class provided by application that
        *   uses this control
        * @param aPBDrawImpl Implementation for rendering progress Indicator
        * @param aMSKImpl Implementation for drawing MSK label 
        * Note: The ownership of the file handle is not transferred. 
        *       It is the responsibility of the caller to close this file 
        *       after deleting this control.
        * @return CSVGTCustControl*
        */

        static CSVGTCustControl* NewLC( RFile& aFileHandle,
                                        MSvgtAppObserver* aAppObs,
                                        MSVGTProgressBarDrawImpl* aPBDrawImpl,
                                        MSVGTMSKImpl* aMSKImpl,
                                        MSvgtApplicationExitObserver* 
                                            aExitObserver = NULL,
                                        TBool aProgressiveRendering = EFalse,
                                        const CCoeControl* aParent = NULL,
                                        TInt aDisplayFlag = 
                                        ESvgDisplayWaitNote|
                                        ESvgDisplayErrorNotes|
                                        ESvgDisplayIcons|
                                        ESvgDrawPanIndicator|
                                        ESvgDrawPointer|
                                        ESvgDrawPauseIndicator|
                                        ESvgDrawProgressBar|
                                        ESvgUseBackLight );
        /**
        * Destructor.
        */

        virtual ~CSVGTCustControl();

    public: // New functions
              	       	
   	       	
      	/**
      	* Sets the MSK Labels under various circumstances. 
      	* @since 3.2
      	*/
       	void SetMSKLabelL();
      	
      	/**
      	* Indicates whether the context menu is opened or not
      	* @since 3.2
      	* @param aButtonID -EAknSoftkeyContextOptions indicates context menu icon is pressed
      	*				   -EAknSoftkeySelect animation on mousedown/mouseup event.
      	* @return TBool-ETrue Indicates context menu icon is pressed
      	*				EFalse otherwise.
      	*/
      	TBool GetMSKLabelFlag(TInt aButtonId) const;
      	
      		
      	/**
      	* Sets either the iPlayMSKLabel and iContextMenuActivated flag.
      	* @since 3.2
      	* @param aButtonID -EAknSoftkeyContextOptions indicates context menu icon is pressed
      	*				   -EAknSoftkeySelect animation on mousedown/mouseup event.
      	*/
      	void ChangeMSKLabelFlag(TInt aButtonId);
        
        /**
        * Process commands for handling SVGT content.
        * @since 3.0
        * @param aCommandId Integer indicating command that was activated.
        */
        virtual void ProcessViewerCommandL( TInt aCommandId );

        /**
        * Get function for current zoom level.
        * @since 3.0
        * @param aZoomLevel Integer indicating the number of times the
        *  user has zoomed in(Zero indicates original view or no Zoom)
        */
        void GetCurrentZoomLevel( TInt& aZoomLevel ) const;

        /**
        * Get function for current play status.
        * @since 3.0
        * @param aIsPlaying Boolean indicating whether Content is
        *  Playing/Paused.
        */
        void GetCurrentAnimState( TSvgtViewerAnimStatus& aAnimState ) const;

        /**
        * Indicates whether the Control in Full Screen Mode.
        * @since 3.0
        * @param aIsFullScreenOn Boolean indicating whether Content is
        *  in Full Screen Mode.
        */
        void GetCurrentFullScreenStatus( TBool& aIsFullScreenOn ) const;
        
        /**
        * Indicates whether the Loop feature is enabled for the Control
        * @since 3.0
        * @param aIsLoopOn Boolean indicating whether Content is
        *  in Loop Mode.
        */
        void GetCurrentLoopStatus( TBool& aIsLoopOn ) const;

        /**
        * Gets the normal screen layout rectangle for control from LAF
        * @since 3.0
        * @param aRect TRect indicating the rectangle extent of the control
        */
        void GetNormalScreenLayoutRect( TRect& aRect ) const;

        /**
        * Indicates whether the Loop feature is allowed for the Control
        * Note: Loop Feature is allowed only for Non-Interactive content
        * which has finite duration animations.
        * @since 3.0
        * @return TBool - ETrue Indicates Loop is allowed
        *                 EFalse Otherwise.
        */
        TBool IsLoopAllowed() const;

        /**
        * Finds whether the content contains animations
        * @return TBool -  ETrue indicates content contains animations,
        *                  EFalse otherwise
        * @since 3.0
        */
        TBool  IsAnimationPresent() const;

        /**
        * Finds whether the content contains any infinite duration animation
        * @return TBool -  ETrue indicates Finite duration,
        *                  EFalse otherwise
        * @since 3.0
        */
        TBool  IsContentFinite() const;

        /**
        * Indicates whether the SVG content is loaded 
        * @return TBool -  ETrue indicates content loading complete,
        *                  EFalse otherwise
        * @since 3.0
        */        
        TBool  IsProcessDone() const;
        
        /**
        * Sets the screen mode of the Control State Variable.
        * Note: This does not change the size of the framebuffer - 
        * This is done in the SizeChanged().
        * @since 3.0
        * @param aIsFullScreenOn Boolean indicating whether Content is
        *  in Full Screen Mode.
        */
        virtual void SetCurrentFullScreenStatus( TBool aIsFullScreenOn );

        /**
        * Function used by the application to handle background/foreground
        * events. When the application goes to background the presentation
        * needs to be paused. When application comes back to foreground,
        * the user would need to start the paused presentation. 
        * @param aForeground: ETrue: Foreground event, 
        *                     EFalse: Background event.
        * @since 3.0
        */
        void HandleApplicationForegroundEvent( TBool aForeground );
        
        /**
        * Indicates whether the "Select Text" menu item be displayed
        * @since 3.1
        * @return TBool - ETrue Indicates menu item is displayed
        *                 EFalse Otherwise.
        */
        TBool IsTextSelectOn() const;
        
        /**
        * Indicates whether the "Edit Text" menu item be displayed
        * @since 3.1
        * @return TBool - ETrue Indicates menu item is displayed
        *                 EFalse Otherwise.
        */
        TBool IsTextEditOn() const;

        /**
        * Indicates whether the text content has changed since
        * last Save.
        * @since 3.1
        * @return TBool - ETrue Indicates text has changed
        *                 EFalse Otherwise.
        */
        TBool IsTextContentChanged() const;

        /**
        * Used to notify custom control that save is complete and
        * any cleanup operation can be performed.
        * @since 3.1
        */
        void SaveComplete( const TInt aError );
        TInt DoHandleLoadingThreadError();
        void DoExitFromDialogL();
        
        void AssignImageData( const TDesC& aUri, HBufC8* aData );
        TFileType ImageFileType( const TDesC& aUri ) const;
        void SetThreadRunning(TBool aState);
        TBool IsThreadRunning() const;
        void PerformEngineRedraw();
        
        /**
        * Used to reset the backlight time for finite content
        * any cleanup operation can be performed.
        * @since 3.1
        */
        void ResetBacklightTime();

        void StopEventHandler();
        
        /**
        * This is a utility function is used for performing functionality 
        * once the engine load is complete
        * @since 3.1
        */
        void DoPostLoadFuncL();
        
        /**
        * This is a utility function for saving the SVG content to 
        * a file
        * @since 3.1
        */
        TInt SaveSvgDom( const TDesC& aFileName );
        
        void GetViewPort(TInt getWidth, TInt getHeight, TBool isWidthInPercentage, TBool isHeightInPercentage, TInt& setWidth, TInt& setHeight);
        
    public: // Functions from base classes
        /**
        * From MSvgRequestObserver Method called by SVG Engine
        *  after it has rendered a frame.
        * @since 3.0
        */
        void UpdateScreen();

        /**
        * From MSvgRequestObserver This method is for future extension,
        *  in which an external script engine could be used to evaluate
        *  a script description.
        * @since 3.0
        * @param aScript Descriptor containing script
        * @param aCallerElement SVG Element that calls the script by a
        *  an event of the "on" attribute.
        * @return TBool.
        */
        TBool ScriptCall( const TDesC& aScript,
                          CSvgElementImpl* aCallerElement );

        /**
        * From MSvgRequestObserver This method is called by the SVG engine
        *  to retrieve the absolute path of the image file.
        * @since 3.0
        * @param aUri Descriptor containing the relative path of the image.
        * @param aFilePath Descriptor which contains absolute path that is
        *  to be used by the Engine to retrieve the image.
        * @return TBool -
        *  ETrue indicates absolute path is present in aFilePath.
        *  EFalse otherwise.
        */        
        TInt FetchImage( const TDesC& aUri, 
                          RFs& aSession, RFile& aFileHandle ) ;

        TInt FetchFont( const TDesC& /* aUri */, 
                                    RFs& /* aSession */, RFile& /* aFileHandle */ );

        /**
        * From MSvgRequestObserver This method is called by the SVG engine
        *  to get the SMIL Fit attribute value.
        * @since 3.0
        * @param aSmilValue Descriptor containing the SMIL Fit attribute.
        */
        void GetSmilFitValue( TDes& aSmilValue );

        /**
        * From MSvgRequestObserver This method is called by the SVG engine
        *  to update the presentation status.
        * @since 3.0
        * @param aNoOfAnimation Integer. When called for the first time
        *  it indicates total number of animations.
        *  Subsequently it contains number of animations that were completed.
        */
        void UpdatePresentation( const TInt32&  aNoOfAnimation );

        /**
        * From CCoeControl Key Handling Method for control.
        * @since 3.0
        * @param aKeyEvent Key Event.
        * @param aType Type of Key Event.
        * @return TKeyResponse - EKeyWasConsumed/EKeyWasNotConsumed.
        */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                     TEventCode aType );
        /**
        * From CCoeControl 
        * @see CCoeControl::HandleResourceChange
        */
        void HandleResourceChange( TInt aType );
        
                                             
    public: //From MSvgHyperlinkListener

        /**
        * From MSvgHyperlinkListener
        * @see MSvgHyperlinkListener::LinkEntered
        */ 
        virtual TBool LinkEntered( const TDesC& aUri );

        /**
        * From MSvgHyperlinkListener
        * @see MSvgHyperlinkListener::LinkExited
        */ 
        virtual TBool LinkExited( const TDesC& aUri ) ;
        
 

        /**
        * From MSvgHyperlinkListener
        * @see MSvgHyperlinkListener::LinkActivated
        */ 
        virtual TBool LinkActivated( const TDesC& aUri ) ;

        /**
        * From MSvgHyperlinkListener
        * @see MSvgHyperlinkListener::LinkActivatedWithShow
        */ 
        virtual TBool LinkActivatedWithShow( const TDesC& aUri, 
                                             const TDesC& aShow ) ;
                                            

    public: 

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::ReportAllElements
        */ 
        virtual TBool ReportAllElements() ;

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::WillAssignImageData
        */ 
        virtual TBool WillAssignImageData() ;

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::ImageDataReference
        */ 
        virtual void ImageDataReference( const TDesC& aUri );

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::DocumentStart
        */ 
        virtual TBool DocumentStart() ;

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::DocumentEnd
        */ 
        virtual TBool DocumentEnd() ;

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::ElementStart
        */ 
        virtual TBool ElementStart( const TDesC& aTagName , 
                        MSvgAttributeList& aAttributeList );

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::ElementEnd
        */ 
        virtual TBool ElementEnd( const TDesC& aTagName ) ;

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::ExternalDataRequested
        */ 
        virtual TBool ExternalDataRequested( const TDesC& aUri ) ;

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::ExternalDataReceived
        */ 
        virtual TBool ExternalDataReceived( const TDesC& aUri ) ;

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::ExternalDataRequestFailed
        */ 
        virtual TBool ExternalDataRequestFailed( const TDesC& aUri ) ;

        /**
        * From MSvgLoadingListener
        * @see MSvgLoadingListener::UnsupportedElement
        */ 
        virtual TBool UnsupportedElement( const TDesC& aTagName , 
                        MSvgAttributeList& aAttributeList );
                        
        virtual void ImagesLoaded(TInt aError);
		
		public: // From MSvgAudioMuteListener
				/**
        * This function is called to add a observer that will want a 
        * callback from custcontrol
        * @param 	aListener object which needs the callback
        * 				aType represents the type of listener
        * @since 5.0
        */
        IMPORT_C void AddListener(MSvgCustControlListener *aListener, TCustControlListener aType);
		
    protected:  // New functions - Startup Helper Functions
        /**
        * This function is called to create the bitmaps used by the SVG
        * engine for rendering.
        * @param aBitmapSize Size of Bitmap used for Framebuffer
        * @since 3.0
        */
        void CreateBitmapsL( const TSize& aBitmapSize );
        
        /**
        * This function is called to create the icons used by the control
        * for panning/pointer display etc.
        * @since 3.0
        */        
        void CreateIconsL();
        /**
        * This function is called to initialize the SVGT-Engine interface.
        * @since 3.0
        */
        void InitializeEngineL();

        /**
        * This function uses the SVG Engine to load the SVGT content.
        * @since 3.0
        * @param aFileHandle file handle to the SVGT content
        *  SVGT content.
        */
        void LoadContentL( RFile& aFileHandle );

        /**
        * This function is used for the loading of SVG content in case
        * of progressive rendering
        * @since 3.1
        * @param TAny* Pointer to the custom control 
        */
        static TInt LoadSvgContent( TAny* aAny );
        
        /**
        * This function is used for initialising the SVG engine in case
        * of progressive rendering
        * @since 3.1
        */
        void InitSvgContentL();
        
        /**
        * This is a utility function is used for displaying the 
        * information notes 
        * @since 3.1
        */
        void DisplayInfoNoteL( TInt aResourceId, TBool aIsWaitingDialog );
        
    protected:  // New functions - Pointer Helper Functions
        /**
        * Draws the Pointer on the screen.
        * @since 3.0
        */
        void DrawPointer() const;

        /**
        * This function is called when pointer needs to be redrawn.
        * @since 3.0
        */
        void UpdatePointer();

    protected:  // New functions - Full/Normal Screen Helper Functions
        /**
        * This function is called to change the framebuffer to new size 
        * in the SizeChanged function
        * @since 3.0
        */
        void SwitchScreenModeL();

    protected:  // New functions - Find Text Helper Functions

        /**
        * This function is called to calculate the find text bounding boxes.
        * @since 3.0
        * @return Bool Indicating Text was found or not
        */
        TBool CalcFindTextBBox();

        /**
        * This function is called to pan the content in order that the
        * current bounding box is visible. The reference point is taken as the
        * center of the screen.
        * @since 3.0
        */
        void PanToCurBBox();

        /**
        * This function is called to perform the necessary procesing for
        * Find Text.
        * @since 3.0
        */
        void ProcessFindTextCmdL();

        /**
        * Draws the Bounding box for the current match on the screen.
        * @since 3.0
        *
        */
        void DrawFindTextBBox( const TRect& aRect ) const;
        
        /**
        * This function is called to perform the necessary procesing when
        * text being searched using Find Text command is not found.
        * @since 3.0
        */
        void ProcessNoTextFoundL();


    protected:  // New functions - Panning Indicator Helper Functions
        /**
        * Draws the Pan Indicators on the screen.
        * @since 3.0
        */
        void DrawPanInd() const;

    protected:  // New functions - Related to Progress Bar/Loop
        /**
        * Counts the number of animations started in the SVGT content.
        * @since 3.0
        */
        void CountAnimStarted();

        /**
        * Counts the number of animations paused in the SVGT content.
        * @since 3.0
        */
        void CountAnimPaused();
        
        /**
        * Counts the number of animations stopped in the SVGT content.
        * @since 3.0
        */
        void CountAnimStopped();

        
        /**
        * Function indicates whether the animation currently in 
        * progress is first one.
        * @return TBool - ETrue - First one
        *                 EFalse Otherwise.
        * @since 3.0
        */
        TBool IsAnimFirst() const;
        
        /**
        * Function indicates whether the animation currently in 
        * progress is first one.
        * @return TBool - ETrue - First one
        *                 EFalse Otherwise.
        * @since 3.0
        */
        TBool IsAnimLast() const;

    protected:  // New functions for Pointer handling
        /**
        * Function sets the Pointer display status and redraws the whole
        * control inorder to update display status
        * @since 3.0
        */
        void SetPointerDisplayStatusAndRedraw( TBool aIsPointerDisplayed );
    protected:  // New functions for pause icon blink support
        /**
        * Function sets the Pause display status and starts the periodic
        * timer to draw the Pause Indicator
        * @param TBool ETrue Indicates that Pause should be displayed
        * @since 3.0
        */
        void ShowPauseIcon( TBool aPauseIconVisible );
        
        /**
        * Callback Function for periodic timer for blinking the pause indicator 
        * @see TCallBack 
        * @since 3.0
        */
        static TInt BlinkPauseIcon( TAny* aPtr );

        /**
        * Function which does the actual work of toggling pause indicator
        * @since 3.0
        */
        void DoBlinkPauseIcon();
    protected:  // New functions for cleanup 
        /**
        * Function which frees the icons created by CreateIconsL().
        * @since 3.0
        */
        void FreeIcons();

    protected:  // New functions for wait-note processing
        /**
        * Launch wait note.
        * @since 3.0
        * @param aResourceId Resource id for note text.
        */
        void LaunchWaitNoteL( TInt aResourceId );

        /**
        * Dismiss wait dialog.
        * @since 3.0
        */
        void DismissWaitNote();
    protected:  // New functions for progress bar timer handling

        /**
        * Start progress indicator timer.
        * @since 3.0
        * @param aAfter Time period in microseconds after which the timer ticks.
        */
        void StartProgIndTimer( TInt aAfter );

        /**
        * Stop the progress indicator timer.
        * @since 3.0
        */
        void StopProgIndTimer();

    protected:   // New functions for handling animation controls 
                 // play/pause/stop
        /**
        * This function is called to perform the necessary procesing for
        * the Play Command.
        * @since 3.0
        */
        void ProcessPlayCmd();
        
        /**
        * This function is called to perform the necessary procesing for
        * the Pause Command.
        * @since 3.0
        */
        void ProcessPauseCmd();
        
        /**
        * This function is called to perform the necessary procesing for
        * the Stop Command.
        * @since 3.0
        */
        void ProcessStopCmd();
   
   private: // Functions for voice call check        
        /**
        * This function checks whether a voice call is in progress        
        * and in connected state. 
        * @return TBool - ETrue - If voice call is in connected state
        *                 EFalse Otherwise.
        * @since 3.0
        */
        TBool IsVoiceCallActive();        

   private: // New Function for Note remover timer
        /**
        * Callback Function for timer for removing the note that is not 
        * removed in the UI.
        * @see TCallBack 
        * @since 3.0
        */
        static TInt RemoveInfoNote( TAny* aPtr );

   private: // New Functions for rotation
        /**
        * This function rotates the presentation with given angle 
        * about bitmap center. 
        * @param aAngle in Degrees
        */
        void DoRotation(const TInt aAngle);
    
   private: // New Functions for Text Editing/Selection
#ifdef SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE   
        /**
        * This function displays the existing text in the text/textArea 
        * in a read-only dialog so that the user can copy text from it to
        * the system clipboard.
        * @param aText Text content Descriptor
        */
        void DoTextSelectionL( TDes& aText ) const;

        /**
        * This function displays the existing text in the text/textArea
        * element which is editable and returns the modified text in the
        * same descriptor.
        * @param aIsTextElement Bool indicating whether elem is text/textArea
        * @param aElemHandle TInt Handle for the elem from engine
        * @param aText Text content Descriptor modified on return
        */
        void DoTextEditingL( TBool aIsTextElement, TInt aElemHandle, 
            TDes& aText );
#endif // SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE
    private: // New functions for Four Point Rects
        /**
        * This function calculates the center of the 4-point rect
        * @param aBBoxRect the Four point Rect whose center is to be found
        * @return TPoint - Center coordinates
        */
        TPoint FPRCenter( const MRect* aBBoxRect ) const;
        
        /**
        * This function checks whether the FPR intersects with the rectangle
        * @param aBBoxRect the Four point Rect 
        * @param aRect Rectangle with which intersection is to be found
        * @return TBool - Whether Intersects(ETrue) or not.
        */
        TBool FPRIntersects( const MRect* aBBoxRect, const TRect& aRect ) const;

        /**
        * This function converts a four point rectangle to an array of points
        * @param aBBoxRect the Four point Rect 
        * @param aArrayPtr Array of TPoints
        */
        void FPR2PointArrayL( const MRect* aBBoxRect, 
            CArrayFix<TPoint>*& aArrayPtr ) const;
        
    protected:  // Functions from base classes
        /**
        * From CCoeControl Draws the SVGT frame on the screen.
        * @since 3.0
        * @param aRect Rectangle which needs to be redrawn.
        */
        virtual void Draw( const TRect& aRect ) const;

        /**
        * From CCoeControl Constructs the control from resource.
        * @since 3.0
        * @param aReader Resource Reader with which to access the
        *  control's resource.
        */
        virtual void ConstructFromResourceL( TResourceReader& aReader );

        /**
        * From CCoeControl Handles control resizing.
        * @since 3.0
        */
        virtual void SizeChanged();
        
        /**
        * From MSvgAnimationListener Callback called by SVG 
        * engine when animation starts
        * @since 3.0
        */
        TBool AnimationStarted( TBool aIsAnimationIndefinite );
        
        /**
        * From MSvgAnimationListener Callback called by SVG 
        * engine when animation ends.
        * @since 3.0
        */
        TBool AnimationEnded();
        
        /**
        * From MSvgAnimationListener Callback called by SVG 
        * engine when animation is paused.
        * @since 3.0
        */
        TBool AnimationPaused();

        /**
        * From MSVGTProgressBarListener. This function 
        * updates the progress bar in the navi pane. It is called 
        * back by the progress bar timer.
        * @since 3.0
        */
        void UpdateProgressBar();

        /**
        * From MSVGTPointerInactivityListener. This function 
        * updates the display status for the pointer. It is called 
        * back by the Pointer Hide Timer after a period of 
        * inactivity.
        * @since 3.0
        */
        void PointerInactivityTimeout();
#ifdef SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE
        /**
        * From MSvgTextAreaListener. This callback is called to notify
        * when a pointer enters a textbox element.
        * @since 3.1
        * @see MSvgTextAreaListener
        */
        virtual TBool TextAreaEntered( TInt aTextAreaHandle );

        /**
        * From MSvgTextAreaListener. This callback is called to notify 
        * when a pointer exits a textbox element.
        *
        * @since 3.1
        * @see MSvgTextAreaListener
        */
        virtual TBool TextAreaExited( TInt aTextAreaHandle );

        /**
        * From MSvgTextAreaListener. This callback is called to notify 
        * the client that a link has been activated.
        *
        * @since 3.1
        * @see MSvgTextAreaListener
        */
        virtual TBool TextAreaActivated( TInt aTextAreaHandle );
        
        /**
        * From MSvgTextListener - Notified when a pointer enters a 
        * text element.
        *
        * @since 3.1 
        * @see MSvgTextListener
        */
        virtual TBool TextEntered( TInt aTextHandle );

        /**
        * From MSvgTextListener - Notified when a pointer exits a 
        * text element.
        *
        * @since 3.1 
        * @see MSvgTextListener
        */
        virtual TBool TextExited( TInt aTextHandle );

        /**
        * From MSvgTextListener - Notified when a pointer selects a 
        * text element.
        *
        * @since 3.1 
        * @see MSvgTextListener
        */
        virtual TBool TextActivated( TInt aTextHandle );
#endif // SVGTUICONTROL_DBG_TEXT_EDIT_SELECT_API_ENABLE

        /**
        * From MSvgInteractiveElementListener. This callback is called to notify 
        * the client that an interactive element has been entered.
        *
        * @since 3.2
        * @see MSvgInteractiveElementListener
        */
        virtual TBool InteractiveElementEntered(TPtrC aElementIdPtr, TUint16 aInteractonTypeList );
        
        /**
        * From MSvgInteractiveElementListener - Notified when a pointer exits an 
        * interactive element.
        *
        * @since 3.2 
        * @see MSvgInteractiveElementListener
        */
        virtual TBool InteractiveElementExited(TPtrC aElementIdPtr);
        
    private:

        /**
        * C++ default constructor.
        * @param RFile File Handle - File handle of SVGT content
        */
        CSVGTCustControl( RFile& aFileHandle , 
                          TBool aProgressiveRendering, 
                          TInt aDisplayFlag = 
                             ESvgDisplayWaitNote|
                             ESvgDisplayErrorNotes|
                             ESvgDisplayIcons|
                             ESvgDrawPanIndicator|
                             ESvgDrawPointer|
                             ESvgDrawPauseIndicator|
                             ESvgDrawProgressBar|
                             ESvgUseBackLight );

        /**
        * Second Phase constructor.
        * @param aAppObs Implementation Class provided by application that
        *   uses this control
        * @param aPBDrawImpl Implementation for rendering progress Indicator
        * @param aMSKImpl Implementation for drawing MSK label 
        */
        void ConstructL( MSvgtAppObserver* aAppObs,
                         MSVGTProgressBarDrawImpl* aPBDrawImpl,
                         MSVGTMSKImpl* aMSKImpl,
                         MSvgtApplicationExitObserver* aExitObserver,
                         const CCoeControl* aParent );
    private:    // Data
        // Stores the Application's implementation of the Callbacks.
        MSvgtAppObserver* iAppObserver;
        // Stores the Application's Exit callback implementation pointer
        MSvgtApplicationExitObserver* iExitObserver;
        // Bitmap used by SVG Engine to render content.
        CFbsBitmap*  iSVGTBitMap;
        
        // Mask for the bitmap.
        CFbsBitmap*  iSVGTBitMapMask;

        // Icons
        // Pointer Icons - owned 
        CGulIcon* iPointerIcon; 
        CGulIcon* iHandIcon;
        CGulIcon* iTextIcon;
                
        // Panning Indicator Icons - owned 
        CGulIcon* iPanIndLeftIcon;
        CGulIcon* iPanIndRightIcon;
        CGulIcon* iPanIndUpIcon;
        CGulIcon* iPanIndDownIcon;
        
        // Pause Indicator Icons - owned
        CGulIcon* iPauseIcon;
        
        // SVG Engine Interface Implementation pointer
        CSvgEngineInterfaceImpl*     iSvgModule;

        // Stores the total number of animation in content.
        TInt32 iTotalNumerOfAnimation;

        // Stores the current animation status.
        TSvgtAnimStatus iAnimationStatus;
        
        // Stores the viewer animation state.
        TSvgtViewerAnimStatus iAnimState;

        // SVGT content filehandle
        RFile& iContentFileHandle;

        // Boolean to keep track of whether the loading of content is done.
        TBool iIsLoadingDone;
        
        // Integer variable which stores the FetchImage error
        TInt iFetchImageError;

        // Zoom Level is the number of times content has been zoomed-in.
        TInt iZoomLevel;

        // Boolean indicating whether Loop Mode is enabled.
        TBool iIsLoopOn;

        // Boolean indicating whether Full Screen mode is enabled.
        TBool iIsFullScreenOn;

        // Pointer State Variable - Delta Change in X in pixels.
        TInt iPointerDx;

        // Pointer State Variable - Delta Change in Y in pixels.
        TInt iPointerDy;

        // Pointer State Variable - Current X pixel coordinate.
        TInt iPointerX;

        // Pointer State Variable - Current Y pixel coordinate.
        TInt iPointerY;

        // Last Searched Text for use with Find Text
        TBuf<KFindTextQueryMaxLen> iLastSearchText;

        // Array to store the Bounding boxes for found text
        RPointerArray <MRect> iBoundBoxArray;

        // Array to store the text found
        RArray <TPtrC> iFoundTextArray;

        // Array to store the text element ids found - for use in animation case.
        RArray <TInt> iTextElemIdArray;

        // Bool that keeps track whether current search string(iLastSearchText)
        // is valid
        TBool iIsSearchStrValid;

        // Current Match Index
        TInt iCurSearchMatchId;
        
        //Flag to check whether the svg file has only Preview rights
        TBool iPreviewMode;

        // Number of animations in progress (whose repeatCount != Infinite)
        TInt iNumAnimInProgress;
        
        // Progress Indicator Timer
        CSVGTProgressBarTimer* iProgressBarTimer;
        
        // Duration of SVG Content
        TUint iDuration;
        
        // Interface for Progress Bar Draw Implementation
        MSVGTProgressBarDrawImpl* iProgressBarDrawImpl;
        
         //Interface for MSK label implementation
         MSVGTMSKImpl* iMSKImpl;
        
        // Pointer Hide Timer
        CSVGTPointerHideTimer* iInactivityTimer;
        
        // Bool that keeps track whether pointer should be drawn or not.
        TBool iIsPointerDisplayed;
        
        // Timer for blinking pause icon
        CPeriodic* iBlinker;

        // Bool that keeps track whether paused icon is drawn or not.
        TBool iPauseIconVisible;
       
             
        // Number of continous key presses since last key down
        TInt iKeyRepeatCount;
        // Main Viewer Pane Layout
        TAknLayoutRect iViewerPaneRect;
        // Layout Rectangles for Pan Indicators
        TAknLayoutRect iPanIndLeftRect;
        TAknLayoutRect iPanIndRightRect;
        TAknLayoutRect iPanIndUpRect;
        TAknLayoutRect iPanIndDownRect;
        TAknLayoutRect iPauseRect;
        
        // Layout Touch Area Rectangles of Pan Indicators
        TAknLayoutRect iPanIndLeftAidRect;
        TAknLayoutRect iPanIndRightAidRect;
        TAknLayoutRect iPanIndUpAidRect;
        TAknLayoutRect iPanIndDownAidRect;
        
        // Layout Rectangle for Pointer
        TAknLayoutRect iPointerRect;
        TAknLayoutRect iHandRect;
        TAknLayoutRect iTextRect;
        
        // Layout Rectangle for aid of placement of pointer
        TAknLayoutRect iPointerAidRect;
        TAknLayoutRect iHandAidRect;
        TAknLayoutRect iTextAidRect;
        
        // Framebuffer Bitmap
        CFbsBitmap*  iSVGTFrameBufBitMap;
                
        // Bitmap Device
        CFbsBitmapDevice* iSVGTFrameBufDevice;
        
        // Bitmap Context
        CFbsBitGc* iSVGTFrameBufContext; 

        // Current Layout - To check whether layout changed in SizeChanged()
        TRect iCurrentLayout;
        
        // Find Text Highlight Colour
        TRgb iFindTextHiColor;        
        
        // Number of microseconds elapsed since content started.
        TUint iProgIndTimeElap;
        
        // Number of milliseconds elapsed since last progress indicator update
        TUint iProgIndResidueTime;
        
        // Last update of progress indicator tick in millisec
        TUint iProgIndLastTickTime;
        // Wait note dialog pointer
        CAknWaitDialog* iWaitDialog;
        
        // DrmHelper to display save related query
        CSVGTDrmHelper*   iSvgDrmHelper;
        
        // To get the type and the state of the phone call
        RProperty iSystemState;
        
        // Used to keep track of whether key events are ignored 
        // when pointer is unhidden
        TBool iIgnorePointerKeyEvent;        
        
        // Information Note for displaying the "text not found" note
        CAknInformationNote* iInfoNote;
        
        // Timer for removing the "text not found" wait note
        CPeriodic* iNoteRemoverTimer; 
        
        // Indicates what type of pointer is to be drawn
        TSvgtViewerPointerTypes iPointerType;
        
        // Enable/Disable Control for text operations(Editing/selection)
        TBool iEnableTextOps;
        
        // Boolean indicating whether text edit menu is to be displayed
        TBool iIsTextEditOn;
        
        // Boolean indicating whether text select menu is to be displayed
        TBool iIsTextSelectOn;
        
        // Text (text/textArea) element currently under the cursor
        TInt iCurrentTextElementId;
        
        // Boolean indicating whether current element is text(ETrue) or
        // a textArea (EFalse) element
        TBool iIsCurrentTextElement;

        TBool iProgressiveRendering;
        TInt iDisplayFlags;
        
        CSvgtEventHandlerAO* iEventHandlerAO;
        CSvgtThreadController* iThreadController;
        RThread iMainThread;
        MSvgError* iLoadingThreadError;
        TFileType iImageFileType;
        TTime iPreviousBackLightTimeStamp;
        TBool iIsThreadRunning;
        TBool iIsWaitNoteDisplayed;
//TOUCH SUPPORT START    
#ifdef RD_SVGT_AUDIO_SUPPORT
       CAknVolumePopup* iVolumePopup;
       TInt iPrevVolume;
       TInt iVolumeLevel;

    CRemConCoreApiTarget* iInterfaceSelector;
    CRemConInterfaceSelector* iSelector;
#endif //RD_SVGT_AUDIO_SUPPORT
//TOUCH SUPPORT END
        // The cause for select operation - Hyperlink/text/textArea activate
        TSvgCustomControlSelectCause iSelectCause;        
        // Boolean indicating whether the cursor is inside a textArea element
        TBool iTextAreaEntered;
        
        // Boolean indicating whether text is modified in content
        // since last save
        TBool iIsTextChanged;
        // Booleans that are updated every time a frame is rendered
        // in order to avoid overhead of calling IsPanPossibleFourWay
        // API all the time.
        // Pan Left Possible
        TBool iPanLt;
        // Pan Right Possible
        TBool iPanRt;
        // Pan Up Possible
        TBool iPanUp;
        // Pan Down Possible
        TBool iPanDn;
        
        //Bool that checks whether the select key is pressed for options or a normal seletion
        TBool iViewerCommand;
        
        //Flag to check whether play msk is visible
		TBool  iPlayMSKLabel;
					
		//Flag that indicates MSK is not visible	
		TBool iContextMenuActivated;
        
 		// Flag to indicate whether the element is "a" element or not
 		TBool iAElement;
 		
 		//Flag to check whether scrolling is done when the pointer is hidden
 		TBool iScrollKey;
 		
 		
 		//Flag to indicate that pointer-down animation is possible with the element 
 		TBool iPointerDownAnimation; 
 		
 		// Count maintained to get a list of interactive elements entered
 		TInt iInteractiveElementEnteredCnt;
#ifdef _DEBUG
        TInt iDbgNumAnimStarted;
        TInt iDbgNumAnimStopped;
        TInt iDbgNumAnimPaused;
#endif        
        
       TBool iEngineInitialized;
       
       TBool iDownKeyOccurred;
       
       //indicates if volume is currently muted or not
       TBool iIsVolumeMuted;
       TBool iIsForeground;
			
			 //AudioListener to indicate to UIdialog that volume is muted
       MSvgAudioMuteListener* iAudioMuteListener;
    };

#endif  // __SVGTCUSTCONTROL_H__

// End of File
