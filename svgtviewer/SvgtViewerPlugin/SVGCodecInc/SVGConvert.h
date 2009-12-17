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
* Description:  ECom plugin to create the thumbnails from svg presentation.
*
*/



#ifndef SVGCONVERT_H
#define SVGCONVERT_H

#include <imageplugin.h>

class CSvgReadCodec;
class CFrameInfoStrings;

/**
* The PluginAPI for SVG content decoder. 
* Intended to use it for the processing the SVG content.
*/

class CSvgDecoder : public CImageDecoderPlugin
    {
public:

    /**
    * Static constructor. Creates and returns a pointer to CSvgDecoder
    * @since 3.1
    * @return Pointer to the deocder instance.
    */
    static CSvgDecoder* NewL();
    
    /**
    * Destructor.
    */
    ~CSvgDecoder();

public:    //From CImageDecoderPlugin
    /**
    * Returns the image type & image sub type of the frame number specified
    * @since 3.1
    * @param aFrameNumber
    * @param aImageType
    * @param aImageSubType
    * @return None.
    */
    void ImageType(TInt aFrameNumber, TUid& aImageType, TUid& aImageSubType) const; 
    
    /**
    * Returns the codec specific frame information stored in resource files.
    * This is a virtual funtion that each individual plugin must implement.
    * @param  aFs A handle to a file server session.
    * @param  aFrameNumber    The frame number to which the codec specific frame information
    *        applies (optional, defaults to zero).
    * @return A pointer to a newly allocated CFrameInfoStrings object.
    *        Ownership is transferred to the caller.
    */
    CFrameInfoStrings* FrameInfoStringsL(RFs& aFs, TInt aFrameNumber);

    TSize ContentSizeInTwips(const TSize aContentSizeInPixels) const;
    
private:
    //Default constructor
    CSvgDecoder();

private: //From CImageDecoderPlugin

	/**
	* Invokes the ReadFrameHeadersL() method of the supplied plugin.
    * The plugin's version of ReadFrameHeadersL() should read the image header \
    * information, create
	* the required codec and read the frame headers.
	* This is a virtual funtion that each individual plugin must implement.
	*/
    void ScanDataL();
    
    /**
    * Reads the image header & data information. It populates the image information \ 
    * like content size, pixel depth.
    * @since 3.1
    */
    void ReadFormatL();

private:    //Data members
    TPtrC8 iFileData;
    };


#endif // SVGCONVERT_H
// End of File

