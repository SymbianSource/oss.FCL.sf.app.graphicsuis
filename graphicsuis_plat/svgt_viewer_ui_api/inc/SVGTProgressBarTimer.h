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
* Description:  This header defines the CSVGTProgressBarTimer Class
*
*/

#ifndef __SVGTPROGRESSBARTIMER_H__
#define __SVGTPROGRESSBARTIMER_H__

// Includes 
#if !defined(__E32BASE_H__)
#include <e32base.h>
#endif


/**
*  Interface for the progress timer event listener.
*  @lib SVGTUIControl.lib
*  @since 3.0
*/

class MSVGTProgressBarListener 
    {
    public:
    
        /**
        * This function is called when the progress timer event occurs. The
        * user of this interface will be notified after the amount of time elapsed
        * configured in KSccProgressBarTimerYieldDur.
        * @since 3.0
        */
        virtual void UpdateProgressBar( ) = 0;
    };

/**
*  Provides the timer functionality for progress time display for the 
*  dialog.
*  @lib SVGTUIControl.lib
*  @since 3.0
*/

class CSVGTProgressBarTimer : public CTimer
    {
    public: // Constructors and Destructor
        /**
        * Two-phased constructor.
        */
        static CSVGTProgressBarTimer* NewLC( MSVGTProgressBarListener* 
            aListener ,
            TBool aResetInactivityTime  );

        /**
        * Two-phased constructor.
        */
        static CSVGTProgressBarTimer* NewL( MSVGTProgressBarListener* 
            aListener,
            TBool aResetInactivityTime );

        // Destructor
        virtual ~CSVGTProgressBarTimer();

    public: // New functions
    public: // Functions from base classes

        /**
        * From CActive Implements cancellation of an outstanding request.
        * @since 3.0
        */
        void DoCancel();

        /**
        * From CActive Handles the timer's request completion event.
        * @since 3.0
        */
        void RunL();

    protected: // New functions
    protected: // Functions from base classes

    private:
        /**
        * C++ default constructor.
        */
        CSVGTProgressBarTimer( MSVGTProgressBarListener* 
            aListener , TBool aResetInactivityTime  );
        
        /**
        * Second Phase constructor.
        */
        void ConstructL();

    private:    // Data
        
        // Listener for progress bar timer event
        MSVGTProgressBarListener* iListener;
        
        // Reset Inactivity time
        TBool iUseInactivityTime;
    };
#endif // __SVGTPROGRESSBARTIMER_H__

// End of File

