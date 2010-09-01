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
* Description:  This file implements the read codec for the SVG presentation
*
*/


#include "SVGCodec.h"

// INCLUDE FILES
#include <uri16.h>
#include <coeutils.h>
#include <SVGEngineInterfaceImpl.h>
#include <gdi.h>
#include <pathconfiguration.hrh>
// User Includes
#include <SVGTUIControlDbgFlags.hrh>
#include <imcvcodc.h>
#include <utf.h>

#ifdef RD_MULTIPLE_DRIVE    
#include <pathinfo.h>
#include <driveinfo.h>
#endif

// General Constants
const TInt KSccConstZero = 0;
const TUint KSccBackground = 0xffffffff;
// SMIL Fit Value
_LIT( KSccSmilFitValue,"meet" );

//
_LIT( KWww, "www" );
_LIT( KHttp, "http://");
_LIT( KDotDot, ".." );
_LIT( KBSlashStr, "/" );
_LIT( KSlashStr, "\\" );
_LIT( KDotSlashStr, ".\\" );
_LIT( KColonStr, ":" );

_LIT(KJpg,   ".jpg" );
_LIT(KJpeg,  ".jpeg" );
_LIT(KPng,   ".png" );
_LIT(KBmp,   ".bmp" );
_LIT(KSvg,   ".svg" );
_LIT(KSvgz,  ".svgz" );
//

enum TSVGCodecPanic
    {
    ECouldNotGenerateThumbnail
    };

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSvgReadCodec::CSvgReadCodec
// Default constructor. 
// -----------------------------------------------------------------------------
//
CSvgReadCodec::CSvgReadCodec( const TSize& aSize ): iImagesPresent(EFalse),
                                                    iImagesLoaded (EFalse),
                                                    iOriginalSize(aSize)

    {
    }

// -----------------------------------------------------------------------------
// CSvgReadCodec::NewL
// Static constructor. Returns the pointer to the CSvgReadCodec
// -----------------------------------------------------------------------------
//
CSvgReadCodec* CSvgReadCodec::NewL( const TSize& aSize )
    {
    CSvgReadCodec* self = new(ELeave) CSvgReadCodec( aSize );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self); 
    return self;
    }
    
void CSvgReadCodec::ConstructL()
    {
    iSvgModule = NULL;
    iSVGTBitMapDummy = NULL;
    InitializeEngineL();
    }

void CSvgReadCodec::InitializeEngineL()
    {
    if ( !iSvgModule )
       {
       TFontSpec spec;

       if ( !iSVGTBitMapDummy )
           {
           // For thumbnails, the user provided bitmap is used,
           // Have to give some dummy bitmap to the engine in the constructor.
           iSVGTBitMapDummy  = new( ELeave ) CFbsBitmap;
           User::LeaveIfError( 
                iSVGTBitMapDummy->Create( 
                    TSize( KSccConstZero, KSccConstZero ), 
                    EGray2 ) );
           }

       iSvgModule = CSvgEngineInterfaceImpl::NewL( iSVGTBitMapDummy, this, spec );
// 0xffffffff is the default color.
       iSvgModule->SetBackgroundColor( KSccBackground );
       iSvgModule->AddListener(static_cast<MSvgLoadingListener*>(this),
            ESvgLoadingListener);

       }
    }

// -----------------------------------------------------------------------------
// CSvgReadCodec::~CSvgReadCodec
// Destructor
// -----------------------------------------------------------------------------
//
CSvgReadCodec::~CSvgReadCodec()
    {
    DeleteEngine();

    // Dummy bitmap
    delete iSVGTBitMapDummy;

    // SVG file data
    delete iFileData;
    }

