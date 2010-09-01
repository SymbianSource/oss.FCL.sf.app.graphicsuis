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
* Description:  This file implements the SVGT Thumbnail functionality
*                to display SVGT content
*
*/



// INCLUDE FILES
#include <AknIconUtils.h>
#include <e32def.h>
#include <e32base.h>
#include <eikenv.h>
#include <fbs.h>
#include <gdi.h>
#include <bautils.h>
#include <SVGEngineInterfaceImpl.h>
#include <data_caging_path_literals.hrh>

// User Includes
#include "SVGTUIControlDbgFlags.hrh"
#include "SVGTThumbnailUtil.h"

// Constants
// General Constants
const TInt KSccConstMinusOne = -1;
const TInt KSccConstZero = 0;
const TUint KSccBackground = 0xffffffff;


// SMIL Fit Value
_LIT( KSccSmilFitValue,"meet" );

// ============================ MEMBER FUNCTIONS ===============================
// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::GenerateThumbnailLD
// Generate thumbnails for SVGT content.
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CSVGTThumbnailUtil::GenerateThumbnailLD(
    RFile& aFileHandle,                 // Filehandle of SVGT
    CFbsBitmap& aThumbnailBitmap,       // Thumbnail Bitmap
    CFbsBitmap& aThumbnailBitmapMask )  // BitMap mask    
    {
    CSVGTThumbnailUtil *tnUtilPtr = CSVGTThumbnailUtil::NewLC( aFileHandle, 
            aThumbnailBitmap,
            aThumbnailBitmapMask );
    TBool retVal = tnUtilPtr->GenerateThumbnail();
    CleanupStack::PopAndDestroy( tnUtilPtr );
    return retVal;
    }



// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::UpdateScreen
// Implements MSvgRequestObserver::UpdateScreen.
// -----------------------------------------------------------------------------
//
void CSVGTThumbnailUtil::UpdateScreen()
    {
    }

// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::ScriptCall
// Implements MSvgRequestObserver::ScriptCall.
// -----------------------------------------------------------------------------
//
TBool CSVGTThumbnailUtil::ScriptCall( const TDesC& /*aScript*/,
                                  CSvgElementImpl* /*aCallerElement*/ )
    {
    return EFalse;
    }


// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::FetchImage
// Implements MSvgRequestObserver::FetchImage. Calls the application call
// back corresponding to this function if exists, else returns EFalse
// indicating to the engine that the image was not retrieved.
// -----------------------------------------------------------------------------
//     
TInt  CSVGTThumbnailUtil::FetchImage( const TDesC& /*aUri*/, 
             RFs& /*aSession*/, RFile& /*aFileHandle*/ )
    {
    return KErrNotFound;        
    }

TInt CSVGTThumbnailUtil::FetchFont( const TDesC& /* aUri */, 
                                    RFs& /* aSession */, RFile& /* aFileHandle */ )
    {
    return KErrNotSupported;
    }
    
// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil:: GetSmilFitValue
// Implements the MSvgRequestObserver::GetSmilFitValue.
// -----------------------------------------------------------------------------
//
void CSVGTThumbnailUtil::GetSmilFitValue( TDes& aSmilValue )
    {
    aSmilValue.Copy( KSccSmilFitValue );
    }

// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::UpdatePresentation
// Implements MSvgRequestObserver::UpdatePresentation.
// -----------------------------------------------------------------------------
//
void  CSVGTThumbnailUtil::UpdatePresentation( const TInt32&  
    /* aNoOfAnimation */ )
    {
    }

// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::InitializeEngineL
// Initiliazes the interface with the SVG engine.
// -----------------------------------------------------------------------------
//
void CSVGTThumbnailUtil::InitializeEngineL()
    {
    if ( !iSvgModule )
       {
       TFontSpec spec;

       if ( !iSVGTBitMapDummy )
           {
           // For thumbnails, the user provided bitmap is used,
           // Have to give some dummy bitmap to the engine in the constructor.
           iSVGTBitMapDummy  = new( ELeave ) CFbsBitmap;
           User::LeaveIfError( iSVGTBitMapDummy->Create(
            TSize( KSccConstZero, KSccConstZero ), EGray2 ) );
           }

       iSvgModule = CSvgEngineInterfaceImpl::NewL( iSVGTBitMapDummy,
               this, spec );
       iSvgModule->SetBackgroundColor( KSccBackground );
       }
    }

// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::GenerateThumbnail
// Helper function that generates Thumbnail for the SVG content.
// -----------------------------------------------------------------------------
//
TBool CSVGTThumbnailUtil::GenerateThumbnail()
    {
    if ( iSvgModule )
        {
        MSvgError* result;
        result = iSvgModule->PrepareDom( iContentFileHandle, iThumbnailHandle );

        // NULL is returned, possible out of memory when create error object.
        if ( ( !result ) || ( result->HasError() ) )
            {
            return EFalse;
            }
#ifdef SVGTUICONTROL_DBG_DRM_THUMBNAIL_API_ENABLE        
        // Set thumbnail mode On
        iSvgModule->SetThumbNailMode( ETrue );                          
#endif // SVGTUICONTROL_DBG_DRM_THUMBNAIL_API_ENABLE

        // Setup the engine to use the DOM
        result = iSvgModule->UseDom( iThumbnailHandle, iThumbnailBitmap );
        if ( result->HasError() && !result->IsWarning() )
            {                
            return EFalse;
            }

        // View Box define for Dom associated frame
        iSvgModule->ChooseViewBoxIfNotSet(iThumbnailHandle);
        iSvgModule->InitializeEngine();
        iSvgModule->RenderFrame( NULL, 0);

        // Render the Mask Bitmap
        iSvgModule->GenerateMask( iThumbnailBitmapMask );
#ifdef SVGTUICONTROL_DBG_DRM_THUMBNAIL_API_ENABLE        
        // Set thumbnail mode Off
        iSvgModule->SetThumbNailMode( EFalse );            
#endif // SVGTUICONTROL_DBG_DRM_THUMBNAIL_API_ENABLE

        return ETrue;
        }
    else
        {
        return EFalse;
        }
        
    }

// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::CSVGTThumbnailUtil
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CSVGTThumbnailUtil::CSVGTThumbnailUtil( RFile& aFileHandle ) : 
    // Thumbnail Variables
    iThumbnailHandle( KSccConstMinusOne ),
    iContentFileHandle( aFileHandle )
{
}

// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CSVGTThumbnailUtil::ConstructL(
    CFbsBitmap& aThumbnailBitmap,     // Bitmap on which content is to be
                                      // rendered
    CFbsBitmap& aThumbnailBitmapMask )// Bitmap Mask
    {
    // Save the user parameters for later access.
    iSvgModule = NULL;
    iSVGTBitMapDummy = NULL;
    iThumbnailBitmap = &aThumbnailBitmap;
    iThumbnailBitmapMask = &aThumbnailBitmapMask;
    InitializeEngineL();
    }

// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::NewL
// Factory function for creating CSVGTThumbnailUtil objects for thumbnail
// generation.
// Returns: CSVGTThumbnailUtil* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTThumbnailUtil* CSVGTThumbnailUtil::NewL(
    RFile& aFileHandle,               // File handle of SVGT Content
    CFbsBitmap& aThumbnailBitmap,     // Bitmap on which content is to be
                                      // rendered
    CFbsBitmap& aThumbnailBitmapMask )// Bitmap Mask
    {
    CSVGTThumbnailUtil* self = CSVGTThumbnailUtil::NewLC( aFileHandle,
                                                      aThumbnailBitmap,
                                                      aThumbnailBitmapMask );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::NewLC
// Factory function for creating CSVGTThumbnailUtil objects for thumbnail
// generation.
// Returns: CSVGTThumbnailUtil* ; Pointer to the created object.
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTThumbnailUtil* CSVGTThumbnailUtil::NewLC(
    RFile& aFileHandle,               // File Handle of SVGT Content
    CFbsBitmap& aThumbnailBitmap,     // Bitmap on which content is to be
                                      // rendered
    CFbsBitmap& aThumbnailBitmapMask) // Bitmap Mask
    {
    CSVGTThumbnailUtil* self = new ( ELeave ) CSVGTThumbnailUtil( 
        aFileHandle );
    CleanupStack::PushL( self );
    self->ConstructL( aThumbnailBitmap, aThumbnailBitmapMask );
    return self;
    }


// -----------------------------------------------------------------------------
// CSVGTThumbnailUtil::~CSVGTThumbnailUtil
// Destructor
// -----------------------------------------------------------------------------
//
CSVGTThumbnailUtil::~CSVGTThumbnailUtil()
    {
    if ( iThumbnailHandle != KSccConstMinusOne )
        {
        if ( iSvgModule )
            {
            iSvgModule->DeleteDom( iThumbnailHandle );            
            }
        }
   
    // Dummy bitmap
    delete iSVGTBitMapDummy;
    
    // SVG Engine
    delete iSvgModule;
    
    // Reset the Thumbnail and Mask as this is owned by the caller
    iThumbnailBitmap = NULL;
    iThumbnailBitmapMask = NULL;
    }


// End of File
