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
* Description:  DRM helper for svg files.
*
*/



#ifndef SVGTDRMHELPER_H
#define SVGTDRMHELPER_H

// INCLUDES
#include <e32std.h>

// DATA TYPES
enum TSVGTRightsStatus
    {
    ESVGTFull,
    ESVGTMissing,
    ESVGTRestricted,
    ESVGTExpired,
    ESVGTPreview
    };

enum TSVGTExpireConstraint
    {
    ESVGTNone,
    ESVGTCount,
    ESVGTTime,
    ESVGTCountAndTime
    };

// CONSTANTS
const TInt KSVGTRightsAboutToExpire = -40000;

// FORWARD DECLARATIONS
class CDRMHelper;
class CDRMHelperRightsConstraints;
class MDesCArray;
class CCoeEnv;

// CLASS DECLARATION

/**
*  DRM helper for SVG files.
*
*  @lib SVGTUIControl.dll
*  @since 3.0
*/
class CSVGTDrmHelper : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.       
        */
        static CSVGTDrmHelper* NewL();
        
        /**
        * Two-phased constructor.       
        */
        static CSVGTDrmHelper* NewLC();   
                
        /**
        * Destructor.
        */
        virtual ~CSVGTDrmHelper();

    public: // New functions
    

        /**
        * This Function checks whether the file has preview rights.
        * @since 3.0
        * @param aFileHandle File handle for which usage rights info is loaded.
        * @return True if the file has only preview rights
        *         False if not.
        */
        TBool IsPreviewL( RFile& aFileHandle );
       
        /**
        * This Function checks the rights associated with the file.
        * @since 3.0
        * @param aFileHandle File handle for which usage rights is checked.
        * @return Button code
        */
        TInt CheckRightsAmountL( RFile& aFileHandle );
        

        /**
        * This Function gets the drm details needed for thumbnail generation.
        * @since 3.0
        * @param aFileHandle File handle for which usage rights info is loaded.
        * @param aRightsValid True if the rights are valid
        * @param aIsSeparateDeliveryNS True if the file is a separate delivery 
        *               drm file non-supersitributable
        * @param aIsProtected True if the file is protected
        */
        void GetDrmDetailsL( RFile& aFileHandle,
                             TBool& aRightsValid,
                             TBool& aIsSeparateDeliveryNS, 
                             TBool& aIsProtected );
                 
    
        /**
        * Loads usage rights information for a svg file using 
        * CDRMHelper. The loaded constraint is CContentAccess::EPlay.
        * @since 3.0
        * @param aFileHandle File handle for which usage rights info is loaded.
        */
        void LoadUsageRigthsInfoL( RFile& aFileHandle );


        /**
        * Gets usage rigths status.
        * @since 3.0
        * @param aStatus:
        *           ESVGTFull:        Full rights
        *           ESVGTMissing:     Rights missing
        *           ESVGTRestricted:  Restricted rights
        *           ESVGTExpired:     Expired rights (could be also future rights)
        *           ESVGTPreview:     Preview rights
        */
        void GetUsageRightsStatus( TSVGTRightsStatus& aStatus );

        /**
        * Checks if the usage rigths are about to expire. If they are, returns 
        * info about the rights contraints that are about to expire.
        * @since 3.0
        * @param aAboutToExpire Boolean telling if the rigths are about to
        *                       expire or not.
        * @param aConstraint Constraint that is about to expire:
        *                    ESVGTCount:          Usage count about to expire
        *                    ESVGTTime:           Usage time about to expire
        *                    ESVGTCountAndTime:   Usage count and time about to
        *                                       expire
        * @param aCountLeft Usage counts left (only if count is about to expire)
        * @param aDaysLeft Usage days left (only if time is about to expire)
        */
        void GetExpireStatus( TBool& aAboutToExpire,
                                      TSVGTExpireConstraint& aConstraint,
                                      TInt& aCountLeft,
                                      TInt& aDaysLeft );

        /**
        * Checks if sending of the file is allowed.
        * @since 3.0
        * @return ETrue:    Sending is allowed.
        *         EFalse:   Sending is not allowed.
        */
        TBool SendingAllowed() const;

        /**
        * Gets usage count.
        * @since 3.0
        * @param aCount Counts left.
        * @return Error code:
        *           KErrNone:       Count returned.
        *           KErrNotFound:   Object doesn't have count based rights.
        *           KErrNotReady:   Usage rights info not loaded.
        */
        TInt GetCount( TUint32& aCount );

        /**
        * Gets start time of time based rights.
        * @since 3.0
        * @param aStartTime Start time.
        * @return Error code:
        *           KErrNone:       Start time returned.
        *           KErrNotFound:   Object doesn't have time based rights.
        *           KErrNotReady:   Usage rights info not loaded.
        */
        TInt GetStartTime( TTime& aStartTime );

        /**
        * Gets end time of time based rights.
        * @since 3.0
        * @param aEndTime End time.
        * @return Error code:
        *           KErrNone:       End time returned.
        *           KErrNotFound:   Object doesn't have time based rights.
        *           KErrNotReady:   Usage rights info not loaded.
        */
        TInt GetEndTime( TTime& aEndTime );

        /**
        * Gets interval time constraint.
        * @since 3.0
        * @param aInterval Interval.
        * @return Error code:
        *           KErrNone:       Interval returned.
        *           KErrNotFound:   Object doesn't have interval constraint.
        *           KErrNotReady:   Usage rights info not loaded.
        */
        TInt GetInterval( TTimeIntervalSeconds& aInterval );

        /**
        * Gets start time of activated interval constraint.
        * @since 3.0
        * @param aStartTime Start time.
        * @return Error code:
        *           KErrNone:       Start time returned.
        *           KErrNotFound:   Interval has not been started yet.
        *           KErrNotReady:   Usage rights info not loaded.
        */
        TInt GetIntervalStart( TTime& aStartTime );
                
        /**
        * Provides info about files DRM protection status.
        * @since 3.0
        * @return ETrue:  The file is DRM protected.
        *         EFalse: The file is not DRM protected.
        */
        TBool IsProtected() const;

        /**
        * Checks if given content can be set as an automated content.
        * @since 3.0
        * @return  Boolean.
        */
        TBool CanSetAutomated() const;

    private: // Private contructors

        /**
        * C++ default constructor.
        */
        CSVGTDrmHelper();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();        

    private:    // Data
        CDRMHelper* iDrmHelper;
        CDRMHelperRightsConstraints* iRightsConstraints;
        CCoeEnv* iEnv;
        
        TBool iProtected;
        TBool iExpired;
        TBool iSendingAllowed;
        TBool iCanSetAutomated;       
    };

#endif      // SVGTDRMHELPER_H  
            
// End of File
