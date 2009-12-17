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
* Description:  DRM helper for svgt files.
*
*/



// INCLUDE FILES
#include    <DRMHelper.h>
#include    <bamdesca.h>
#include    <bautils.h>
#include    <coemain.h>
#include    <StringLoader.h>
#include    <aknnotewrappers.h>
#include    <caf/content.h>
#include    <caf/attribute.h>
#include    <caf/caftypes.h>
#include    <caf/caferr.h>
#include    <Oma2Agent.h>

#include    "SVGTDrmHelper.h"

// CONSTANTS

const TUint32 KSVGTCountConstraint = 3;
const TInt KSVGTTimeConstraint = 7;
const TInt KSVGTSecondsInDay = 86400;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::CSVGTDrmHelper
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSVGTDrmHelper::CSVGTDrmHelper() :      
    iEnv(CCoeEnv::Static())
    {
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSVGTDrmHelper::ConstructL()
    {
    iDrmHelper = CDRMHelper::NewL( *iEnv );
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSVGTDrmHelper* CSVGTDrmHelper::NewL()
    {
    CSVGTDrmHelper* self = CSVGTDrmHelper::NewLC();    
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSVGTDrmHelper* CSVGTDrmHelper::NewLC()
    {
    CSVGTDrmHelper* self = new(ELeave) CSVGTDrmHelper();    
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::~CSVGTDrmHelper
// Destructor
// -----------------------------------------------------------------------------
//
CSVGTDrmHelper::~CSVGTDrmHelper()
    {
    delete iRightsConstraints;  

    // Delete the DRM Helper
    delete iDrmHelper;
    // Reset the environment pointer
    iEnv = NULL;
    }
    
// -----------------------------------------------------------------------------
// CSVGTDrmHelper::IsPreviewL
// -----------------------------------------------------------------------------
//        
TBool CSVGTDrmHelper::IsPreviewL( RFile& aFileHandle )    
    {
    LoadUsageRigthsInfoL( aFileHandle );        
    
    TSVGTRightsStatus rightsStatus;
    GetUsageRightsStatus( rightsStatus );         
    return ( rightsStatus == ESVGTPreview );  

    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::CheckRightsAmountL
// -----------------------------------------------------------------------------
//     
TInt CSVGTDrmHelper::CheckRightsAmountL( RFile& aFileHandle )
    {
    LoadUsageRigthsInfoL( aFileHandle );
    if ( iProtected )
        {
        return iDrmHelper->CheckRightsAmountL( aFileHandle );   
        }
    else
        {
        return 0;
        }
    }
// -----------------------------------------------------------------------------
// CSVGTDrmHelper::GetDrmDetailsL
// -----------------------------------------------------------------------------
// 
void CSVGTDrmHelper::GetDrmDetailsL( RFile& aFileHandle,
                         TBool& aRightsValid, TBool& aIsSeparateDeliveryNS, 
                         TBool& aIsProtected )
    {


    LoadUsageRigthsInfoL( aFileHandle );     
    
    aIsProtected = IsProtected();    
    
    if( !aIsProtected )
        {       
        return;
        }             
   
    TSVGTRightsStatus rightsStatus;
    GetUsageRightsStatus( rightsStatus );       
    aRightsValid = ( rightsStatus == ESVGTFull ) ||
                   ( rightsStatus == ESVGTRestricted ) ||
                   ( rightsStatus == ESVGTPreview ); 
                   
                   
    // Creates a handle to content handled via CAF
    CContent *content = CContent::NewLC( aFileHandle ); 
    content->GetAttribute( EIsForwardable, aIsSeparateDeliveryNS );
    CleanupStack::PopAndDestroy( content );     
    return;   
    
    }
    

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::LoadUsageRigthsInfoL
// -----------------------------------------------------------------------------
//
void CSVGTDrmHelper::LoadUsageRigthsInfoL( RFile& aFileHandle )
    {
    
    delete iRightsConstraints;
    iRightsConstraints = NULL;
    //flag to check for SD protection
    iIsSeparateDelivery=0;
    // Creates a handle to content handled via CAF
    CContent *content = CContent::NewLC( aFileHandle ); 
    content->GetAttribute( EIsProtected, iProtected );
    content->GetAttribute( EDeliveryMethod, iIsSeparateDelivery );
    CleanupStack::PopAndDestroy( content );

    if ( iProtected )
        {

        CDRMHelperRightsConstraints* ignore1 = NULL;
        CDRMHelperRightsConstraints* ignore2 = NULL;
        CDRMHelperRightsConstraints* ignore3 = NULL;
        

        TRAPD(error, iDrmHelper->GetRightsDetailsL(aFileHandle,
                                                   ContentAccess::EView,
                                                   iExpired,
                                                   iSendingAllowed,
                                                   ignore1,
                                                   iRightsConstraints,
                                                   ignore2,
                                                   ignore3));  
       
        // Delete ignored constraints
        delete ignore1;
        ignore1 = NULL;
        delete ignore2;
        ignore2 = NULL;
        delete ignore3;
        ignore3 = NULL;

        if ( error == KErrCANoRights )
            {
            delete iRightsConstraints;
            iRightsConstraints = NULL;
            }
        else
            {
            User::LeaveIfError(error);
            }
        }
    else
        {
        iExpired = EFalse;
        iSendingAllowed = ETrue;
        }     

    iDrmHelper->CanSetAutomated(aFileHandle, iCanSetAutomated);
    
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::GetUsageRightsStatus
// -----------------------------------------------------------------------------
//
void CSVGTDrmHelper::GetUsageRightsStatus(TSVGTRightsStatus& aStatus)
    {
    if (!iProtected)
        {
        aStatus = ESVGTFull;
        }
    else if (iRightsConstraints)
        {
        if (iExpired)
            {
            aStatus = ESVGTExpired;
            }
        else if (iRightsConstraints->IsPreview())
            {
            	if(!(iIsSeparateDelivery==EOmaDrm2||iIsSeparateDelivery==EOmaDrm1SeparateDelivery))
            	{
            	aStatus = ESVGTPreview;	
            	}
            }
        else if (iRightsConstraints->FullRights())
            {
            aStatus = ESVGTFull;
            }
        else
            {
            TTime time;
            TTime startTime;

            time.HomeTime();
            TInt error = GetStartTime(startTime);

            // Check that usage time has already started
            if (!error && time < startTime)
                {
                aStatus = ESVGTExpired;
                }
            else
                {
                aStatus = ESVGTRestricted;
                }
            }
        }
    else
        {
        aStatus = ESVGTMissing;
        }
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::GetExpireStatus
// -----------------------------------------------------------------------------
//
void CSVGTDrmHelper::GetExpireStatus(TBool& aAboutToExpire,
                                            TSVGTExpireConstraint& aConstraint,
                                            TInt& aCountLeft,
                                            TInt& aDaysLeft)
    {
    TSVGTRightsStatus status;
    GetUsageRightsStatus(status);

    if (status == ESVGTRestricted)
        {
        TUint32 count = 0;
        TInt countError = GetCount(count);
        
        TTime endTime;
        TInt timeError = GetEndTime(endTime);
        TTime currentTime;
        currentTime.HomeTime();
        TInt diff = endTime.DaysFrom(currentTime).Int();

        TTimeIntervalSeconds interval;
        TInt intervalError = GetInterval(interval);
        TInt intervalDiff = 0;
        if (!intervalError)
            {
            TTime intervalStart;
            TTimeIntervalSeconds secondsFromStart;

            if (!GetIntervalStart(intervalStart))
                {
                // No error => interval has already started
                currentTime.SecondsFrom(intervalStart, secondsFromStart);
                intervalDiff = 
                    (interval.Int() - secondsFromStart.Int()) / KSVGTSecondsInDay;
                }
            else
                {
                // Error occurred => interval not started yet
                intervalDiff = interval.Int() / KSVGTSecondsInDay;
                }
            }

        if (!countError && count <= KSVGTCountConstraint &&
            ((!timeError && diff <= KSVGTTimeConstraint) ||
            (!intervalError && intervalDiff <= KSVGTTimeConstraint)))
            {
            // Count and time or interval contraints about to expire
            aConstraint = ESVGTCountAndTime;
            aCountLeft = count;
            aAboutToExpire = ETrue;

            if (!timeError && !intervalError)
                {
                // Both constraints exist => use smaller one
                aDaysLeft = Min(diff, intervalDiff);
                }
            else if (!timeError)
                {
                // Only time constraint exists => use it
                aDaysLeft = diff;
                }
            else
                {
                // Only interval constraint exists => use it
                aDaysLeft = intervalDiff;
                }
            }
        else if (!countError && count <= KSVGTCountConstraint)
            {
            // Count constraint about to expire
            aConstraint = ESVGTCount;
            aCountLeft = count;
            aAboutToExpire = ETrue;
            }
        else if ((!timeError && diff <= KSVGTTimeConstraint) ||
                 (!intervalError && intervalDiff <= KSVGTTimeConstraint))
            {
            // Time or interval constraint about to expire
            aConstraint = ESVGTTime;
            aAboutToExpire = ETrue;

            if (!timeError && !intervalError)
                {
                // Both constraints exist => use smaller one
                aDaysLeft = Min(diff, intervalDiff);
                }
            else if (!timeError)
                {
                // Only time constraint exists => use it
                aDaysLeft = diff;
                }
            else
                {
                // Only Interval constraint exists => use it
                aDaysLeft = intervalDiff;
                }
            }
        else
            {
            // No constraint about to expire
            aAboutToExpire = EFalse;
            aConstraint = ESVGTNone;
            }
        }
    else
        {
        // Non-restricted object cannot expire
        aAboutToExpire = EFalse;
        }
    }


// -----------------------------------------------------------------------------
// CSVGTDrmHelper::SendingAllowed
// -----------------------------------------------------------------------------
//
TBool CSVGTDrmHelper::SendingAllowed() const
    {
    return iSendingAllowed;
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::GetCount
// -----------------------------------------------------------------------------
//
TInt CSVGTDrmHelper::GetCount(TUint32& aCount)
    {
    TInt error = KErrNone;

    if (!iProtected)
        {
        error = KErrNotFound;
        }
    else if (iRightsConstraints)
        {
        TUint32 ignore;
        TRAP(error, iRightsConstraints->GetCountersL(aCount, ignore));
        }
    else
        {
        error = KErrNotReady;
        }

    return error;
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::GetStartTime
// -----------------------------------------------------------------------------
//
TInt CSVGTDrmHelper::GetStartTime(TTime& aStartTime)
    {
    TInt error = KErrNone;

    if (!iProtected)
        {
        error = KErrNotFound;
        }
    else if (iRightsConstraints)
        {
        TRAP(error, iRightsConstraints->GetStartTimeL(aStartTime));
        }
    else
        {
        error = KErrNotReady;
        }

    return error;
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::GetEndTime
// -----------------------------------------------------------------------------
//
TInt CSVGTDrmHelper::GetEndTime(TTime& aEndTime)
    {
    TInt error = KErrNone;

    if (!iProtected)
        {
        error = KErrNotFound;
        }
    else if (iRightsConstraints)
        {
        TRAP(error, iRightsConstraints->GetEndTimeL(aEndTime));
        }
    else
        {
        error = KErrNotReady;
        }

    return error;
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::GetInterval
// -----------------------------------------------------------------------------
//
TInt CSVGTDrmHelper::GetInterval(TTimeIntervalSeconds& aInterval)
    {
    TInt error = KErrNone;

    if (!iProtected)
        {
        error = KErrNotFound;
        }
    else if (iRightsConstraints)
        {
        TRAP(error, iRightsConstraints->GetIntervalL(aInterval));
        }
    else
        {
        error = KErrNotReady;
        }

    return error;
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::GetIntervalStart
// -----------------------------------------------------------------------------
//
TInt CSVGTDrmHelper::GetIntervalStart(TTime& aStartTime)
    {
    TInt error = KErrNone;

    if (!iProtected)
        {
        error = KErrNotFound;
        }
    else if (iRightsConstraints)
        {
        TRAP(error, iRightsConstraints->GetIntervalStartL(aStartTime));
        }
    else
        {
        error = KErrNotReady;
        }

    return error;
    }
    
// -----------------------------------------------------------------------------
// CSVGTDrmHelper::IsProtected
// -----------------------------------------------------------------------------
//
TBool CSVGTDrmHelper::IsProtected() const
    {
    return iProtected;
    }

// -----------------------------------------------------------------------------
// CSVGTDrmHelper::CanSetAutomated
// -----------------------------------------------------------------------------
//
TBool CSVGTDrmHelper::CanSetAutomated() const
    {
    return iCanSetAutomated;
    }
    
//  End of File  
