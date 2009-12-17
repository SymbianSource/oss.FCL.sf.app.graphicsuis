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
* Description:  Declares svg file details dialog
*
*/



#ifndef SVGTFILEVIEWDETAILSDIALOG_H
#define SVGTFILEVIEWDETAILSDIALOG_H

//  INCLUDES
#include <e32base.h>
#include <aknpopup.h>
#include <SVGTDrmHelper.h>

// FORWARD DECLARATIONS

class CAknDoublePopupMenuStyleListBox;
class CEikonEnv;


// CLASS DECLARATIONS

/**
*  CSvgtFileDetails 
*  Container class for file details
*/
class CSvgtFileDetails : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        CSvgtFileDetails();
        
        /**
        * Destructor.
        */
        virtual ~CSvgtFileDetails();

    public:    // Data

        HBufC* iUrl;
        HBufC* iFormat;
        
        TInt   iSize;
        TTime  iTime;
   

        // Drm stuff
        TBool   iDrmProtected;
        TSVGTRightsStatus iDRMRightsStatus;
        TSVGTExpireConstraint iDRMExpireConstraint;
        TBool   iDRMForwardLocked;
        TUint32 iDRMCountsLeft;

        TBool   iDRMFileHasInterval;
        TBool   iDRMIntervalActive;
        TTimeIntervalSeconds iDRMInterval;

        TTime   iDRMValidFrom;
        TTime   iDRMValidUntil;
        
    };



/**
*  CSvgtFileViewDetailsDialog
*  Used to display "View Details" option
*  Makes use of SVGTdrmhelper to display the drm details
*  
*  @lib SVGTFileDetailsDialog.lib
*  @since 2.0
*/
class CSvgtFileViewDetailsDialog : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CSvgtFileViewDetailsDialog* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CSvgtFileViewDetailsDialog();

    public: // New functions

    
        /**
        * Shows file details to user in dialog. 
        * @since 3.0
        * @param aFileHandle File handle for which usage rights info is loaded.
        * @leaves Leaves with KErrNotSupported if file is not valid media file.
        * @return void
        */
        IMPORT_C void ExecuteLD( RFile& aFileHandle );        
  
    private:

        /**
        * C++ default constructor.
        */
        CSvgtFileViewDetailsDialog();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private: // New functions
        
        /**
        * Sets title for CAknPopupList.
        * @since 3.0
        * @param aPopupList CAknPopupList which title is modified
        * @return void
        */
        void SetTitleL( CAknPopupList* aPopupList ) const;

        /**
        * Fills listbox with file details information.
        * @since 3.0
        * @param aFileDetails File details
        * @return void
        */
        void FillListBoxL( const CSvgtFileDetails* aFileDetails );

        /**
        * Adds header and value to list.
        * @since 3.0
        * @param aHeading Heading
        * @param aValue Value
        * @param aItemArray Array where value and header are added.
        * @return void
        */
        void AddItemToListBoxL( const TDesC& aHeading, const TDesC& aValue,
                CDesCArray* aItemArray ) const;

        
        /**
        * Constructs clip url item. 
        * @since 3.0
        * @param aItemArray Array where constructed item is put.
        * @param aFileDetails File details.
        * @return void
        */
        void MakeUrlItemL( CDesCArray* aItemArray, 
                const CSvgtFileDetails* aFileDetails ) const;
        
        /**
        * Constructs media format item. 
        * @since 3.0
        * @param aItemArray Array where constructed item is put.
        * @param aFileDetails File details.
        * @return void
        */
        void MakeFormatItemL( CDesCArray* aItemArray, 
               const CSvgtFileDetails* aFileDetails ) const;


        /**
        * Constructs file size item. 
        * @since 3.0
        * @param aItemArray Array where constructed item is put.
        * @param aFileDetails File details.
        * @return void
        */
        void MakeSizeItemL( CDesCArray* aItemArray, 
               const CSvgtFileDetails* aFileDetails ) const;

        /**
        * Constructs date item. 
        * @since 3.0
        * @param aItemArray Array where constructed item is put.
        * @param aFileDetails File details.
        * @return void
        */
        void MakeDateItemL( CDesCArray* aItemArray, 
           const CSvgtFileDetails* aFileDetails ) const;

        /**
        * Constructs time item. 
        * @since 3.0
        * @param aItemArray Array where constructed item is put.
        * @param aFileDetails File details.
        * @return void
        */
        void MakeTimeItemL( CDesCArray* aItemArray, 
             const CSvgtFileDetails* aFileDetails ) const;
        /**
        * Converts between arabic-indic digits and european digits.
        * @since 3.0
        * @param aText numbers to be converted.
        * @return void
        */
        void LanguageSpecificNumberConversion( TDes& aText) const;

        /**
        * Fetches drm file details.
        * @since 3.0
        * @param aFileHandle File handle for which usage rights info is loaded.
        * @param aFileDetails Pointer to the details to be fetched.
        * @return void
        */
        void FetchDrmDataL( RFile& aFileHandle, 
              CSvgtFileDetails* aFileDetails ) const;
         

        /**
        * Constructs DRM items. 
        * @since 3.0
        * @param aItemArray Array where constructed item is put.
        * @param aFileDetails File details.
        * @return void
        */
        void MakeDrmItemsL( CDesCArray* aItemArray, 
         const CSvgtFileDetails* aFileDetails ) const;

        /**
        * Constructs restricted drm items. E.g. count, interval 
        * @since 3.0
        * @param aRights Drm right type "Play"
        * @param aItemArray Array where constructed item is put.
        * @param aFileDetails File details.
        * @return void
        */
        void MakeRestrictedDrmItemsL( const HBufC* aRights, 
                CDesCArray* aItemArray,
                const CSvgtFileDetails* aFileDetails ) const;
        
        /**
        * Add a single element of time to the string array
        * @param aNumOfElements         Number of elements of the interval
        * @param aResourceIdSingle      Resource id to be used in case of 
        *                               single unit
        * @param aResourceIdOneFinal    Resource id to be used in case of 
        *                               units ending with 1, from 21
        *                               (requested by Slavic languages)
        * @param aResourceIdTwoFour     Resource id to be used in case of 
        *                               units included from two to four, except
        *                               12-14 (requested by Slavic languages)
        * @param aResourceIdFiveZero    Resource id to be used in case of 
        *                               units ending from 5 to 9, plus range
        *                               from 11 to 14 (requested by Slavic 
        *                               languages)
        * @param aStrings               Buffer where to add the element
        */
        void AddSinglePartOfTimeL( TInt   aNumOfElements, 
                                   TInt   aResourceIdSingle, 
                                   TInt   aResourceIdOneFinal, 
                                   TInt   aResourceIdTwoFour,
                                   TInt   aResourceIdFiveZero, 
                                   HBufC*& aStrings ) const;
        
        void AddResourceFileToEnvL();
                                           
        /**
        * Shows file details to user in dialog. 
        * @since 3.0
        * @param aFileDetails File details which are shown to user.
        * @return void
        */
//        void ExecuteLD( const CSvgtFileDetails* aFileDetails );
        void DoLaunchDialogL(const CSvgtFileDetails* aFileDetails );
        
    private:    // Data
        CAknDoublePopupMenuStyleListBox* iListBox;
        CEikonEnv* iEnv;
        TInt iResOffset;
    };

#endif //SVGTFILEVIEWDETAILSDIALOG_H

// End of File