// -----------------------------------------------------------------------------
// CSvgReadCodec::SetFileDataL
// Sets the SVG presentation data
// -----------------------------------------------------------------------------
//
void CSvgReadCodec::SetFileDataL( const TDesC8& aFileData )
    {
    delete iFileData;
    iFileData = NULL;
    
   	// For files which are in UTF-16 or unicode format the first 2 bytes
	// will be FF and FE. In UTF-16 and unicode format every single character
	// is represented by two bytes. 
	
    HBufC8* binaryBuffer=NULL;
  	binaryBuffer = HBufC8::NewL( aFileData.Length() ); 
  	
  	TPtr8 binaryBufferPtr = binaryBuffer->Des();
	binaryBufferPtr = aFileData;
	
	TInt hichar = (CEditableText::EReversedByteOrderMark & 0xFF00)>>8;
	TInt lochar = CEditableText::EReversedByteOrderMark & 0x00FF;
	TInt bytesPerChar = 1;
    
    if(binaryBufferPtr[0] == hichar && binaryBufferPtr[1] == lochar)
    	{
    	bytesPerChar = 2;

    	HBufC* dataBuffer=NULL;
        TInt fileSize = aFileData.Length();
    	dataBuffer = HBufC::New(aFileData.Length()/bytesPerChar);
    	if ( !dataBuffer)
    		{
        	//_LIT( KErrorMsg, "Failed to Load Svg Content: Out of memory");
        	delete binaryBuffer;
        	return;
    		}

    	TPtr dataBufferPtr = dataBuffer->Des();
    	
    	// Skip two bytes and set the pointer to the next location
    	// from where the actual data starts.
    	dataBufferPtr.Set((TUint16*)binaryBufferPtr.Ptr()+1,
    						fileSize/bytesPerChar-1, 
    						fileSize/bytesPerChar-1);
    		
    	HBufC8* outputBuffer= NULL;
    	outputBuffer=HBufC8::New(fileSize);
    	if(!outputBuffer)
    		{
    		//_LIT( KErrorMsg, "Failed to Load Svg Content: Out of memory");
        	delete dataBuffer;
        	delete binaryBuffer;
        	return ;
    		}
    	TPtr8 outputBufferptr=outputBuffer->Des();
    	CnvUtfConverter::ConvertFromUnicodeToUtf8(
            							outputBufferptr,   // Destination
            							dataBufferPtr );
        
        iFileData = HBufC8::NewL(fileSize);
        iFileData->Des().Copy(outputBufferptr);
        
        delete dataBuffer;
        delete outputBuffer;
        
    	}
    else
        {
        iFileData = HBufC8::NewL(aFileData.Length());
        iFileData->Des().Copy( aFileData );
        }
        delete binaryBuffer;
    }
    
// -----------------------------------------------------------------------------
// CSvgReadCodec::ProcessFrameL
// Processes the frame data contained in aSrc.
// -----------------------------------------------------------------------------
//
TFrameState CSvgReadCodec::ProcessFrameL(TBufPtr8& aSrc )
    {
    aSrc.Shift( 1 );
    TFrameState lRetVal = EFrameComplete;
    DoProcessL();
    
    if(iImagesPresent && !iImagesLoaded)
        {
        lRetVal = EFrameIncompleteRepositionRequest;
        }
    
    return lRetVal;
    }

// -----------------------------------------------------------------------------
// CSvgReadCodec::DoProcessL
// Processes the SVG presentation file data. Called by ProcessFrameL()
// -----------------------------------------------------------------------------
//
void CSvgReadCodec::DoProcessL()
    {
    }

// -----------------------------------------------------------------------------
// CSvgReadCodec::InitFrameL
// Processes the SVG presentation file data. Called by ProcessFrameL()
// -----------------------------------------------------------------------------
//
void CSvgReadCodec::InitFrameL(TFrameInfo& /* aFrameInfo*/, 
    CFrameImageData& /*aFrameImageData*/, 
    TBool /*aDisableErrorDiffusion */, 
    CFbsBitmap& aFrame, 
    CFbsBitmap* /* aDestinationMask */ )
    {
        GenerateThumbnailL(&aFrame, NULL);
    }

void CSvgReadCodec::UpdateScreen()
    {
    }

TBool CSvgReadCodec::ScriptCall( const TDesC& /*aScript*/, CSvgElementImpl* /*aCallerElement*/ )
    {
    return EFalse;
    }

