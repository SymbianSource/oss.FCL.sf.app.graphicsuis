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
* Description:  SVGT Controller implements the active objects for the 
*                progressive rendering feature(Redraw, Thread Exit)
*
*/


//SvgtController.h

#ifndef SVGTCONTROLLER_H
#define SVGTCONTROLLER_H

#include <e32base.h>
#include <f32file.h>

const TInt KAnimationLength = 10;
const TInt KAnimatioBufferLength = 20;
class CSVGTCustControl;
class CAknNavigationDecorator;
class CEikonEnv;

/**
*  CSvgtThreadController is a Active Object (CActive) used to
*  notify thread death event to the main thread to do the post load operations
*
*  @lib SVGTUIControl.lib
*  @since 3.1
*/

class CSvgtThreadController : public CActive
    {
    public:
        /**
        * Two phase constructor
        * @since 3.1
        * @param aEngine Engine object 
        * @param aCustControl
        * @return CSvgtThreadController*
        */
        static CSvgtThreadController* NewL(CSVGTCustControl* aCustControl);

        /**
        * Two phase constructor which pushes the object on cleanup stack
        * @since 3.1
        * @param aEngine Engine object 
        * @param aCustControl
        * @return CSvgtThreadController*
        */
        static CSvgtThreadController* NewLC(CSVGTCustControl* aCustControl);

        /**
        * Destructor
        * @since 3.1
        */
        ~CSvgtThreadController();
        
        /**
        * Makes it active to get notify on thread death
        * @since 3.1
        * @param aThread Main thread object 
        */
        void IssueThreadMonitorRequest(const RThread& aThread);

        /**
        * Stops the thread execution
        * @since 3.1
        * @param aThread Main thread object 
        */
        void StopThreadExecution(const RThread& aThread);
        
    private:
        /**
        * Cancels all the request
        * @since 3.1
        * @see CActive
        */
        void DoCancel();

        /**
        * Handles an active object's request completion event.
        * @since 3.1
        * @see CActive
        */
        void RunL();

       /**
        * Parameterized constructor
        * @since 3.1
        * @param aEngine Engine object 
        * @param aCustControl
        * @param aMainThread
        */
        CSvgtThreadController(CSVGTCustControl* aCustControl);
        /**
        * Two phase constructor
        * @since 3.1
        */
        void ConstructL();

        /**
        * This fuction does the loading animation
        * @since 3.1
        */
        void DoLoadAnimationL();
        
        /**
        * Callback fuction for loading animation
        * @since 3.1
        * @param aThreadController Stores the object of type CSvgtThreadController
        */
        static TInt LoadAnimationCallBack(TAny* aThreadController);
        
        /**
        * Clears the navi pane indicator when the loading animation gets over
        * @since 3.1
        * @param aThreadController Stores the object of type CSvgtThreadController
        */
        void ClearNaviPaneDecorator();
        
    private:    //data members
        // Custom control object
        CSVGTCustControl* iCustControl;
        
        // Periodic timer to do loading animation
        CPeriodic* iLoadAnimator;
        
        // Counts the number steps in the animation
        TInt iCount;
        
        // Stores the text to display in the context bar while loading animation
        TBuf<KAnimatioBufferLength> iAnimationText;
        
        // Stores the the navigation pane decorator
        CAknNavigationDecorator* iNaviDecorator;
        
        // Stores the main thread id
        TThreadId iMainThread;
       
        // Application enviornment
        CEikonEnv* iEikEnv;
    };

#endif //SVGTCONTROLLER_H

// End of File

