/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class handles the event list & process the events 
*                 while progressive rendering
*
*/


#include "SvgtEventHandlerAO.h"
#include <SVGTAppObserverUtil.h>
#include <imcvcodc.h>
#include "SvgtEvent.h"

const TInt KTimerIntervalInMiliSeconds = 1000;

const TInt KInitialTimeInterval = 100;
const TInt KMaxTimeInterval = 2000;
// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::NewL
// Two phase construction
// -----------------------------------------------------------------------------
//
CSvgtEventHandlerAO* CSvgtEventHandlerAO::NewL(MSvgtAppObserver* aAppObserverUtil, 
                                CSVGTCustControl* aCustControl ,
                                const TThreadId aMainThreadId )
    {
    CSvgtEventHandlerAO* self = new ( ELeave ) CSvgtEventHandlerAO( aAppObserverUtil , 
                                                                    aCustControl, 
                                                                    aMainThreadId);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::CSvgtEventHandlerAO
// Constructor
// -----------------------------------------------------------------------------
//
CSvgtEventHandlerAO::CSvgtEventHandlerAO(MSvgtAppObserver* aAppObserverUtil, 
                                        CSVGTCustControl* aCustControl, 
                                        const TThreadId aMainThreadId):
                                        CActive( CActive::EPriorityStandard ),
                                        iCustControl(aCustControl),
                                        iMainThreadId( aMainThreadId ),
                                        iIsDocumentComplete( EFalse ),
                                        iStepCount( 1 ),
                                        iSvgTimeBetweenRedraw( KInitialTimeInterval )
    {
    iAppObserverUtil = static_cast<CSVGTAppObserverUtil*>(aAppObserverUtil);
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::ConstructL
// 2nd phase constructor
// -----------------------------------------------------------------------------
//
void CSvgtEventHandlerAO::ConstructL()
    {
    iPreviousRedrawClock.HomeTime();
    // Add to active scheduler
    CActiveScheduler::Add( this );
    // Set the status as pending
    iStatus = KRequestPending;
    // Set the active object as Active
    SetActive();
    // Create the critical section IPC for sync. the RequestComplete Calls
    iCritSection.CreateLocal();
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::~CSvgtEventHandlerAO
// Destructor
// -----------------------------------------------------------------------------
//
CSvgtEventHandlerAO::~CSvgtEventHandlerAO()
    {
    Cancel();
    iEventList.ResetAndDestroy();
    iEventList.Close();
    iCritSection.Close();
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::AddEventToList
// It adds the event (except redraw event) to the event queue.
// -----------------------------------------------------------------------------
//
TBool CSvgtEventHandlerAO::AddEventToList( CSvgtEvent* aEvent)
    {
    iEventList.Append( aEvent );
    return ETrue;        
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::AddRedrawEventToList
// It adds the redraw event to the event queue.
// -----------------------------------------------------------------------------
//
TBool CSvgtEventHandlerAO::AddRedrawEventToList( const TBool aForceAdd )
    {
    TBool isEventAdded = EFalse;
    if(aForceAdd || IsRedrawTimeElapsed())
        {
        CSvgtEvent* redrawEvent = new CSvgtEventRedraw;
        iEventList.Append( redrawEvent );
        isEventAdded = ETrue;
        }
    
    return isEventAdded;        
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::MakeRequestComplete
// It make the request complete.
// -----------------------------------------------------------------------------
//
void CSvgtEventHandlerAO::MakeRequestComplete( TInt aError )
    {
    // The Asynchronouse service provider must only call the RequestComplete()
    // once for each request. Multiple completion events on a single active
    // object result in a stray signal panic.
    // Hence use a critical section to synchronize access to RequestComplete()
    iCritSection.Wait();
    if( ( IsActive() ) && ( iStatus == KRequestPending ) )
        { 
        // The asynchronous service provider is the loading thread and since 
        // this is called from both loading thread as well as main thread
        // use the RThread object to complete the request for active
        // object present in the main thread's active scheduler
        TRequestStatus* status = &iStatus;
        RThread mainThread;

        if( mainThread.Open( iMainThreadId) )
            {
            // Problem opening thread, ignore error and return : Should not happen.
            return;
            }
            
        mainThread.RequestComplete( status, aError );
        mainThread.Close();
        }
    iCritSection.Signal();
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::AddEventToList
// It indicates that document is document loading completed.
// -----------------------------------------------------------------------------
//
void CSvgtEventHandlerAO::SetDocumentComplete()
    {
    iIsDocumentComplete = ETrue;
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::RunL
// Handles an active object's request completion event.
// -----------------------------------------------------------------------------
//
void CSvgtEventHandlerAO::RunL()
    {
    // Set the Active Object Active always
    if( !IsActive() )
        {
        iStatus = KRequestPending;
        SetActive();
        }

    if(iEventList.Count())    
        {
        CSvgtEvent* event = iEventList[0];
        iEventList.Remove( 0 );
        switch(event->EventType())
            {
            case CSvgtEvent::ESvgtEventEmbededImage:
                {
                CSvgtEventEmbededImage* svgEvent = 
                                        static_cast<CSvgtEventEmbededImage*>(event);
                iAppObserverUtil->AssignEmbededDataL( svgEvent->ImageUri() );
                }
                break;
                
            case CSvgtEvent::ESvgtEventFetchImage:
                {
                CSvgtEventFetchImage* fetchEvent = 
                                        static_cast<CSvgtEventFetchImage*>(event);
                iAppObserverUtil->NewFetchImageData(fetchEvent->ImageUri());
                }
                break;
            case CSvgtEvent::ESvgtEventLinkActivated:
                {
                CSvgtEventLinkActivated* linkEvent = 
                                            static_cast<CSvgtEventLinkActivated*>(event);
                iAppObserverUtil->LinkActivated(linkEvent->ImageUri());
                }
                break;
            case CSvgtEvent::ESvgtEventLinkActivatedWithShow:
                {
                CSvgtEventLinkActivatedWithShow* linkShowEvent = 
                                static_cast<CSvgtEventLinkActivatedWithShow*>(event);
                iAppObserverUtil->LinkActivatedWithShow(linkShowEvent->ImageUri(), 
                                                        linkShowEvent->Show());
                }
                break;
            case CSvgtEvent::ESvgtEventRedraw:
                {
                //DO REDRAW
                iCustControl->PerformEngineRedraw();
                iCustControl->DrawNow();
                }
                break;
                
            default:
                break;
            }

        delete event;
        }


        if( iEventList.Count() )
            {
            // Pending events in the list, reschedule active
            // object
            MakeRequestComplete( KErrNone );
            }
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::DoCancel
// Cancels all the pending request
// -----------------------------------------------------------------------------
//
void CSvgtEventHandlerAO::DoCancel()
    {
    // The service provided for this active object is the 
    // loading thread. Since the loading thread is already indicated to stop
    // generating requests, need to terminate the request
    // pending in the main thread
    DoTerminate();
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::RunError
// Handles a leave occurring in the request completion event handler RunL().
// -----------------------------------------------------------------------------
//
TInt CSvgtEventHandlerAO::RunError( TInt aError )
    {
    // When an error occurs, call base class error handler
    // Note that active object should not be Cancel() here
    // as Loading thread could still be alive and do a
    // MakeRequestComplete. This would generate a stray
    // signal.
    return ( CActive::RunError( aError ) );
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::IsRedrawTimeElapsed
// It determines whether the time elapsed or not.
// -----------------------------------------------------------------------------
//
TBool CSvgtEventHandlerAO::IsRedrawTimeElapsed()
    {
    TBool isTimeElapsed = EFalse;
    TTime currentClock;
    currentClock.HomeTime();

    TTimeIntervalMicroSeconds time64;
    time64 = currentClock.MicroSecondsFrom( iPreviousRedrawClock );

    TUint32 timeDelta = I64INT( time64.Int64() ) / KTimerIntervalInMiliSeconds; 
                                                            // milliseconds
    
    if ( timeDelta > iSvgTimeBetweenRedraw )//KSvgTimeBetweenRedraw=500  
        {
        if( iSvgTimeBetweenRedraw < KMaxTimeInterval )
            {
            iSvgTimeBetweenRedraw *= ++iStepCount;
            }
        isTimeElapsed = ETrue;
        // Save the current time to calculate interframe delay
        iPreviousRedrawClock.HomeTime();        
        }

    return isTimeElapsed;
    }

// -----------------------------------------------------------------------------
// CSvgtEventHandlerAO::DoTerminate
// Cancels the pending async request 
// -----------------------------------------------------------------------------
//
void CSvgtEventHandlerAO::DoTerminate()
    {
    // Post a pseudo complete, so that Cancel() is done 
    // successfully. The Loading thread which is the actual service
    // provider is by now already stopped.
    // The Asynchronous service provider should complete the 
    // request with KErrCancel as quickly as possible, because CActive::Cancel
    // blocks until completion occurs.
    MakeRequestComplete( KErrCancel );    
    }


//End of file
