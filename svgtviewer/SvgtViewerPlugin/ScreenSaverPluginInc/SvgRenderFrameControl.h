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
* Description:  SVGRenderFrameControl provides an implementation of a control 
*                which renders SVG using RenderFrames API provided by the 
*                engine.
*
*/


#ifndef SVGRENDERFRAMECONTROL_H
#define SVGRENDERFRAMECONTROL_H

#include <gdi.h>
#include <SVGRequestObserver.h>
#include <e32base.h>
#include <coecntrl.h>
#include "SvgRenderFrameTimer.h"

const TInt KMaxTimeDuration=2500;
class CSvgEngineInterfaceImpl;
class MSvgtAppObserver;


/**
*  CSvgRenderFrameControl is a CCoeControl used to display SVGT content.
*  It makes use of RenderFrames API provided by the svg engine to get
*  the set of frame buffers and render it one by using render frame timer
*
*  @lib SVGTScreenSaverPlugin.lib
*  @since 3.1
*/
class CSvgRenderFrameControl: public CCoeControl,
                              public MSvgRenderFrameTimerListener,
                              public MSvgRequestObserver
    {    
    
    public: 
        
        /**
        * Two-phased constructor.
        * @since 3.1
        */
        static CSvgRenderFrameControl* NewL( 
                                        const CCoeControl* aParent, 
                                        RFile& aFileHandle,
                                        MSvgtAppObserver* aObserver = NULL );
                                             
        /**
        * Two-phased constructor.
        * @since 3.1
        */
        static CSvgRenderFrameControl* NewLC( 
                                         const CCoeControl* aParent, 
                                         RFile& aFileHandle,
                                         MSvgtAppObserver* aObserver = NULL );
        
        /**
        * Destructor.
        */
        virtual ~CSvgRenderFrameControl();           

            
        /**
        * From CCoeControl, 
        * @since 3.1
        */
        void Draw(const TRect& aRect) const;
        
        /**
        * From CCoeControl, 
        * @since 3.1
        */
        void SizeChanged();
        
    public : // From MSvgRequestObserver        
        
        /**
        * From MSvgRequestObserver, Method Called by Svg Engine,
        * when the frame buffer is updated and has to be redrawn
        * @since 3.1
        */
        virtual void    UpdateScreen();
        
        /**
        * From MSvgRequestObserver, Method Called by Svg Engine,
        * when there is a script support
        * @since 3.1
        */
        virtual TBool   ScriptCall( const TDesC& aScript,
                                    CSvgElementImpl* aCallerElement );
        
        /**
        * From MSvgRequestObserver, Method Called by Svg Engine,
        * when it encounters an image element
        * @since 3.1
        */
        virtual TInt FetchImage( const TDesC& aUri, RFs& aSession, 
                                 RFile& aFileHandle );

        TInt FetchFont( const TDesC& /* aUri */, 
                                    RFs& /* aSession */, RFile& /* aFileHandle */ );

        /**
        * This Method is called by the Svg engine to  Get the SMIL 
        * Fit Attribute Value
        * @since 3.1
        */
        virtual  void GetSmilFitValue( TDes& aSmilValue );

        /**
        * This Method updates the presentation status
        * @since 3.1
        */
        virtual  void UpdatePresentation(const TInt32&  aNoOfAnimation);            
        
    public: // From MSvgRenderFrameTimerListener
        
        /**
        * This function is a callback from animation timer, 
        * when there is an animation timeout.
        * @since 3.1
        */             
        virtual void AnimationTimeOut();
                            
        
    public: // New Function
        
        /*
        * Starts the timer
        * @since 3.1
        */
        void SetTimer();
        
        /*
        * Cancels the timer
        * @since 3.1
        */
        void CancelTimer();
        
        /*
        * Processes the command sent by the user of the control
        * @param aCommandId - command to be processed
        * @since 3.1
        */
        virtual void ProcessViewerCommandL( TInt aCommandId );   
        
        /*
        * Gets the current index of the frame buffers
        * @since 3.1
        */
        TInt GetCurrentIndex() const;
        
        /*
        * Sets the index of the buffer to be used
        * @param aIndex - index to be set
        * @since 3.1
        */        
        void SetCurrentIndex( TInt aIndex );
        
        /**
        * This function initializes the svg engine.
        * This function is also used to check the validity of the file.
        * @param aCheckFile True if used in check file mode
        *                   False otherwise
        * @since 3.1
        */      
        void InitializeEngineL( TBool aCheckFile, 
                                MSvgRequestObserver* aObserver = NULL );       
     
        
        /**
        * This function deletes the svg engine.
        * @since 3.1
        */  
        void DeleteEngine();
        
        /**
        * This function uses the svg engine to get the frames in an array 
        * of buffers.
        * @since 3.1
        */ 
        TInt RenderMyFrames();
        
        /**
        * This function moves the index of the array 
        * of buffers.
        * @since 3.1
        */         
        void MoveIndex();
        
        /**
        * This function logs the debug message 
        * @param aMsg - message to be logged
        * @since 3.1
        */          
        void PrintDebugMsg( const TDesC&  aMsg ) const;            
       
       
    public:   
        /**
        * Default Constructor
        * @since 3.1
        */  
        CSvgRenderFrameControl( RFile& aFileHandle ,
                         MSvgtAppObserver* aObserver = NULL );

        
        /**
        * 2nd phase constructor
        * @since 3.1
        */
        void ConstructL( const CCoeControl* aParent );

    private:        
        
        // Pointer to the Svg Engine
        CSvgEngineInterfaceImpl* iSvgEngine;
        
        // Frame buffer to be given to svg engine during initialization
        CFbsBitmap* iBmap;

        // Array of bitmaps generated by RenderFrames
        RPointerArray<CFbsBitmap> iBitmapFrames;
        
        // Array of corresponding masks for the Bitmaps
        RPointerArray<CFbsBitmap> iMaskFrames;
        
        // Array of TimeInterval between frames
        RArray<TUint> iDelayIntervals;  
        
        // file handle of the svg file to be set as ScreenSaver.
        RFile* iFileHandle;
        
        // Index of the bitmap in the array
        TInt iCurrentFrame; 
        
        // Start duration to be given to RenderFrames
        TUint iDuration;       
    
        // Handle to the Svg Dom
        TInt iSvgDom;        
        
        // timer
        CSvgRenderFrameTimer* iRenderFrameTimer;
        
        // boolean to check whether it is in play or paused state
        TBool iCanPlay;
        
        // size of the control currently
        TRect iRect;
        
        // Parent Control
        const CCoeControl* iParent;
        
        // App observer pointer
        MSvgtAppObserver* iAppObserver;
               
        };

#endif // SVGRENDERFRAMECONTROL_H
// End of File
