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
* Description:  This file implements the timer functionality required for the
*                progress time functionality in the dialog.
*
*/


// User Includes
#include "SVGTProgressBarTimer.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTProgressBarTimer::CSVGTProgressBarTimer
// Default Constructor. Initialises Timer State Variables.
// -----------------------------------------------------------------------------
//
CSVGTProgressBarTimer::CSVGTProgressBarTimer(MSVGTProgressBarListener* 
            aListener , TBool aResetInactivityTime ) : 
    CTimer( CActive::EPriorityHigh ), iListener( aListener ), 
    iUseInactivityTime( aResetInactivityTime )
    {
    }

// -----------------------------------------------------------------------------
// CSVGTProgressBarTimer::ConstructL
// Second Phase Constructor for CSVGTProgressBarTimer. Adds this object to the 
// applications active scheduler.
// -----------------------------------------------------------------------------
//
void CSVGTProgressBarTimer::ConstructL()
    {
    // Call the base class ConstructL
    CTimer::ConstructL();
    // Add to active scheduler
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CSVGTProgressBarTimer::NewLC
// Factory function for creating CSVGTProgressBarTimer objects.It also pushes the
// created dialog object onto the cleanup stack.
// Returns: CSVGTProgressBarTimer* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTProgressBarTimer* CSVGTProgressBarTimer::NewLC( MSVGTProgressBarListener* 
            aListener , 
            TBool aResetInactivityTime ) // Listener for timer tick event
    {
    CSVGTProgressBarTimer* self = new ( ELeave ) CSVGTProgressBarTimer( 
        aListener , aResetInactivityTime );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTProgressBarTimer::NewL
// Factory function for creating CSVGTProgressBarTimer objects.
// Returns: CSVGTProgressBarTimer* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTProgressBarTimer* CSVGTProgressBarTimer::NewL( MSVGTProgressBarListener* 
            aListener , 
            TBool aResetInactivityTime ) // Listener for timer tick event
    {
    CSVGTProgressBarTimer* self = NewLC( aListener , aResetInactivityTime );
    CleanupStack::Pop( self );
    return self;
    }


// Destructor
CSVGTProgressBarTimer::~CSVGTProgressBarTimer()
    {
    // Reset the listener
    iListener = NULL;
    // Cancel any pending timer events if any.
    Cancel();
    }
    
// -----------------------------------------------------------------------------
// CSVGTProgressBarTimer::DoCancel
// Reimplements CActive::DoCancel. 
// -----------------------------------------------------------------------------
//
void CSVGTProgressBarTimer::DoCancel()
    {
    CTimer::DoCancel();
    }

// -----------------------------------------------------------------------------
// CSVGTProgressBarTimer::RunL
// Reimplements CActive::RunL. This function updates the media time.
// -----------------------------------------------------------------------------
//
void CSVGTProgressBarTimer::RunL()
    {
    if ( iUseInactivityTime )
        {
        User::ResetInactivityTime();        
        }
            
    // CallBack listener
    if ( iListener )
        {
        iListener->UpdateProgressBar();
        }
    }

// End of File
