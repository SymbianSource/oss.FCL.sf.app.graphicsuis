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
* Description:  Implementation of the CSvgtFileViewDetailsDialog.
*
*/

    

// INCLUDE FILES
#include "svgtfileviewdetails.h"

#include <aknlists.h>
#include <bautils.h>
#include <StringLoader.h>
#include <e32base.h>
#include <apmstd.h> // KMaxDataTypeLength
#include <DRMHelper.h> 
#include <caf/content.h>
#include <caf/attribute.h>
#include <data_caging_path_literals.hrh>
#include <tz.h>
#include <tzconverter.h>

#include <svgtfileviewdetails.rsg>

_LIT(KResourceFileName,"svgtfileviewdetails.rsc");

const TInt KSizeStringLengthSix = 6;
const TInt KSizeStringLengthFive = 5;
const TInt KSizeStringLengthFour = 4;
const TInt KSizeStringLengthThree = 3;

const TInt KValBufferLength = 512;
const TInt KTimeCount = 2;
const TInt KArrayGranularity = 2; 
const TInt KDecimalCount = 2;

const TInt KOneDigit1 = 1;
const TInt KOneDigit5 = 5;
const TInt KOneDigit9 = 9;
const TInt KTwoDigit11 = 11;
const TInt KTwoDigit14 = 14;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSvgtFileDetails::CSvgtFileDetails
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSvgtFileDetails::CSvgtFileDetails() 
    :   iUrl(NULL), iFormat(NULL), 
        iDRMExpireConstraint( ESVGTNone )   //Initialize with no constraints
    {
    }

// -----------------------------------------------------------------------------
// CSvgtFileDetails::~CSvgtFileDetails   
// Destructor
// -----------------------------------------------------------------------------
//
CSvgtFileDetails::~CSvgtFileDetails()
    {
    delete iUrl;    
    delete iFormat; 
    }


// CONSTANTS


_LIT(KDataTypeSvgt,"svg"); 

const TInt KFileSizeOneKilobyte = 1024;
const TInt KThousandNotKilobyte = 1000;
const TInt KMaxTimeLength = 36;

// For meta data 
_LIT( KSpace, " ");