TInt  CSvgReadCodec::FetchImage( const TDesC& aUri, RFs& aSession, RFile& aFileHandle )
    {
    TInt errorCode = KErrNotFound;
    TInt ret = 0;
    TRAPD(err, ret = IsDownloadNeededL(aUri));
    if(err==KErrNone)
        {
        if( ret == ELocalImageFile)
            {
            TFileName fileName;

#ifdef RD_MULTIPLE_DRIVE    

//    RFs fsSession;
            HBufC* fullImagesPathBuf = HBufC::New( KMaxPath );
            if (fullImagesPathBuf)
                {
                TPtr fullImagesPathPtr = fullImagesPathBuf->Des();
                TInt intDrive;
                DriveInfo::TDriveArray driveArray;

//    fsSession.Connect();
    
                TInt err1=DriveInfo::GetUserVisibleDrives( aSession, driveArray );
                if(err1==KErrNone)
                    {
                    for(TInt i=0; i < driveArray.Count(); i++)
                        {
                        TChar driveLetter = driveArray.LetterAt(i);
                        TInt err2=RFs::CharToDrive(driveLetter, intDrive);
                        if(err2==KErrNone)
                            {
                            TInt err3=PathInfo::GetRootPath(fullImagesPathPtr, intDrive);
                            if(err3==KErrNone)
                                {
                                fullImagesPathPtr.Append( PathInfo::ImagesPath() );
                                if(GetLocalFile( aUri, fileName, fullImagesPathPtr))
                                    {
                                    errorCode = aFileHandle.Open( aSession, fileName, EFileShareAny );
                                    break;
                                    }
                                }
                            }
                        }
                    }
                }
            delete fullImagesPathBuf;
            fullImagesPathBuf=NULL;
          
//    fsSession.Close();
        
#else

            HBufC* phoneMemImagesPath = GetImagesPath(EPhoneMemory);
            TPtr ptrphoneMemImagesPath(phoneMemImagesPath->Des());
            HBufC* memCardImagesPath = GetImagesPath(EMemoryCard);
            TPtr ptrMemCardImagesPath(memCardImagesPath->Des());
        
            if(GetLocalFile( aUri, fileName, ptrphoneMemImagesPath))
                errorCode = aFileHandle.Open( aSession, fileName, EFileShareAny );
            else if(GetLocalFile( aUri, fileName, ptrMemCardImagesPath))
                errorCode = aFileHandle.Open( aSession, fileName, EFileShareAny );
        
            delete memCardImagesPath;
            delete phoneMemImagesPath;
#endif        
            }
        }
    return errorCode;
}

TInt CSvgReadCodec::FetchFont( const TDesC& /* aUri */, 
                                    RFs& /* aSession */, RFile& /* aFileHandle */ )
    {
    return KErrNotSupported;
    }
	   
    
void CSvgReadCodec::GetSmilFitValue( TDes& aSmilValue )
    {
    aSmilValue.Copy( KSccSmilFitValue );
    }

void  CSvgReadCodec::UpdatePresentation( const TInt32& /* aNoOfAnimation */ )
    {
    }
    
void CSvgReadCodec::PrepareEngine()
    {
    iPrepareDomResult = iSvgModule->PrepareDom( *iFileData, iThumbnailHandle );
    TInt err = iPrepareDomResult->HasError();
    iSvgModule->UseDom( iThumbnailHandle, NULL);
    iContentSize = iSvgModule->GetSvgBoundingBox().Size();
    if(iContentSize.iHeight < 1)
        {
        iContentSize.iHeight = 1;
        }
    if(iContentSize.iWidth < 1)
        {
        iContentSize.iWidth = 1;
        }
    }
    
TSize CSvgReadCodec::ContentSize() const
    {
    return iContentSize;
    }

void CSvgReadCodec::GenerateThumbnailL(CFbsBitmap* aFrame, CFbsBitmap* /*aFrameMask*/)
    {
    if ( iSvgModule )
        {
        // NULL is returned, possible out of memory when create error object.
        User::LeaveIfNull( iPrepareDomResult );
        User::LeaveIfError(iPrepareDomResult->HasError());
#ifdef SVGTUICONTROL_DBG_DRM_THUMBNAIL_API_ENABLE        
        iSvgModule->SetThumbNailMode( ETrue );            // Set thumbnail mode On
#endif // SVGTUICONTROL_DBG_DRM_THUMBNAIL_API_ENABLE
        
        // Setup the engine to use the DOM
        MSvgError* pResult = iSvgModule->UseDom( iThumbnailHandle, aFrame );
        if ( pResult->HasError() && !pResult->IsWarning() )
            {                
            User::Leave( KErrGeneral );
            }
        
        // View Box define for Dom associated frame
        iSvgModule->ChooseViewBoxIfNotSet(iThumbnailHandle);
        
        // Setup the frame size as the one obtained from 
        // creator of codec.
        const TSize frameSize = aFrame->SizeInPixels();
        // Get the Display mode for bitmap
        TDisplayMode lFrameDispMode = aFrame->DisplayMode();
        // Get the Displaymode for mask
            iSvgModule->InitializeEngine();
            iSvgModule->RenderFrame( NULL, 0);
      // render static content as well
       if ( iFrameList.Count() )
            {
            CFbsBitGc* bitmapContext=NULL;
            CFbsBitmapDevice* bitmapDevice = CFbsBitmapDevice::NewL( aFrame );
            CleanupStack::PushL( bitmapDevice );
            User::LeaveIfError( bitmapDevice->CreateContext( bitmapContext ) );
            CleanupStack::PushL( bitmapContext );
            bitmapContext->BitBlt( TPoint(0,0),iFrameList[ 0 ] );
            CleanupStack::PopAndDestroy(2);
            } 
       
#ifdef SVGTUICONTROL_DBG_DRM_THUMBNAIL_API_ENABLE        
        iSvgModule->SetThumbNailMode( EFalse );       // Set thumbnail mode Off 
#endif // SVGTUICONTROL_DBG_DRM_THUMBNAIL_API_ENABLE
        }
    }

