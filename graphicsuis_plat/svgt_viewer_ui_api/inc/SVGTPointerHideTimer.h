/*
* Copyright (c) 2004, 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This header defines the CSVGTPointerHideTimer Class
*
*/

#ifndef SVGTPOINTERHIDETIMER_H
#define SVGTPOINTERHIDETIMER_H

// Includes 
#if !defined(__E32BASE_H__)
#include <e32base.h>
#endif

/**
*  Interface for the pointer inactivity event listener.
*  @lib SVGTUIControl.lib
*  @since 3.0
*/

class MSVGTPointerInactivityListener 
    {
    public:
    
        /**
        * This function is called when the pointer inactivity occurs.
        * @since 3.0
        */
        virtual void PointerInactivityTimeout( ) = 0;
    };

/**
*  Provides the timer functionality for hiding the pointer after period of
*  inactivity
*  @lib SVGTUIControl.lib
*  @since 3.0
*/

class CSVGTPointerHideTimer : public CTimer
    {
    public: // Constructors and Destructor
        /**
        * Two-phased constructor.
        */
        static CSVGTPointerHideTimer* NewLC( MSVGTPointerInactivityListener* 
                aListener );

        /**
        * Two-phased constructor.
        */
        static CSVGTPointerHideTimer* NewL( MSVGTPointerInactivityListener* 
                aListener );

        // Destructor
        virtual ~CSVGTPointerHideTimer();

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
        CSVGTPointerHideTimer( MSVGTPointerInactivityListener* 
                aListener );
        
        /**
        * Second Phase constructor.
        */
        void ConstructL();

    private:    // Data
        MSVGTPointerInactivityListener *iInactivityListener;
    };
#endif // __SVGTBACKLIGHTTIMER_H__

// End of File
