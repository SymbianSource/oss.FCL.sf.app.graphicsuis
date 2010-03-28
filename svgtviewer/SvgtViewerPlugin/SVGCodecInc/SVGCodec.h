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
* Description:  SVGT codec for reading the file header & data.
*
*/


#ifndef SVGCODEC_H
#define SVGCODEC_H

#include <imagecodec.h>
#include <imagedata.h>
#include <SVGRequestObserver.h>
#include <SVGListener.h>


class CSvgEngineInterfaceImpl;
class CFbsBitmap;
class TFrameInfo;
class TSVGFrameInfo;
class MSvgError;
class CGulIcon;

// SVG Read codec.
class CSvgReadCodec : public CImageProcessorReadCodec
                       ,public MSvgRequestObserver
                       , public MSvgLoadingListener

    {
public:
    enum TDownloadNeeded 
        {
        // The uri refers to a local image file
        ELocalImageFile,
        // The uri refers to a remote image file
        ERemoteImageFile,
        // The uri doesnt refer to an image file
        ENotAnImageFile
        };
    
    enum TImagesLocation
        {
        EPhoneMemory,
        EMemoryCard
        };
public:

    /**
    * Static constructor. Creates and returns a pointer to CSvgReadCodec
    * @since 3.1
    * @return Pointer to the read codec instance.
    */
    static CSvgReadCodec* NewL(const TSize& aSize);

    /**
    * Destructor.
    */
    virtual ~CSvgReadCodec();

    /**
    * Sets the SVG presentation data
    * @since 3.1
    * @param aFileData  SVG presentation file data
    */
    void SetFileDataL( const TDesC8& aFileData );
    
public: //From MSvgRequestObserver
    void UpdateScreen();

    TBool ScriptCall( const TDesC& aScript, CSvgElementImpl* aCallerElement );

    TInt FetchImage( const TDesC& aUri, RFs& aSession, RFile& aFileHandle ) ;
    TInt FetchFont( const TDesC& /* aUri */, RFs& /* aSession */, RFile& /* aFileHandle */ );

    void GetSmilFitValue( TDes& aSmilValue );

    void UpdatePresentation( const TInt32&  aNoOfAnimation );        

public: //User methods
    void PrepareEngine();
    TSize ContentSize() const;
    void GenerateThumbnailL(CFbsBitmap* aFrame, CFbsBitmap* aFrameMask);
    TDownloadNeeded IsDownloadNeededL( const TDesC& aUri ) const;
    TBool GetLocalFile( const TDesC& aUri, 
                                    TDes& aFileName ,
                                    const TDesC& aRefPath ) const;
    
    HBufC* GetImagesPath( CSvgReadCodec::TImagesLocation aImagesLocation) const;
    
protected:
    // From CImageProcessorReadCodec
    /**
    * Performs initial processing of image data and mask bitmaps.
	* @param	aFrameInfo
	        A reference to a TFrameInfo object.
	* @param	aFrameImageData
	        A reference to a CFrameImageData object.
	* @param	aDisableErrorDiffusion
	        A flag indicating whether error diffusion should be disabled.
	* @param	aDestination
	        The destination bitmap.
	* @param	aDestinationMask
	        The destination mask bitmap.
    */
    virtual void InitFrameL(TFrameInfo& aFrameInfo, CFrameImageData& aFrameImageData, TBool aDisableErrorDiffusion, CFbsBitmap& aDestination, CFbsBitmap* aDestinationMask);
    
	/**
	* Processes the frame data contained in aSrc.
 	* This is a pure virtual function that each derived class must implement.
	* @param  aSrc
	*       A reference to the buffer containing the frame data.
    *
	* @return	The current frame state after processing.
	*/
    
    virtual TFrameState ProcessFrameL(TBufPtr8& aSrc);

private:
    
    /**
    * Precesses the SVG presentation file data
    */
    void DoProcessL();
    
    /**
    * Default constructor
    * @param aSize specifies the thubnail size
    */
    CSvgReadCodec(const TSize& aSize);

    void ConstructL();
    void InitializeEngineL();
    /**
    * Deletes the structures associated with the engine
    * viz. The Frame List, Mask List, Delay Interval List, 
    * The DOM for the content and the Engine Implementation itself
    */
    void DeleteEngine();
// From MSvgLoadingListener    
public:
    /**
     * Query the listener if every element is to be reported or only
     * the children of <svg> tag.
     *
     * @since 1.0
     * @return : ETrue if every element is to be reported, otherwise
     *           only the children of <svg> are reported.
     */
    TBool ReportAllElements();

    /**
     * Query the listener if client will call 'AssignImageData' to decode images
     * separately from parsing.  This is used to avoid non-thread-safe calls in
     * creating bitmap and decoding images, which must be executed in the main thread.
     *
     * @since 1.0
     * @return : ETrue to use 'ImageDataReference' instead of 'FetchImage'.
     */
    TBool WillAssignImageData();

    /**
     * Callback when the href:xlink attribute is encountered for the <image>
     * element.  This method is used to notify clients of image data references
     * needed by <image> element.  This method is called only when 'WillAssignImageData'
     * returns ETrue.
     *
     * @since 1.0
     * @return : none.
     */
    void ImageDataReference( const TDesC& aUri );

    /**
     * Notified when the start of a svg document(<svg> tag) is encountered.
     *
     * @since 1.0
     * @return : For future use.  Value is ignored.
     */
    TBool DocumentStart() ;

    /**
     * Notified when the end of a svg document(</svg> tag) is encountered.
     *
     * @since 1.0
     * @return : For future use.  Value is ignored.
     */
    TBool DocumentEnd();

    /**
     * Notified when the start of a svg element is encountered.
     *
     * @since 1.0
     * @param : aTagName -- name of svg tag
     * @param : aAttributeList -- attribute list.
     * @return : For future use.  Value is ignored.
     */
    TBool ElementStart( const TDesC& aTagName,
                                MSvgAttributeList& aAttributeList);

    /**
     * Notified when the end of a svg element is encountered.
     * activated.
     *
     * @since 1.0
     * @param : aTagName -- name of svg tag
     * @return : For future use.  Value is ignored.
     */
    TBool ElementEnd( const TDesC& aTagName );

    /**
     * Notified when an external data is needed by the svg document,
     * such as a image-file or an embedded svg-file.
     *
     * @since 1.0
     * @param : aUri -- URI string of external data
     * @return : For future use.  Value is ignored.
     */
    TBool ExternalDataRequested( const TDesC& aUri );

    /**
     * Notified when an external data has been retrieved,
     * such as a image-file or an embedded svg-file.
     *
     * @since 1.0
     * @param : aUri -- URI string of external data
     * @return : For future use.  Value is ignored.
     */
    TBool ExternalDataReceived( const TDesC& aUri );

    /**
     * Notified when an external data request has failed.
     *
     * @since 1.0
     * @param : aUri -- URI string of external data
     * @return : For future use.  Value is ignored.
     */
    TBool ExternalDataRequestFailed( const TDesC& aUri );

    /**
     * Notified when an unsupported element is encountered.
     *
     * @since 1.0
     * @param : aTagName -- tag name of element
     * @param : aAttributeList -- attribute list.
     * @return : For future use.  Value is ignored.
     */
    TBool UnsupportedElement( const TDesC& aTagName,
                                      MSvgAttributeList& aAttributeList );

    void ImagesLoaded(TInt aError);
    void GetNewDataPosition(TInt &aPosition, TInt &aLength);
    
private:    // Data
    // Dummy Bitmap used by SVG Engine to render content.
    CFbsBitmap* iSVGTBitMapDummy;

    // SVG Engine Interface Implementation pointer
    CSvgEngineInterfaceImpl* iSvgModule;
    
    //Frame Array
    RPointerArray<CFbsBitmap> iFrameList;
    RPointerArray<CFbsBitmap> iMaskList;
    RArray<TUint> iDelayList;
    
    // SVG Thumbnail Handle
    TInt iThumbnailHandle;
      
    // SVGT content filename descriptor
    HBufC8* iFileData;
    TSize iContentSize;
    MSvgError* iPrepareDomResult;
    
    TBool iImagesPresent;
    
    TBool iImagesLoaded;

    TSize iOriginalSize;
    };

#endif  //SVGCODEC_H