// -----------------------------------------------------------------------------
// CSvgReadCodec::DeleteEngine
// Deletes the structures associated with the engine
// viz. The Frame List, Mask List, Delay Interval List, 
// The DOM for the content and the Engine Implementation itself
// -----------------------------------------------------------------------------
//  
void CSvgReadCodec::DeleteEngine()
    {       
    // Cleanup elements in the frame list
    for ( TInt i = 0; i < iFrameList.Count(); i++ )
        {
        delete iFrameList[i];
        }        

    // Cleanup elements in the mask list
    for ( TInt i = 0; i < iMaskList.Count(); i++ )
        {
        delete iMaskList[i];
        }        
    
    // Cleanup the memory used for storing the pointers/values
    iFrameList.Reset();
    iMaskList.Reset();
    iDelayList.Reset();
    
    // Close the resources
    iFrameList.Close();
    iMaskList.Close();
    iDelayList.Close();

    // Delete the DOM created for the content    
    //if ( iThumbnailHandle )
    //    {
    //    iSvgModule->DeleteDom( iThumbnailHandle );
    //    iThumbnailHandle = 0;
    //    }
    
    // Delete the Engine module 
    delete iSvgModule;    
    iSvgModule = NULL;
    }   

CSvgReadCodec::TDownloadNeeded CSvgReadCodec::IsDownloadNeededL( const TDesC& aUri ) const
    {

    TUriParser UriParser;
    
    TBuf<KMaxFileName> localUri;
    // url starts with www so append http://
    if ( !aUri.Left( 4 ).Compare( KWww ) )     
        {
        localUri.Append( KHttp );
        localUri.Append( aUri );
        User::LeaveIfError( UriParser.Parse( localUri ) );   
        }
    else
        {
        User::LeaveIfError( UriParser.Parse( aUri ) );   
        }
    
    if ( UriParser.IsPresent( EUriHost ) )
        {
        // Extract the Path, filename and extension from the uri
        TFileName filename = UriParser.Extract( EUriPath ); 
        TParsePtrC fileParser( filename );
    
        if ( fileParser.NamePresent() && fileParser.ExtPresent() )
            {                        
            if ( fileParser.Ext().Compare( KJpg ) == 0  ||  
                 fileParser.Ext().Compare( KJpeg ) == 0 ||
                 fileParser.Ext().Compare( KBmp ) == 0  ||
                 fileParser.Ext().Compare( KPng ) == 0  ||
                 fileParser.Ext().Compare( KSvg ) == 0  ||
                 fileParser.Ext().Compare( KSvgz ) == 0 )
                {
                // file name and ext present and 
                // it is amy of the supported image types
                return ERemoteImageFile;         
                }
            else
                {
                //file Extension present but not a supported image ext,
                //may be some html file
                return ENotAnImageFile;
                }
            } 
        else
            {
            //Should be a domain name so return ENotAnImageFile
            return ENotAnImageFile;
            }
        }
    else
        {     
        return ELocalImageFile;
        }                        
    }

