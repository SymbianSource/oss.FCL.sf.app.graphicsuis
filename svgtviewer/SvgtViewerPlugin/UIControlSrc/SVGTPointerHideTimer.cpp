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
* Description:  This file implements the timer functionality required for the
*                hiding of pointer in case of inactivity.
*
*/


// User Includes
#include "SVGTPointerHideTimer.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTPointerHideTimer::CSVGTPointerHideTimer
// Default Constructor. Initialises Timer State Variables.
// -----------------------------------------------------------------------------
//
CSVGTPointerHideTimer::CSVGTPointerHideTimer( MSVGTPointerInactivityListener* 
        aListener) : // Listener for the pointer inactivity
    CTimer( CActive::EPriorityStandard ), iInactivityListener( aListener )
    {
    }

// -----------------------------------------------------------------------------
// CSVGTPointerHideTimer::ConstructL
// Second Phase Constructor for CSVGTPointerHideTimer. Adds this object to the 
// applications active scheduler.
// -----------------------------------------------------------------------------
//
void CSVGTPointerHideTimer::ConstructL()
    {
    // Call the base class ConstructL
    CTimer::ConstructL();
    // Add to active scheduler
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CSVGTPointerHideTimer::NewLC
// Factory function for creating CSVGTPointerHideTimer objects. It also 
// pushes the created dialog object onto the cleanup stack.
// Returns: CSVGTPointerHideTimer* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTPointerHideTimer* CSVGTPointerHideTimer::NewLC(
    MSVGTPointerInactivityListener* aListener ) // Listener for inactivity event
    {
    CSVGTPointerHideTimer* self = new ( ELeave ) CSVGTPointerHideTimer( 
            aListener );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTPointerHideTimer::NewL
// Factory function for creating CSVGTPointerHideTimer objects.
// Returns: CSVGTPointerHideTimer* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTPointerHideTimer* CSVGTPointerHideTimer::NewL( 
    MSVGTPointerInactivityListener* aListener ) // Listener for inactivity event
    {
    CSVGTPointerHideTimer* self = NewLC( aListener );
    CleanupStack::Pop( self );
    return self;
    }


// Destructor
CSVGTPointerHideTimer::~CSVGTPointerHideTimer()
    {
    // Reset the listener
    iInactivityListener = NULL;
    // Cancel any pending timer events 
    Cancel();
    }

// -----------------------------------------------------------------------------
// CSVGTPointerHideTimer::DoCancel
// Reimplements CActive::DoCancel. 
// -----------------------------------------------------------------------------
//
void CSVGTPointerHideTimer::DoCancel()
    {
    CTimer::DoCancel();
    }

// -----------------------------------------------------------------------------
// CSVGTPointerHideTimer::RunL
// Reimplements CActive::RunL. This function resets the inactivity timer 
// so that the control is triggered to hide the pointer when no 
// activity occurs for a specified time period
// -----------------------------------------------------------------------------
//
void CSVGTPointerHideTimer::RunL()
    {
    // Callback 
    if ( iInactivityListener )
        {
        iInactivityListener->PointerInactivityTimeout();
        }
    }

// End of File
