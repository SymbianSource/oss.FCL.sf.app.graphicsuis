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
* Description:  This header file defines the  CSvgtEventHandlerAO class 
*
*/


#ifndef SVGTEVENTHANDLERAO_H
#define SVGTEVENTHANDLERAO_H

// INCLUDES
#include <e32base.h>
#include <e32cmn.h>
#include "SVGTCustControl.h"

class MSvgtAppObserver;
class CSVGTAppObserverUtil;
class CSvgtEvent;

_LIT(KSvgTagName, "svg");
_LIT(KDataTagName, "data:");

/**
*       This class handles the event list & process the events 
*       while progressive rendering
*/
class CSvgtEventHandlerAO : public CActive
    {
    public:
        /**
        * Two phase constructor
        * @since 3.1
        * @param aAppObserverUtil AppOberserverUtil Object
        * @param aEngine Engine object 
        * @param aCustControl
        * @return CSvgtEventHandlerAO*
        */
        static CSvgtEventHandlerAO* NewL(MSvgtAppObserver* aAppObserverUtil, 
                                            CSVGTCustControl* aCustControl,
                                            const TThreadId aMainThreadId );

        /**
        *  Destructor
        */
        virtual ~CSvgtEventHandlerAO();

        /**
        * It adds the event (except redraw event) to the event queue.
        * @since 3.1
        * @param aEvent Event Object
        * @return TBool
        */
        TBool AddEventToList(CSvgtEvent* aEvent);

        /**
        * It adds the redraw event to the event queue.
        * @since 3.1
        * @param aForceAdd Decides whether to add the redraw event forcefully
        * @return TBool
        */
        TBool AddRedrawEventToList( const TBool aForceAdd = EFalse);
        
        /**
        * It make the request complete.
        * @since 3.1
        * @param aError Error code for the service provided
        * @return none
        */
        void MakeRequestComplete( TInt aError );
        
        /**
        * It indicates that document is document loading completed.
        * @since 3.1
        * @return none
        */
        void SetDocumentComplete();
        
    private:
        /**
        * Handles an active object's request completion event.
        * @since 3.1
        * @see CActive
        */
        void RunL();
        
        /**
        * Cancels all the request
        * @since 3.1
        * @see CActive
        */
        void DoCancel();

        /**
        * Handles a leave occurring in the request completion event handler RunL().
        * @since 3.1
        * @see CActive
        */
        TInt RunError( TInt aError );

        /**
        * It determines whether the time elapsed or not.
        * @since 3.1
        * @return TBool
        */
        TBool IsRedrawTimeElapsed();

        /**
        * Terminates the active object
        * @since 3.1
        */
        void DoTerminate();
        
    private:    //class methods
        /**
        * constructor
        */
        CSvgtEventHandlerAO(MSvgtAppObserver* aAppObserverUtil, 
                                CSVGTCustControl* aCustControl, 
                                const TThreadId aMainThreadId );
        /**
        * Two phase constructor
        * @since 3.1
        */
        void ConstructL();

    private:    //Data members

        CSVGTAppObserverUtil* iAppObserverUtil;
        CSvgEngineInterfaceImpl* iSvgEngine;
        CSVGTCustControl* iCustControl;
        RPointerArray<CSvgtEvent>   iEventList;

        // Stores the previous time stamp
        TTime iPreviousRedrawClock;
        TThreadId iMainThreadId;
        TBool iIsDocumentComplete;
        
        TInt iStepCount;
        TInt iSvgTimeBetweenRedraw;
        // For Synchronising RequestComplete() calls
        RCriticalSection iCritSection;
    };

#endif  //SVGTEVENTHANDLERAO_H

// End of File