TBool CSvgReadCodec::GetLocalFile( const TDesC& aUri, 
                                    TDes& aFileName ,
                                    const TDesC& aRefPath ) const
    {
    aFileName.Zero();
    TBuf<KMaxFileName> copyUri = aUri;
    // Before giving the uri to the file parser,
    // convert '/' to '\'.          
    TInt pos;
    while( KErrNotFound != ( pos = copyUri.FindF( KBSlashStr ) ) )
        {
        copyUri.Replace(pos,1,KSlashStr );
        }

    // if the uri starts with ".\" remove ".\"
    if ( copyUri.Left( 2 ).Compare( KDotSlashStr ) == 0 ) 
        {
        copyUri = copyUri.Right( copyUri.Length() - KDotSlashStr().Length() );
        }   
        
    // If the second letter is not ':' 
    // and if it is not starting with '\',
    // and if the path has '/',
    // then prepend slash to it.
    if ( copyUri.Length() >= 2 && 
         copyUri.Mid( 1, 1 ).Compare( KColonStr ) != 0 && 
         copyUri.Mid( 0, 1 ).Compare( KSlashStr ) != 0 &&
         ( copyUri.Find( KSlashStr ) != KErrNotFound ) )
        {
        copyUri.Insert( 0, KSlashStr );
        }       
    
    TParse fileParser;
    if( fileParser.Set( copyUri, NULL, NULL ) != KErrNone )
        {
        return EFalse;
        } 
            
    if ( fileParser.DrivePresent() )
        {
        // The uri is an absolute path
        aFileName.Append( fileParser.Drive() ); 
        // Append path, file name and extension from the uri.
        if ( fileParser.PathPresent() )
            {
            aFileName.Append( fileParser.Path() );              
            }
        if ( fileParser.NamePresent() )
            {
            aFileName.Append( fileParser.Name() );              
            }
        if ( fileParser.ExtPresent() )
            {
            aFileName.Append( fileParser.Ext() );
            }
            
        if ( !ConeUtils::FileExists( aFileName  ) )
            {
            return EFalse;
            }            
        return ETrue;
        }
    else
        {
        
        TParse svgFilenameParser;
        if ( svgFilenameParser.Set( aRefPath,NULL,NULL ) != KErrNone )
            {
            return EFalse;
            }
        
        if ( svgFilenameParser.DrivePresent() )
            {
            aFileName.Append( svgFilenameParser.Drive() );               
            }
        
        TInt dotdotPos = copyUri.Find( KDotDot );
        TInt rightPos = 0;        
        
        if ( dotdotPos != KErrNotFound )
            {
            // If ".." is present collapse it with the parent path
            if ( svgFilenameParser.PathPresent() )
                {                  
                do   
                    {
                    if ( dotdotPos > 1 )
                        {
                        svgFilenameParser.AddDir( copyUri.Left(dotdotPos - 1) );
                        }
                                                          
                    if ( svgFilenameParser.PopDir() != KErrNone )
                        {
                        return EFalse;
                        }               
                        
                    rightPos = (copyUri.Length()) - (dotdotPos + 3);
                    
                    if ( rightPos > 0 )
                        {                       
                        copyUri = copyUri.Right( rightPos );    
                        }
                                                                                                   
                    }while ( (dotdotPos  = copyUri.Find( KDotDot ) ) 
                                   != KErrNotFound ); 
                                   
                aFileName.Append( svgFilenameParser.Path() );
                aFileName.Append( copyUri );             
                }
            else
                {
                return EFalse;
                }                       
            }
        else
            {
            if ( svgFilenameParser.PathPresent() )
                {
                aFileName.Append( svgFilenameParser.Path() );
                }           
        
            // Append path, file name and extension from the uri.    
            if ( fileParser.PathPresent() )
                {
                TFileName path(fileParser.Path() );
                path.Copy(path.Right(path.Length()-1));
                aFileName.Append( path );
                
                }
                                
            if ( fileParser.NamePresent() )
                {
                aFileName.Append( fileParser.Name() );              
                }
            if ( fileParser.ExtPresent() )
                {
                aFileName.Append( fileParser.Ext() );
                }
            }  
        if ( !ConeUtils::FileExists( aFileName  ) )
            {
            return EFalse;
            }
        return ETrue;   
        }                                  
    }
               
