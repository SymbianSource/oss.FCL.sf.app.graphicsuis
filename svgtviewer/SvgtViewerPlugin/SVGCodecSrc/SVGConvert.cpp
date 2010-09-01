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
* Description:  This file implements the decoder for the SVG presentation
*
*/


#include "SVGConvert.h"
#include <coemain.h>
#include <imageconversion.h>
#include <icl_uids.hrh>
#include <102073d7_extra.rsg>
#include <SvgCodecImageConstants.hrh>
#include "SVGCodec.h"

_LIT(KSVGPanicCategory, "SVGConvertPlugin");
const TInt KInitialFrameWidth = 1;
const TInt KInitialFrameHeight = 1;

// Number of bits per pixel used in ReadFormatL
const TInt KSvgDecBitsPerPixel = 24;

enum TIclPanic
    {
    EFrameNumberOutOfRange = 0x1
    };
    
// Global panic function
GLDEF_C void Panic(TIclPanic aError)
    {
    User::Panic(KSVGPanicCategory, aError);
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSvgDecoder::NewL
// Static constructor. Returns the pointer to the CSvgDecoder
// -----------------------------------------------------------------------------
//
CSvgDecoder* CSvgDecoder::NewL()
    {
    return new (ELeave) CSvgDecoder;
    }

// -----------------------------------------------------------------------------
// CSvgDecoder::~CSvgDecoder
// C++ destructor.
// -----------------------------------------------------------------------------
//
CSvgDecoder::~CSvgDecoder()
    {
    CImageDecoderPlugin::Cleanup();
    }

// -----------------------------------------------------------------------------
// CSvgDecoder::ImageType 
// Returns the image type and sub-type for a given frame of the image that
// has just been opened.
// -----------------------------------------------------------------------------
//
void CSvgDecoder::ImageType(TInt aFrameNumber, TUid& aImageType, TUid& aImageSubType) const
    {
    __ASSERT_ALWAYS(aFrameNumber == 0, Panic( EFrameNumberOutOfRange ));
    aImageType = KImageTypeSVGUid;
    aImageSubType = KNullUid;
    }

// -----------------------------------------------------------------------------
// Returns the codec specific frame information stored in resource files.
// This is a virtual funtion that each individual plugin must implement.
// -----------------------------------------------------------------------------
//
CFrameInfoStrings* CSvgDecoder::FrameInfoStringsL( RFs& /*aFs*/, TInt /*aFrameNumber*/ )
    {
    return NULL;
    }

// -----------------------------------------------------------------------------
// CSvgDecoder::CSvgDecoder
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSvgDecoder::CSvgDecoder()
    {
    }

// -----------------------------------------------------------------------------
//    Invokes the ReadFrameHeadersL() method of the supplied plugin.
//    This is a virtual funtion that each individual plugin must implement.
// -----------------------------------------------------------------------------
//
void CSvgDecoder::ScanDataL()
    {
    CSvgReadCodec* imageReadCodec = CSvgReadCodec::NewL( 
        TSize(KInitialFrameWidth, KInitialFrameHeight) 
        );
    SetImageReadCodec( imageReadCodec );

    ReadFormatL();
    ReadFrameHeadersL();
    }
    
// -----------------------------------------------------------------------------
// Reads the image header & data information
// -----------------------------------------------------------------------------
//
void CSvgDecoder::ReadFormatL()
    {
    ReadDataL(0, iFileData, KMaxTInt);
    SetDataLength( iFileData.Length() );
    CSvgReadCodec* imageReadCodec = static_cast<CSvgReadCodec* >(ImageReadCodec());
    imageReadCodec->SetFileDataL( iFileData );

    imageReadCodec->PrepareEngine();
    const TSize contentSize = imageReadCodec->ContentSize();
    
    TFrameInfo imageInfo;
    imageInfo = ImageInfo();
    imageInfo.iFrameCoordsInPixels.SetRect(TPoint(0, 0), contentSize );
    imageInfo.iOverallSizeInPixels = contentSize;
    imageInfo.iBitsPerPixel = KSvgDecBitsPerPixel;
    imageInfo.iDelay = 0;
    imageInfo.iFlags = TFrameInfo::ECanDither | TFrameInfo::ETransparencyPossible;
    imageInfo.iFrameDisplayMode = EColor64K;
    imageInfo.iFrameSizeInTwips = ContentSizeInTwips(contentSize);
    
    SetImageInfo(imageInfo);
    SetStartPosition(0);
    }

TSize CSvgDecoder::ContentSizeInTwips(const TSize aContentSizeInPixels) const
    {
    // Create zoom factor object
    TRect boxInTwips(TPoint(0,0), TSize(0,0));
    CCoeEnv* coeEnv = CCoeEnv::Static();
    if(coeEnv)
        {
        TZoomFactor deviceMap(CCoeEnv::Static()->ScreenDevice());
        // Set zoom factor at 1 to 1
        deviceMap.SetZoomFactor(TZoomFactor::EZoomOneToOne);
        const TRect boxInPixels(TPoint(0,0), aContentSizeInPixels);
        // convert rectangle co-ordinates into pixels
        boxInTwips = deviceMap.PixelsToTwips(boxInPixels);
        }
    return boxInTwips.Size();
    }
    
// End of File

