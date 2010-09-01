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
* Description:  This file implements the active objects used for progressive 
*                rendering 
*
*/


#include "SvgtController.h"
#include <eikenv.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <eikspane.h>
#include "SVGTCustControl.h"

const TInt KAnimationInerval = 100000;

_LIT(KDot, ". ");

/*
*************************************************************************************
*   ThreadController Implemetation
*
***************************************************************************************
*/
// -----------------------------------------------------------------------------
// CSvgtThreadController::NewL
// Two phase constructor
// -----------------------------------------------------------------------------
//
CSvgtThreadController* CSvgtThreadController::NewL(CSVGTCustControl* aCustControl)
    {
    CSvgtThreadController* self = CSvgtThreadController::NewLC(aCustControl);
    CleanupStack::Pop(self);    //self
    return self;
    }
    
// -----------------------------------------------------------------------------
// CSvgtThreadController::NewLC
// Two phase constructor
// -----------------------------------------------------------------------------
//
CSvgtThreadController* CSvgtThreadController::NewLC(CSVGTCustControl* aCustControl)
    {
    CSvgtThreadController* self = new (ELeave) CSvgtThreadController(aCustControl);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CSvgtThreadController::~CSvgtThreadController
// Destructor
// -----------------------------------------------------------------------------
//
CSvgtThreadController::~CSvgtThreadController()
    {
    Cancel();
    if(iLoadAnimator)
        {
        iLoadAnimator->Cancel();
        }
        
    delete iLoadAnimator;
    ClearNaviPaneDecorator();
    // Reset eikon env pointer
    iEikEnv = NULL;
    // Reset the custom control pointer
    iCustControl = NULL;
    }
    
// -----------------------------------------------------------------------------
// CSvgtThreadController::DoCancel
// Cancels all the pending request
// -----------------------------------------------------------------------------
//
void CSvgtThreadController::DoCancel()
    {
    }
    
// -----------------------------------------------------------------------------
// CSvgtThreadController::RunL
// Handles an active object's request completion event.
// -----------------------------------------------------------------------------
//
void CSvgtThreadController::RunL()
    {
    iCustControl->SetThreadRunning(EFalse);
    iLoadAnimator->Cancel();
    iCount = KAnimationLength;
    DoLoadAnimationL();
    TInt lLoadError = iCustControl->DoHandleLoadingThreadError();
    TInt lPostLoadError = KErrNone;
    
    // Stop displaying progressive-render indicator
    ClearNaviPaneDecorator();
    //iCustControl->StopEventHandler();
    if( lLoadError == KErrNone )
        {
        // No Load error, Do the post load functionality
        TRAP( lPostLoadError, iCustControl->DoPostLoadFuncL() );    
        }
    
    // If there were any errors then ask parent to exit
    if ( ( lLoadError!= KErrNone ) || ( lPostLoadError != KErrNone ) )
        {
        // Error has occurred, commence cleanup 
        // Request Observer to Exit
   	    iCustControl->DoExitFromDialogL();
        }
    }
    
// -----------------------------------------------------------------------------
// CSvgtThreadController::CSvgtThreadController
// Parameterized constructor
// -----------------------------------------------------------------------------
//
CSvgtThreadController::CSvgtThreadController(CSVGTCustControl* aCustControl):
                                                CActive(EPriorityStandard),
                                                iCustControl(aCustControl)
    {
    CActiveScheduler::Add(this);
    }
    
// -----------------------------------------------------------------------------
// CSvgtThreadController::ConstructL
// Two phase constructor
// -----------------------------------------------------------------------------
//
void CSvgtThreadController::ConstructL()
    {
    iLoadAnimator = CPeriodic::NewL(CActive::EPriorityStandard);
    
    // Store the environment pointer to avoid slow static access
    iEikEnv = CEikonEnv::Static();
    User::LeaveIfNull( iEikEnv );
    }
    
// -----------------------------------------------------------------------------
// CSvgtThreadController::IssueThreadMonitorRequest
// Makes it active to get notify on thread death
// -----------------------------------------------------------------------------
//
void CSvgtThreadController::IssueThreadMonitorRequest(const RThread& aThread)
    {
    aThread.Logon(iStatus);
    iMainThread = aThread.Id();
    SetActive();

    TCallBack callBack(LoadAnimationCallBack, this);
    iCount  = 0;
    iLoadAnimator->Start(0, KAnimationInerval, callBack);
    }
    
// -----------------------------------------------------------------------------
// CSvgtThreadController::StopThreadExecution
// Stops the thread execution
// -----------------------------------------------------------------------------
//
void CSvgtThreadController::StopThreadExecution(const RThread& aThread)
    {
    aThread.LogonCancel(iStatus);
    }

// -----------------------------------------------------------------------------
// CSvgtThreadController::DoLoadAnimationL
// This fuction does the loading animation
// -----------------------------------------------------------------------------
//
void CSvgtThreadController::DoLoadAnimationL()
    {
    CEikStatusPane* sp = iEikEnv->AppUiFactory()->StatusPane();
    CAknNavigationControlContainer* np = 
        static_cast<CAknNavigationControlContainer*>
            ( sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) );

    if(iCount >= KAnimationLength)
        {
        iCount = 0;
        iAnimationText.Zero();
        }
    else
        {
        iCount++;
        iAnimationText.Append(KDot);
        }

    CAknNavigationDecorator* lOldNaviDecorator = iNaviDecorator;
    iNaviDecorator = NULL;
    if ( lOldNaviDecorator )
        {
        delete lOldNaviDecorator ;
        lOldNaviDecorator = NULL;
        }

    // Clear the old Navi-Pane Decorator
    // Create the navi pane label            
    iNaviDecorator = np->CreateNavigationLabelL( iAnimationText );

    // Save the old Navi Decorator so that it can be deleted after 
    // creating the new navi decorator 
    // Push the new Navi decorator in the Navigation Pane
    np->PushL( *iNaviDecorator );
    
    }
    
// -----------------------------------------------------------------------------
// CSvgtThreadController::LoadAnimationCallBack
// Callback fuction for loading animation
// -----------------------------------------------------------------------------
//
TInt CSvgtThreadController::LoadAnimationCallBack(TAny* aThreadController)
    {
    CSvgtThreadController* threadController = 
        static_cast< CSvgtThreadController* >( aThreadController );
    if ( threadController )
        {
        TRAPD( ignore, threadController->DoLoadAnimationL() );
        if ( ignore != KErrNone )
            {
            // No error handling done.
            }
        }
    return 0;
    }

// -----------------------------------------------------------------------------
// CSvgtThreadController::ClearNaviPaneDecorator
// Clears the navi pane indicator when the loading animation gets over
// -----------------------------------------------------------------------------
//
void CSvgtThreadController::ClearNaviPaneDecorator()
    {
    // Clear the Navi-Pane Decorator    
    if ( iNaviDecorator )
        {
        // Restore the old navi-pane
        CEikStatusPane* sp = iEikEnv->AppUiFactory()->StatusPane();
        
        CAknNavigationControlContainer* np = NULL;
        TRAPD( errGetNaviControl, 
            np = static_cast< CAknNavigationControlContainer* >
            ( sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) ) );
        if ( errGetNaviControl == KErrNone && np )
            {
            TRAPD( errPushDefNaviControl, 
                np->PushDefaultL ()); // Set default.    
            if ( errPushDefNaviControl != KErrNone )
                {
                // No error handling here.
                }
            }
        delete iNaviDecorator;
        iNaviDecorator = NULL;
        }
    }
// End of file

