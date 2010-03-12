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
* Description:  SVGT Thumbnail Utility Class implements the thumbnail
*                used to display SVGT content.
*
*/



#ifndef __SVGTTHUMBNAILUTIL_H__
#define __SVGTTHUMBNAILUTIL_H__

// INCLUDES

#include <SVGRequestObserver.h>


// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class   CSvgEngineInterfaceImpl;
class   CFbsBitmap;
// CLASS DECLARATION

/**
*  CSVGTThumbnailUtil is a utility class used to generate thumbnails.
*  It is used through dialog provided by CSVGTUIDialog.
*
*  @lib SVGTUIControl.lib
*  @since 3.0
*/

class CSVGTThumbnailUtil : public CBase,
                           public MSvgRequestObserver
    {
    public: // Constructors and destructor
        /**
        * Generate thumbnails for SVGT content.
        * @since 3.0
        * @param aFileHandle File Handle to the SVGT file.
        * @param aThumbnailBitmap Bitmap - Client provided bitmap to render 
        *   the SVGT contents opening screen.
        * @param aThumbnailBitmap Bitmap - Client provided bitmap mask.
        * @return Boolean ETrue indicates thumbnail generated successfully,
        *                 EFalse otherwise.
        */
        IMPORT_C static TBool GenerateThumbnailLD( RFile& aFileHandle,
                                           CFbsBitmap& aThumbnailBitmap,
                                           CFbsBitmap& aThumbnailBitmapMask );

    public: // New functions

    public: // Functions from base classes

  
        /**
        * This function is called to initialize the SVGT-Engine interface.
        * @since 3.0
        */
        void InitializeEngineL();

        /**
        * Generates Thumbnail of initial screen of SVG content.
        * @since 3.0     
        * @return Boolean ETrue indicates thumbnail generated successfully,
        *                 EFalse otherwise.
        */

        TBool GenerateThumbnail();
        
     
    protected:  // Functions from base classes
        
        /**
        * From MSvgRequestObserver Method called by SVG Engine
        *  after it has rendered a frame.
        * @since 3.0
        */
        void UpdateScreen();

        /**
        * From MSvgRequestObserver This method is for future extension,
        *  in which an external script engine could be used to evaluate
        *  a script description.
        * @since 3.0
        * @param aScript Descriptor containing script
        * @param aCallerElement SVG Element that calls the script by a
        *  an event of the "on" attribute.
        * @return TBool.
        */
        TBool ScriptCall( const TDesC& aScript,
                          CSvgElementImpl* aCallerElement );

        /**
        * From MSvgRequestObserver This method is called by the SVG engine
        *  to retrieve the absolute path of the image file.
        * @since 3.0
        * @param aUri Descriptor containing the relative path of the image.
        * @param aFilePath Descriptor which contains absolute path that is
        *  to be used by the Engine to retrieve the image.
        * @return TBool -
        *  ETrue indicates absolute path is present in aFilePath.
        *  EFalse otherwise.
        */  
        TInt FetchImage( const TDesC& aUri, 
                          RFs& aSession, RFile& aFileHandle ) ;

        TInt FetchFont( const TDesC& /* aUri */, 
                                    RFs& /* aSession */, RFile& /* aFileHandle */ );

        /**
        * From MSvgRequestObserver This method is called by the SVG engine
        *  to get the SMIL Fit attribute value.
        * @since 3.0
        * @param aSmilValue Descriptor containing the SMIL Fit attribute.
        */
        void GetSmilFitValue( TDes& aSmilValue );

        /**
        * From MSvgRequestObserver This method is called by the SVG engine
        *  to update the presentation status.
        * @since 3.0
        * @param aNoOfAnimation Integer. When called for the first time
        *  it indicates total number of animations.
        *  Subsequently it contains number of animations that were completed.
        */
        void UpdatePresentation( const TInt32&  aNoOfAnimation );        
    private:

        /**
        * C++ default constructor.
        */
        CSVGTThumbnailUtil( RFile& aFileHandle );

        /**
        * Second Phase constructor.
        * @param aThumbnailBitmap Client provided bitmap to render the SVGT
        *   contents opening screen.
        * @param aThumbnailBitmapMask Bitmap - Client provided bitmap for SVGT
        *   bitmap mask.
        */
        void ConstructL( CFbsBitmap& aThumbnailBitmap,
                         CFbsBitmap& aThumbnailBitmapMask );

        /**
        * Two-phased constructor for thumbnail creation.
        * @since 3.0
        * @param aFileHandle file handle to the SVGT file.
        * @param aThumbnailBitmap Bitmap - Client provided bitmap to render
        *   the SVGT contents opening screen.
        * @param aThumbnailBitmapMask Bitmap - Client provided bitmap for SVGT
        *   bitmap mask.
        * @return CSVGTThumbnailUtil*
        */

        static CSVGTThumbnailUtil* NewL( RFile& aFileHandle,
                                       CFbsBitmap& aThumbnailBitmap,
                                       CFbsBitmap& aThumbnailBitmapMask );

        /**
        * Two-phased constructor for thumbnail creation that pushes the
        * object on the cleanup stack.
        * @since 3.0
        * @param aFileHandle file handle to the SVGT file.
        * @param aThumbnailBitmap Client provided bitmap to render the SVGT
        *                         contents opening screen.
        * @param aThumbnailBitmapMask Bitmap - Client provided bitmap for SVGT
        *   bitmap mask.
        * @return CSVGTThumbnailUtil*
        */

        static CSVGTThumbnailUtil* NewLC( RFile& aFileHandle,
                                        CFbsBitmap& aThumbnailBitmap,
                                        CFbsBitmap& aThumbnailBitmapMask );
        /**
        * Destructor.
        */

        virtual ~CSVGTThumbnailUtil();

    private:    // Data
        // Dummy Bitmap used by SVG Engine to render content.
        CFbsBitmap*  iSVGTBitMapDummy;

        // SVG Engine Interface Implementation pointer
        CSvgEngineInterfaceImpl*     iSvgModule;

        // Thumbnail bitmap used by SVG Engine to render content.
        CFbsBitmap*  iThumbnailBitmap;

        // Thumbnail bitmap mask used by SVG Engine to render content mask.
        CFbsBitmap*  iThumbnailBitmapMask;

        // SVG Thumbnail Handle
        TInt iThumbnailHandle;
          
        // SVGT content filename descriptor
        RFile& iContentFileHandle;
    };

#endif  // __SVGTTHUMBNAILUTIL_H__

// End of File