// Language specific time format
const TInt KOneDigit = 10;
const TInt KTwoDigits = 100;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::CSvgtFileViewDetailsDialog
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSvgtFileViewDetailsDialog::CSvgtFileViewDetailsDialog() 
    {
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::ConstructL()
    {
    iListBox = new (ELeave) CAknDoublePopupMenuStyleListBox();
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CSvgtFileViewDetailsDialog* CSvgtFileViewDetailsDialog::NewL()
    {
    CSvgtFileViewDetailsDialog* self = new( ELeave ) CSvgtFileViewDetailsDialog;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::~CSvgtFileViewDetailsDialog   
// Destructor
// -----------------------------------------------------------------------------
//
CSvgtFileViewDetailsDialog::~CSvgtFileViewDetailsDialog()
    {
    delete iListBox;
    iEnv->DeleteResourceFile( iResOffset );  //removing the resource file for enviornment
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::ExecuteLD
// -----------------------------------------------------------------------------
//
EXPORT_C void CSvgtFileViewDetailsDialog::ExecuteLD( RFile& aFileHandle )     
    {
    CleanupStack::PushL( this );
    CSvgtFileDetails* fileDetails = new (ELeave) CSvgtFileDetails();
    CleanupStack::PushL( fileDetails );

    AddResourceFileToEnvL();

   // Creates a handle to content handled via CAF
    CContent *content = CContent::NewLC( aFileHandle ); 
    content->GetAttribute( EIsProtected, fileDetails->iDrmProtected );
    CleanupStack::PopAndDestroy( content );
  
    if ( fileDetails->iDrmProtected )
        {
        // drm data
        FetchDrmDataL( aFileHandle, fileDetails );        
        }
    
    // recognize file and get format
    TBuf<KMaxDataTypeLength> format(KDataTypeSvgt);
    fileDetails->iFormat = format.AllocL();
    
    // creation time
    aFileHandle.Modified(fileDetails->iTime);
   
    // Convert from UTC to local time
    RTz lTz;
    // Connect to TimeZone Server
    User::LeaveIfError( lTz.Connect() );
    CleanupClosePushL( lTz );
    
    // Create Util Class
    CTzConverter* lTzConverterPtr = CTzConverter::NewL( lTz );
    
    CleanupStack::PushL( lTzConverterPtr );
    // Perform conversion to local time
    User::LeaveIfError( lTzConverterPtr->ConvertToLocalTime( fileDetails->iTime ) );

    CleanupStack::PopAndDestroy( 2 ); // lTzConverterPtr, lTz

    // file size
    aFileHandle.Size(fileDetails->iSize);
    
    // Show File Details Dialog
    DoLaunchDialogL( fileDetails );

    CleanupStack::PopAndDestroy( fileDetails ); 
    CleanupStack::PopAndDestroy( this ); 
    
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::DoLaunchDialog
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::DoLaunchDialogL(const CSvgtFileDetails* aFileDetails )
    {
    CAknPopupList* popupList = CAknPopupList::NewL(iListBox,
            R_AVKON_SOFTKEYS_OK_EMPTY__OK, 
            AknPopupLayouts::EMenuDoubleWindow);
    CleanupStack::PushL(popupList);
    
    iListBox->ConstructL(popupList, EAknListBoxViewerFlags);
    iListBox->CreateScrollBarFrameL(ETrue);   
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
                    CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
    
    SetTitleL(popupList);
    FillListBoxL(aFileDetails);

    CleanupStack::Pop( popupList );
    popupList->ExecuteLD();
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::SetTitleL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::SetTitleL(CAknPopupList* aPopupList) const
    {
    HBufC* title = StringLoader::LoadLC(R_FILE_DETAILS);
    if( aPopupList )
       {
       aPopupList->SetTitleL(*title);   
       }    
    CleanupStack::PopAndDestroy( title ); 
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::FillListBoxL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::FillListBoxL(const CSvgtFileDetails* aFileDetails)
    {
    CDesCArray* itemArray = 
            static_cast<CDesCArray*>( iListBox->Model()->ItemTextArray() );
    
    if( aFileDetails )
        {
        if ( aFileDetails->iDrmProtected )
            {
            MakeDrmItemsL(itemArray,aFileDetails);
            }    
           
        MakeUrlItemL(itemArray,aFileDetails);
        MakeFormatItemL(itemArray,aFileDetails);
        MakeDateItemL(itemArray,aFileDetails);
        MakeTimeItemL(itemArray,aFileDetails);
        MakeSizeItemL(itemArray,aFileDetails);
        }
        
    iListBox->HandleItemAdditionL();
    iListBox->SetCurrentItemIndex(0);
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::AddItemToListBoxL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::AddItemToListBoxL(const TDesC& aHeading,
                                             const TDesC& aValue, 
                                             CDesCArray* aItemArray) const
    {
    CDesCArrayFlat* items = new (ELeave) CDesCArrayFlat( KArrayGranularity );
    CleanupStack::PushL(items);
    items->AppendL(aHeading); //First string (heading)
    items->AppendL(aValue);   //Second string (value)
    HBufC* headingAndValue = 
            StringLoader::LoadLC( R_FILE_DETAILS_ROW_FORMAT, *items );
    if( aItemArray )        
        {
        aItemArray->AppendL(*headingAndValue);  
        }    

        CleanupStack::PopAndDestroy( headingAndValue ); // headingAndValue 
        CleanupStack::PopAndDestroy( items ); // items 
    }


// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::MakeUrlItemL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::MakeUrlItemL(CDesCArray* aItemArray,
                                  const CSvgtFileDetails* aFileDetails) const
    {
    
    if ( aFileDetails && aFileDetails->iUrl )
        {
        HBufC* heading = StringLoader::LoadLC(R_FILE_URL_HEADING);
        AddItemToListBoxL(*heading,*aFileDetails->iUrl,aItemArray);
        CleanupStack::PopAndDestroy( heading ); 
        }
    }


// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::MakeFormatItemL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::MakeFormatItemL(CDesCArray* aItemArray,
                                     const CSvgtFileDetails* aFileDetails) const
    {
    if ( aFileDetails && aFileDetails->iFormat )
        {
        HBufC* heading = StringLoader::LoadLC(R_FILE_FORMAT_HEADING);
        AddItemToListBoxL(*heading,*aFileDetails->iFormat,aItemArray);
        CleanupStack::PopAndDestroy( heading );
        }
    }


// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::MakeSizeItemL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::MakeSizeItemL(CDesCArray* aItemArray,
                                   const  CSvgtFileDetails* aFileDetails) const
    {

    if ( aFileDetails && aFileDetails->iSize )
        {   
        TRealFormat format = TRealFormat(KMaxFileName, KDecimalCount);
        TBuf<KMaxFileName> sizeString;
        HBufC* value = NULL;
        HBufC* heading = StringLoader::LoadLC(R_FILE_SIZE_HEADING);
        TReal size = aFileDetails->iSize;
        
        // Show bytes with 3 digits. If more digits needed, 
        // show size in kilobytes.
        if ( size < KThousandNotKilobyte )
            {
            sizeString.Num(size,format);
            switch (sizeString.Length())
                {
                case KSizeStringLengthSix:
                    sizeString = sizeString.Left( KSizeStringLengthThree );
                    break;
                case KSizeStringLengthFive:
                    sizeString = sizeString.Left( KSizeStringLengthFour );
                    break;
                default:
                    break;
                }
            value = StringLoader::LoadLC(R_FILE_SIZE_B,sizeString);
            }
        else
            {
            size = size/KFileSizeOneKilobyte;
            sizeString.Num(size,format);
            switch (sizeString.Length())
                {
                case KSizeStringLengthSix:
                    sizeString = sizeString.Left( KSizeStringLengthThree );
                    break;
                case KSizeStringLengthFive:
                    sizeString = sizeString.Left( KSizeStringLengthFour );
                    break;
                default:
                    break;
                }
            value = StringLoader::LoadLC(R_FILE_SIZE_KB,sizeString);
            }
        TPtr textPtr = value->Des();
        LanguageSpecificNumberConversion( textPtr );
        AddItemToListBoxL(*heading,*value,aItemArray);
        CleanupStack::PopAndDestroy( value ); 
        CleanupStack::PopAndDestroy( heading ); // heading & value
        }
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::MakeDateItemL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::MakeDateItemL(CDesCArray* aItemArray,
                                    const CSvgtFileDetails* aFileDetails) const
    {

    if ( ( aFileDetails ) && ( aFileDetails->iTime > TTime(0) ))
        {
        HBufC* heading = StringLoader::LoadLC(R_FILE_DATE_HEADING);
        HBufC* dateFormatString = 
                StringLoader::LoadLC( R_QTN_DATE_USUAL_WITH_ZERO );
        TBuf<KMaxTimeLength> date;
        aFileDetails->iTime.FormatL(date,*dateFormatString);
        LanguageSpecificNumberConversion( date );
        AddItemToListBoxL(*heading,date,aItemArray);
        CleanupStack::PopAndDestroy( dateFormatString ); 
        CleanupStack::PopAndDestroy( heading ); // heading & dateFormatString
        }

    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::MakeTimeItemL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::MakeTimeItemL(CDesCArray* aItemArray,
                                  const CSvgtFileDetails* aFileDetails) const
    {
    if ( ( aFileDetails ) && ( aFileDetails->iTime > TTime(0)))
        {
        HBufC* heading = StringLoader::LoadLC( R_FILE_TIME_HEADING );
        HBufC* timeFormatString = 
                StringLoader::LoadLC( R_QTN_TIME_USUAL_WITH_ZERO );
        TBuf<KMaxTimeLength> time;
        aFileDetails->iTime.FormatL(time,*timeFormatString);
        LanguageSpecificNumberConversion( time );
        AddItemToListBoxL(*heading,time,aItemArray);
        CleanupStack::PopAndDestroy( timeFormatString ); 
        CleanupStack::PopAndDestroy( heading ); // heading & timeFormatString
        }
    }


// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::MakeDrmItemsL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::MakeDrmItemsL(CDesCArray* aItemArray,
                                       const CSvgtFileDetails* aFileDetails) const
    {

    HBufC* rights = StringLoader::LoadLC(R_FILE_DRM_DISPLAY);
    HBufC* heading = NULL;
    HBufC* value = NULL;
    
    if ( aFileDetails )
        {
        switch (aFileDetails->iDRMRightsStatus)
            {
            case ESVGTFull:
                // status
                heading = StringLoader::LoadLC(R_FILE_DRM_STAT);
                value = StringLoader::LoadLC(R_FILE_DRM_VALID);
                AddItemToListBoxL(*heading,*value,aItemArray);
                CleanupStack::PopAndDestroy( value ); 
                CleanupStack::PopAndDestroy( heading ); // heading && value
                // full rights
                heading = StringLoader::LoadLC(R_FILE_DRM_FULL,*rights);
                value = StringLoader::LoadLC(R_FILE_DRM_UNLIMITED);
                AddItemToListBoxL(*heading,*value,aItemArray);
                CleanupStack::PopAndDestroy( value ); 
                CleanupStack::PopAndDestroy( heading ); // heading && value
                break;
            case ESVGTRestricted:
            case ESVGTPreview:
                // status
                heading = StringLoader::LoadLC(R_FILE_DRM_STAT);
                value = StringLoader::LoadLC(R_FILE_DRM_VALID);
                AddItemToListBoxL(*heading,*value,aItemArray);
                CleanupStack::PopAndDestroy( value ); 
                CleanupStack::PopAndDestroy( heading ); // heading && value
                // make rest of the drm items
                MakeRestrictedDrmItemsL(rights,aItemArray,aFileDetails);          
                break;
            case ESVGTMissing:
                // status
                heading = StringLoader::LoadLC(R_FILE_DRM_STAT);
                value = StringLoader::LoadLC(R_FILE_DRM_EXP);
                AddItemToListBoxL(*heading,*value,aItemArray);
                CleanupStack::PopAndDestroy( value ); 
                CleanupStack::PopAndDestroy( heading ); // heading && value
                break;
            case ESVGTExpired:
                // status
                heading = StringLoader::LoadLC(R_FILE_DRM_STAT);
                value = StringLoader::LoadLC(R_FILE_DRM_EXP);
                AddItemToListBoxL(*heading,*value,aItemArray);
                CleanupStack::PopAndDestroy( value ); 
                CleanupStack::PopAndDestroy( heading ); // heading && value
                // make rest of the drm items
                MakeRestrictedDrmItemsL(rights,aItemArray,aFileDetails);          
                break;
            default:
                break;
            }
        }
    
    heading = NULL;
    value = NULL;
    
    // sending
    heading = StringLoader::LoadLC(R_FILE_DRM_CS);
    if ( aFileDetails && aFileDetails->iDRMForwardLocked )
        {
        value = StringLoader::LoadLC(R_FILE_DRM_FORBID);
        }
    else
        {
        value = StringLoader::LoadLC(R_FILE_DRM_ALLOWED);
        }
    AddItemToListBoxL(*heading,*value,aItemArray); 
    CleanupStack::PopAndDestroy( value ); 
    CleanupStack::PopAndDestroy( heading ); 
    CleanupStack::PopAndDestroy( rights ); // heading, value, rights
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::MakeDrmItemsL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::MakeRestrictedDrmItemsL(
        const HBufC* aRights, CDesCArray* aItemArray, 
        const CSvgtFileDetails* aFileDetails ) const
    {

    HBufC* heading = NULL;
    HBufC* value = NULL;
    
    if ( !aRights || !aItemArray )
        {
        return;
        }
        
    // count
    if ( ( aFileDetails ) && ( aFileDetails->iDRMExpireConstraint == ESVGTCount 
    || aFileDetails->iDRMExpireConstraint == ESVGTCountAndTime ))
        {
        heading = StringLoader::LoadLC(R_FILE_DRM_UTL,*aRights);
        
        TInt counts = aFileDetails->iDRMCountsLeft;
        
        if (counts == 1)
            {
            value = StringLoader::LoadLC(R_FILE_DRM_1_COUNT);
            }
        else
            {
            value = StringLoader::LoadLC(R_FILE_DRM_N_COUNTS,counts);
            }
        TPtr textPtr = value->Des();
        LanguageSpecificNumberConversion( textPtr );
        AddItemToListBoxL(*heading,*value,aItemArray);
        CleanupStack::PopAndDestroy( value ); 
        CleanupStack::PopAndDestroy( heading ); // heading && value
        }

    if ( ( aFileDetails ) && ( aFileDetails->iDRMExpireConstraint == ESVGTTime ||
        aFileDetails->iDRMExpireConstraint == ESVGTCountAndTime ||
        ( aFileDetails->iDRMFileHasInterval && 
        aFileDetails->iDRMIntervalActive )))
        {
        HBufC* dateFormatString = StringLoader::LoadLC( 
                R_QTN_DATE_USUAL_WITH_ZERO );
        HBufC* timeFormatString = StringLoader::LoadLC( R_QTN_TIME_USUAL );
        
        TTime from = aFileDetails->iDRMValidFrom;
        TTime until = aFileDetails->iDRMValidUntil;

        // from
        heading = StringLoader::LoadLC(R_FILE_DRM_VALID_FROM,*aRights);
        HBufC* datePointer = HBufC::NewLC( KMaxLongDateFormatSpec );
        TPtr date = datePointer->Des();
        from.FormatL( date, *dateFormatString );
        TBuf<KMaxTimeLength> time;
        from.FormatL( time, *timeFormatString );
        date.Append( KSpace );
        date.Append( time );
        LanguageSpecificNumberConversion( date );
        AddItemToListBoxL( *heading, date, aItemArray );
        CleanupStack::PopAndDestroy( datePointer ); 
        CleanupStack::PopAndDestroy( heading ); // datePointer, heading 

        // until
        heading = StringLoader::LoadLC(R_FILE_DRM_VALID_TO,*aRights);
        HBufC* dateToPntr = HBufC::NewLC( KMaxLongDateFormatSpec );
        TPtr dateTo = dateToPntr->Des();
        until.FormatL( dateTo, *dateFormatString );
        TBuf<KMaxTimeLength> timeTo;
        until.FormatL( timeTo, *timeFormatString );
        dateTo.Append( KSpace );
        dateTo.Append( timeTo );
        LanguageSpecificNumberConversion( dateTo );
        AddItemToListBoxL( *heading, dateTo, aItemArray );

        // destroying dateFormatString, timeFormatString, heading, dateToPntr 
        CleanupStack::PopAndDestroy( dateToPntr ); 
        CleanupStack::PopAndDestroy( heading ); 
        CleanupStack::PopAndDestroy( timeFormatString ); 
        CleanupStack::PopAndDestroy( dateFormatString ); 
        }
    // interval
    else 
        {
        if ( ( aFileDetails ) && ( aFileDetails->iDRMFileHasInterval && 
             !aFileDetails->iDRMIntervalActive ))
            {
            // times not activated
            heading = StringLoader::LoadLC(R_FILE_DRM_UTS,*aRights);
            value  = StringLoader::LoadLC(R_FILE_DRM_NOT_ACT);
            AddItemToListBoxL(*heading,*value,aItemArray);
            CleanupStack::PopAndDestroy( value ); // heading && value
            CleanupStack::PopAndDestroy( heading );

            // query what the current time is
            TTime currentTime;
            currentTime.HomeTime();
            // calculate rights end time
            TTime endTime = currentTime + aFileDetails->iDRMInterval;

            HBufC* lVal = HBufC::NewLC( KValBufferLength );
            TPtr valPtr(lVal->Des());
            
            TInt count = 0;

            // years
            TTimeIntervalYears years = endTime.YearsFrom(currentTime);
            if (years.Int())
                {
                AddSinglePartOfTimeL( years.Int(), 
                                  R_FILE_DRM_NBR_OF_YEARS_ONE,
                                  R_FILE_DRM_NBR_OF_YEARS_ONE_FINAL,
                                  R_FILE_DRM_NBR_OF_YEARS_TWO_FOUR,
                                  R_FILE_DRM_NBR_OF_YEARS_FIVE_ZERO,
                                  value );
                count++;
                valPtr.Append(*value);
                endTime -= years; 
                CleanupStack::PopAndDestroy( value ); 
                }
            
            // months
            TTimeIntervalMonths months = endTime.MonthsFrom(currentTime);
            if (months.Int())
                {
                AddSinglePartOfTimeL( months.Int(), 
                                  R_FILE_DRM_NBR_OF_MONTHS_ONE,
                                  0,                                  
                                  R_FILE_DRM_NBR_OF_MONTHS_TWO_FOUR, 
                                  R_FILE_DRM_NBR_OF_MONTHS_FIVE_ZERO, 
                                  value );
                 if (count)
                    {
                    valPtr.Append( KSpace );
                    }

                count++;
                valPtr.Append(*value);
                endTime -= months;
                CleanupStack::PopAndDestroy( value );
                }
       
            // days
            TTimeIntervalDays days = endTime.DaysFrom(currentTime);
            if (days.Int() && count < KTimeCount)
                {
                AddSinglePartOfTimeL( days.Int(), 
                                  R_FILE_DRM_NBR_OF_DAYS_ONE,
                                  R_FILE_DRM_NBR_OF_DAYS_ONE_FINAL,
                                  R_FILE_DRM_NBR_OF_DAYS_TWO_FOUR, 
                                  R_FILE_DRM_NBR_OF_DAYS_FIVE_ZERO, 
                                  value );
                if (count)
                    {
                    valPtr.Append( KSpace );
                    }

                valPtr.Append(*value);
                endTime -= days; 
                CleanupStack::PopAndDestroy( value ); 
                count++;
                }
       
            // hours
            TTimeIntervalHours hours;
            TInt ret = endTime.HoursFrom(currentTime,hours);
            if (ret == KErrNone && count < KTimeCount && hours.Int())
                {
                AddSinglePartOfTimeL( hours.Int(), 
                                  R_FILE_DRM_NBR_OF_HOURS_ONE, 
                                  R_FILE_DRM_NBR_OF_HOURS_ONE_FINAL,
                                  R_FILE_DRM_NBR_OF_HOURS_TWO_FOUR, 
                                  R_FILE_DRM_NBR_OF_HOURS_FIVE_ZERO, 
                                  value );
                if (count)
                    {
                    valPtr.Append( KSpace );
                    }

                valPtr.Append(*value);
                endTime -= hours; 
                CleanupStack::PopAndDestroy( value ); 
                count++;
                }

            // mins
            TTimeIntervalMinutes minutes;
            ret = endTime.MinutesFrom(currentTime,minutes);
            if (ret == KErrNone && count < KTimeCount && minutes.Int())
                {
                AddSinglePartOfTimeL( minutes.Int(), 
                                  R_FILE_DRM_NBR_OF_MINS_ONE,
                                  R_FILE_DRM_NBR_OF_MINS_ONE_FINAL,
                                  R_FILE_DRM_NBR_OF_MINS_TWO_FOUR, 
                                  R_FILE_DRM_NBR_OF_MINS_FIVE_ZERO, 
                                  value );
                if (count)
                    {
                    valPtr.Append( KSpace );
                    }

                valPtr.Append(*value);
                endTime -= minutes; 
                CleanupStack::PopAndDestroy( value );
                count++;
                }
            
            // seconds
            TTimeIntervalSeconds seconds;
            ret = endTime.SecondsFrom(currentTime,seconds);
            if (ret == KErrNone && count < KTimeCount && seconds.Int())
                {
                AddSinglePartOfTimeL( seconds.Int(), 
                                  R_FILE_DRM_NBR_OF_SECS_ONE, 
                                  R_FILE_DRM_NBR_OF_SECS_ONE_FINAL,
                                  R_FILE_DRM_NBR_OF_SECS_TWO_FOUR, 
                                  R_FILE_DRM_NBR_OF_SECS_FIVE_ZERO, 
                                  value );

                if (count)
                    {
                    valPtr.Append( KSpace );
                    }

                valPtr.Append(*value);
                CleanupStack::PopAndDestroy( value ); 
                count++;
                }
           
            heading = StringLoader::LoadLC(R_FILE_DRM_UDL,*aRights);
            LanguageSpecificNumberConversion(valPtr);
            AddItemToListBoxL(*heading,valPtr,aItemArray);
            CleanupStack::PopAndDestroy( lVal ); 
            CleanupStack::PopAndDestroy( heading ); 
            }
            
        }
      
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::LanguageSpecificNumberConversion
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::LanguageSpecificNumberConversion( TDes& aText ) const
    {
    if ( AknTextUtils::DigitModeQuery( AknTextUtils::EDigitModeShownToUser ) )
        {
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion( aText );
        }
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::FetchDrmDataL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::FetchDrmDataL( RFile& aFileHandle, 
                                          CSvgtFileDetails* aFileDetails ) const
    {
   
    CSVGTDrmHelper* myHelper = CSVGTDrmHelper::NewLC();
    
    myHelper->LoadUsageRigthsInfoL(aFileHandle);

    if ( aFileDetails )
        {
        myHelper->GetUsageRightsStatus(aFileDetails->iDRMRightsStatus);
      
        // sending
        aFileDetails->iDRMForwardLocked = !myHelper->SendingAllowed();
            
        if (aFileDetails->iDRMRightsStatus == ESVGTRestricted ||
            aFileDetails->iDRMRightsStatus == ESVGTPreview ||
            aFileDetails->iDRMRightsStatus == ESVGTExpired)
            {    
            // counts
            TInt retCount = myHelper->GetCount(aFileDetails->iDRMCountsLeft);

            if (retCount == KErrNone)
                {
                aFileDetails->iDRMExpireConstraint = ESVGTCount;
                }
            
            // time
            TInt retTime = myHelper->GetStartTime(aFileDetails->iDRMValidFrom); 
            retTime = myHelper->GetEndTime(aFileDetails->iDRMValidUntil); 

            if (retTime == KErrNone && retCount == KErrNone)
                {
                aFileDetails->iDRMExpireConstraint = ESVGTCountAndTime;
                }
            else 
                {
                if (retTime == KErrNone)
                    {
                    aFileDetails->iDRMExpireConstraint = ESVGTTime;
                    }
                }
      
             // interval    
            if (myHelper->GetInterval(aFileDetails->iDRMInterval) == KErrNone)
                {
                aFileDetails->iDRMFileHasInterval = ETrue;
                TTime from;
                if (myHelper->GetIntervalStart(from) == KErrNone)
                    {
                    aFileDetails->iDRMIntervalActive = ETrue;
                    // active from
                    aFileDetails->iDRMValidFrom = from;
                    // calculate rights end time
                    aFileDetails->iDRMValidUntil = 
                        aFileDetails->iDRMValidFrom + aFileDetails->iDRMInterval;
                    }
                else
                    {
                    aFileDetails->iDRMIntervalActive = EFalse;
                    }
                }
            else
                {
                aFileDetails->iDRMFileHasInterval = EFalse;
                aFileDetails->iDRMIntervalActive = EFalse;
                }
            }        
        }
    
    // cleanup
    CleanupStack::PopAndDestroy( myHelper );
    }

// -----------------------------------------------------------------------------
// CSvgtFileViewDetailsDialog::AddSinglePartOfTimeL
// -----------------------------------------------------------------------------
//
void CSvgtFileViewDetailsDialog::AddSinglePartOfTimeL( TInt aNumOfElements,
                                                 TInt aResourceIdSingle,
                                                 TInt aResourceIdOneFinal, 
                                                 TInt aResourceIdTwoFour,
                                                 TInt aResourceIdFiveZero, 
                                                 HBufC*& aStrings ) const
    {
    TInt finalOneDigit = aNumOfElements % KOneDigit;
    TInt finalTwoDigits = aNumOfElements % KTwoDigits;

    if ( aNumOfElements == 1 )
        {
        aStrings = StringLoader::LoadLC( aResourceIdSingle);
        }
    else if ( finalOneDigit == KOneDigit1 && finalTwoDigits != KTwoDigit11 )
        {
        // Used for period of years ending with 1 from 21 (21, 31, 41, etc.)
        aStrings = StringLoader::LoadLC( aResourceIdOneFinal, aNumOfElements );
        }
    else if ( finalOneDigit == 0 || 
            ( finalOneDigit >= KOneDigit5 && finalOneDigit <= KOneDigit9 ) ||
            ( finalTwoDigits >= KTwoDigit11 && finalTwoDigits <= KTwoDigit14 ) )
        {
        // Used for period of minutes ending from 5 to 0 plus range between 11 
        // and 14 (5-20, 25-30, 35-40, 45-50, 53-59)
        aStrings = StringLoader::LoadLC( aResourceIdFiveZero, aNumOfElements );
        }
    else
        {
        // Used for period of minutes ending from 2 to 4, excluded 12-14 (2-4,
        // 22-24, 32-34, 42-44, 52-54)        
        aStrings = StringLoader::LoadLC( aResourceIdTwoFour, aNumOfElements );
        }     
    }

void CSvgtFileViewDetailsDialog::AddResourceFileToEnvL() 
    {
    TFileName resFileName;
    Dll::FileName( resFileName ); // get dll path
    TDriveName drive( TParsePtrC( resFileName ).Drive( ) ); // solve drive
    resFileName.Zero();
    resFileName.Append( drive );
    resFileName.Append( KDC_RESOURCE_FILES_DIR );
    resFileName.Append( KResourceFileName );

    iEnv = CEikonEnv::Static();
    BaflUtils::NearestLanguageFile(iEnv->FsSession(), resFileName); //for localization
    iEnv->AddResourceFileL(resFileName);
    RResourceFile resFile;
    CleanupClosePushL(resFile);
    resFile.OpenL(iEnv->FsSession(), resFileName);
    resFile.ConfirmSignatureL();
    iResOffset = resFile.Offset();
    CleanupStack::PopAndDestroy(&resFile); //resFile
    }

    
//  End of File
