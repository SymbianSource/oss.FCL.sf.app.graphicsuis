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
* Description:  SVGRenderFrameTimer provides an implementation of timer 
*                for RenderFrames Control
*
*/


#ifndef SVGRENDERFRAMETIMER_H
#define SVGRENDERFRAMETIMER_H

#if !defined(__E32BASE_H__)
#include <e32base.h>
#endif

/**
*  Interface for the render frame timer listener.
*  @lib SVGTScreenSaverPlugin.lib
*  @since 3.1
*/
class MSvgRenderFrameTimerListener
    {

    public:
    /*
    * Function called when timeout occurs
    * @since 3.1
    */
    virtual void AnimationTimeOut() = 0;   
   
    };

/**
*  Provides the timer functionality for RenderFrames control  
*  @lib SVGTScreenSaverPlugin.lib
*  @since 3.1
*/
class CSvgRenderFrameTimer: public CTimer                            
    {    
    
    public: 
        
        /**
        * Two-phased constructor.
        * @since 3.1
        */
        static CSvgRenderFrameTimer* NewL( MSvgRenderFrameTimerListener* 
                                           aListener );
                                           
        /**
        * Two-phased constructor.
        * @since 3.1
        */                                           
        static CSvgRenderFrameTimer* NewLC(MSvgRenderFrameTimerListener* 
                                           aListener );
        
        /**
        * Destructor.
        */
        virtual ~CSvgRenderFrameTimer();              
        
        
    public: // CTimer interface methods        
         

        /**
        * From CActive Implements cancellation of an outstanding request.
        * @since 3.1
        */
        void DoCancel();

        /**
        * From CActive Handles the timer's request completion event.
        * @since 3.1
        */
        void RunL();
   
    public: // new function(s)
    
        /**
        * Sets the timeout to the interval specified.
        * @param anInterval the timeout value
        * @since 3.1
        */
        void SetTimeOut( const TTimeIntervalMicroSeconds32& anInterval );
       
    
    private:
    
        /**
        * Default Constructor
        * @since 3.1
        */  
        CSvgRenderFrameTimer( MSvgRenderFrameTimerListener* aListener  );
        
        /**
        * 2nd phase constructor
        * @since 3.1
        */
        void ConstructL();

    private:        
        
        // Listener for the timeout
        MSvgRenderFrameTimerListener* iListener;
               
        };

#endif // SVGRENDERFRAMETIMER_H
// End of File
