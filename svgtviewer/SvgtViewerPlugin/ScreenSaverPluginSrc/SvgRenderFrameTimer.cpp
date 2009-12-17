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
* Description:  ECOM Implementation Array.
*
*/


#include "SvgRenderFrameTimer.h"
#include "SvgRenderFrameControl.h"


// INCLUDE FILES
// -----------------------------------------------------------------------------
// CSvgRenderFrameTimer::CSvgRenderFrameTimer
// Default Constructor. Initialises Timer State Variables.
// -----------------------------------------------------------------------------
//
CSvgRenderFrameTimer::CSvgRenderFrameTimer( MSvgRenderFrameTimerListener* 
        aListener) : // Listener for the pointer inactivity
    CTimer( CActive::EPriorityStandard ), iListener( aListener )
    {
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameTimer::ConstructL
// Second Phase Constructor for CSvgRenderFrameTimer. Adds this object to the 
// applications active scheduler.
// -----------------------------------------------------------------------------
//
void CSvgRenderFrameTimer::ConstructL()
    {
    // Call the base class ConstructL
    CTimer::ConstructL();
    // Add to active scheduler
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameTimer::NewLC
// Factory function for creating CSvgRenderFrameTimer objects. It also 
// pushes the created dialog object onto the cleanup stack.
// Returns: CSvgRenderFrameTimer* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSvgRenderFrameTimer* CSvgRenderFrameTimer::NewLC(
    MSvgRenderFrameTimerListener* aListener ) // Listener for inactivity event
    {
    CSvgRenderFrameTimer* self = new ( ELeave ) CSvgRenderFrameTimer( 
            aListener );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameTimer::NewL
// Factory function for creating CSvgRenderFrameTimer objects.
// Returns: CSvgRenderFrameTimer* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSvgRenderFrameTimer* CSvgRenderFrameTimer::NewL( 
    MSvgRenderFrameTimerListener* aListener ) // Listener for inactivity event
    {
    CSvgRenderFrameTimer* self = NewLC( aListener );
    CleanupStack::Pop( self );
    return self;
    }


// Destructor
CSvgRenderFrameTimer::~CSvgRenderFrameTimer()
    {
    // Cancel any pending timer events 
    Cancel();
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameTimer::DoCancel
// Reimplements CActive::DoCancel. 
// -----------------------------------------------------------------------------
//
void CSvgRenderFrameTimer::DoCancel()
    {
    CTimer::DoCancel();
    }

// -----------------------------------------------------------------------------
// CSvgRenderFrameTimer::RunL
// Reimplements CActive::RunL. This function resets the inactivity timer 
// so that the control is triggered to hide the pointer when no 
// activity occurs for a specified time period
// -----------------------------------------------------------------------------
//
void CSvgRenderFrameTimer::RunL()
    {
    // Callback 
    if ( iListener )
        {
        iListener->AnimationTimeOut();
        }
    }

void CSvgRenderFrameTimer::SetTimeOut( 
    const TTimeIntervalMicroSeconds32& anInterval )
    {
    if (!IsActive())
        {
        After( anInterval );
        }
    }

// End of File