HBufC* CSvgReadCodec::GetImagesPath( 
                        CSvgReadCodec::TImagesLocation aImagesLocation) const
    {
    HBufC* fullImagesPath = HBufC::New( KMaxPath );
    if(fullImagesPath)
        {
        TPtr ptrFullImagesPath(fullImagesPath->Des());
    
        TPtrC8 ptrImagesPath((TText8*)text_images_path);
        TFileName imagesPath16;
        imagesPath16.Copy(ptrImagesPath);
    
        switch( aImagesLocation )
            {
            case EPhoneMemory:
                {
                TPtrC8 ptrRootPhoneMemPath((TText8*)text_phone_memory_root_path);
                ptrFullImagesPath.Copy(ptrRootPhoneMemPath);
                }
                break;
            case EMemoryCard:
                {
                TPtrC8 ptrRootMemCardPath((TText8*)text_memory_card_root_path);
                ptrFullImagesPath.Copy(ptrRootMemCardPath);
                }   
                break;
            default:
                break;    
            }
        ptrFullImagesPath.Append(imagesPath16);
        }
    return fullImagesPath;
    }
    
    //------------------------------------------------------------------------
//From MsvgLoadingListener
/**
     * Query the listener if every element is to be reported or only
     * the children of <svg> tag.
     *
     * @since 1.0
     * @return : ETrue if every element is to be reported, otherwise
     *           only the children of <svg> are reported.
     */
    TBool CSvgReadCodec::ReportAllElements()
        {
        return ETrue;
        }

    /**
     * Query the listener if client will call 'AssignImageData' to decode images
     * separately from parsing.  This is used to avoid non-thread-safe calls in
     * creating bitmap and decoding images, which must be executed in the main thread.
     *
     * @since 1.0
     * @return : ETrue to use 'ImageDataReference' instead of 'FetchImage'.
     */
    TBool CSvgReadCodec::WillAssignImageData()
        {
        return ETrue;
        }

    /**
     * Callback when the href:xlink attribute is encountered for the <image>
     * element.  This method is used to notify clients of image data references
     * needed by <image> element.  This method is called only when 'WillAssignImageData'
     * returns ETrue.
     *
     * @since 1.0
     * @return : none.
     */
    void CSvgReadCodec::ImageDataReference( const TDesC& aUri )
        {
        iImagesPresent = ETrue;
        
        HBufC8* lImageData = NULL;
        
        TInt errorCode = KErrNotFound;
        TInt ret = 0;
        TRAPD(err, ret = IsDownloadNeededL(aUri));
        if(err==KErrNone)
            {
            if( ret != ELocalImageFile)
                {
                lImageData = HBufC8::New(0);
                if(lImageData)
                    {
                    iSvgModule->AssignImageData(aUri , lImageData );
                    }
            
                return;
                }
        
            _LIT(KEncodedImage,"data:image");
        
            if( ! ( aUri.Left(10).Compare(KEncodedImage) ))
                {
                _LIT(KBase,"Base64");
                if(aUri.FindF(KBase) != KErrNotFound)
                    {
                    TInt startIndex = aUri.Locate( ';' );
                    TInt endIndex = aUri.Locate( ',' );
                    // find index of first character after white-space
                    TInt index = endIndex + 1;
                    while ( index < aUri.Length() && TChar( aUri[index] ).IsSpace() )
                        index++;
                    // must be 8-bit
                    TInt length = aUri.Length() - index;
                    HBufC8* encoded = HBufC8::New( length );
                    if(encoded)
                        {
                        encoded->Des().Copy( aUri.Right( length ) );
                        // Assign to member variable to destroy after image is process.
                        lImageData = HBufC8::New( length );
                        if(lImageData)
                            {
                            TPtr8 decodedDes = lImageData->Des();
                            // decode
                            TImCodecB64 encodingBase64;
                            TInt decodeError = encodingBase64.Decode( *encoded, decodedDes );
                            delete encoded;
                            encoded = NULL;
                            }
                        }
                    }            
                else
                    {
                    //lImageData = HBufC8::NewL(0);
                    }            
                }
            else
                {
                RFile lSvgFile;
                TFileName lFileName;
                RFs session;
        
                TInt Connecterror = session.Connect();
        
                HBufC* phoneMemImagesPath = GetImagesPath(EPhoneMemory);
                TPtr ptrphoneMemImagesPath(phoneMemImagesPath->Des());
                HBufC* memCardImagesPath = GetImagesPath(EMemoryCard);
                TPtr ptrMemCardImagesPath(memCardImagesPath->Des());
            
                if(GetLocalFile( aUri, lFileName, ptrphoneMemImagesPath))
                    errorCode = lSvgFile.Open( session, lFileName, EFileShareAny );
                else if(GetLocalFile( aUri, lFileName, ptrMemCardImagesPath))
                    errorCode = lSvgFile.Open( session, lFileName, EFileShareAny );
            
                delete memCardImagesPath;
                delete phoneMemImagesPath;
            
                if(errorCode == KErrNone)
                    {
                    // Get the size of the data to create read buffer
                    TInt lFileSize = 0;
                    TInt sizeError = lSvgFile.Size(lFileSize) ;
                    // Create buffer that will contain the file data
                    lImageData = HBufC8::New(lFileSize);
                    if(lImageData)
                        {
                        TPtr8 lFileDataPtr(lImageData->Des());
                
                        // Read from the file
                        TInt ReadError = lSvgFile.Read(lFileDataPtr);
                        }
                    }
                else
                    {
                    lImageData = HBufC8::New(0);
                    }
                lSvgFile.Close();
                session.Close();
                }
            if(lImageData)
                {
                iSvgModule->AssignImageData(aUri , lImageData );                
                }
            }
        }

    /**
     * Notified when the start of a svg document(<svg> tag) is encountered.
     *
     * @since 1.0
     * @return : For future use.  Value is ignored.
     */
    TBool CSvgReadCodec::DocumentStart()
        {
        return EFalse;
        }

    /**
     * Notified when the end of a svg document(</svg> tag) is encountered.
     *
     * @since 1.0
     * @return : For future use.  Value is ignored.
     */
    TBool CSvgReadCodec::DocumentEnd()
        {
        return ETrue;
        }

    /**
     * Notified when the start of a svg element is encountered.
     *
     * @since 1.0
     * @param : aTagName -- name of svg tag
     * @param : aAttributeList -- attribute list.
     * @return : For future use.  Value is ignored.
     */
    TBool CSvgReadCodec::ElementStart( const TDesC& /*aTagName*/,
                                MSvgAttributeList& /*aAttributeList*/)
        {
        return ETrue;
        }

    /**
     * Notified when the end of a svg element is encountered.
     * activated.
     *
     * @since 1.0
     * @param : aTagName -- name of svg tag
     * @return : For future use.  Value is ignored.
     */
    TBool CSvgReadCodec::ElementEnd( const TDesC& /*aTagName*/ )
        {
        return ETrue;
        }

    /**
     * Notified when an external data is needed by the svg document,
     * such as a image-file or an embedded svg-file.
     *
     * @since 1.0
     * @param : aUri -- URI string of external data
     * @return : For future use.  Value is ignored.
     */
    TBool CSvgReadCodec::ExternalDataRequested( const TDesC& /*aUri*/ )
        {
        return EFalse;
        }

    /**
     * Notified when an external data has been retrieved,
     * such as a image-file or an embedded svg-file.
     *
     * @since 1.0
     * @param : aUri -- URI string of external data
     * @return : For future use.  Value is ignored.
     */
    TBool CSvgReadCodec::ExternalDataReceived( const TDesC& /*aUri*/ )
        {
        return EFalse;
        }

    /**
     * Notified when an external data request has failed.
     *
     * @since 1.0
     * @param : aUri -- URI string of external data
     * @return : For future use.  Value is ignored.
     */
    TBool CSvgReadCodec::ExternalDataRequestFailed( const TDesC& /*aUri*/ )
        {
        return EFalse;
        }

    /**
     * Notified when an unsupported element is encountered.
     *
     * @since 1.0
     * @param : aTagName -- tag name of element
     * @param : aAttributeList -- attribute list.
     * @return : For future use.  Value is ignored.
     */
    TBool CSvgReadCodec::UnsupportedElement( const TDesC& /*aTagName*/,
                                      MSvgAttributeList& /*aAttributeList*/ )
        {
        return EFalse;
        }

    
    void CSvgReadCodec::ImagesLoaded(TInt /*aError*/)
        {
        iImagesLoaded = ETrue;
        iImagesPresent = EFalse;
        }

void CSvgReadCodec::GetNewDataPosition(TInt& aPosition, TInt& /*aLength*/)
    {
    // Reset position to force seek to beginning
    aPosition =0;
        }





//End of file
