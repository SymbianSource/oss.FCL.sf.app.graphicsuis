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
* Description:  This class defines the downloads and the required attributes. 
*
*/



// INCLUDE FILES
#include <e32std.h>                                    // TFileName

#include "SVGTViewerAppDownloads.h"


// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSVGTViewerAppDownloads* CSVGTViewerAppDownloads::NewL( 
        TBool aIsEmbedded, 
        TBool aIsSynchronous,
        TBool aLinkFlag )

    {
    CSVGTViewerAppDownloads* self = CSVGTViewerAppDownloads::NewLC( 
                    aIsEmbedded, aIsSynchronous, aLinkFlag);
    CleanupStack::Pop( self );
    return self;
   
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CSVGTViewerAppDownloads* CSVGTViewerAppDownloads::NewLC( 
            TBool aIsEmbedded, 
            TBool aIsSynchronous ,
            TBool aLinkFlag )

    {
    CSVGTViewerAppDownloads* self =   new( ELeave ) CSVGTViewerAppDownloads( 
                    aIsEmbedded, aIsSynchronous, aLinkFlag );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------    
// Destructor
// -----------------------------------------------------------------------------
CSVGTViewerAppDownloads::~CSVGTViewerAppDownloads()
    {    
    // Reset the Http Download Handle as it is owned by caller
    iHttpDownload = NULL;
    }
// -----------------------------------------------------------------------------    
// CSVGTViewerAppDownloads::LinkOffset()
// -----------------------------------------------------------------------------
TInt CSVGTViewerAppDownloads::LinkOffset()
    {
    return _FOFF(CSVGTViewerAppDownloads,iLink);
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::FileName
// Returns the file name associated with the download.
// -----------------------------------------------------------------------------
//
TFileName CSVGTViewerAppDownloads::FileName() const
    {
    return iFileName;
    }
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::EmbeddedMode
// Returns the EmbeddedMode flag associated with the download.
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppDownloads::EmbeddedMode() const
    {
    return iEmbeddedMode;
    }
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::SynchronousMode
// Returns the SynchronousMode flag associated with the download.
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppDownloads::SynchronousMode() const
    {
    return iSynchronousMode;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::DownloadCompleted
// Returns the DownloadCompleted flag associated with the download.
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppDownloads::DownloadCompleted() const
    {
    return iDownloadCompleted;
    }
    
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::QueLink
// This Function returns the download handle associated with the download
// -----------------------------------------------------------------------------
//
TDblQueLink CSVGTViewerAppDownloads::QueLink() const
    {
    return iLink;
    }
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::Download
// This Function returns the download handle associated with the download
// -----------------------------------------------------------------------------
//
RHttpDownload* CSVGTViewerAppDownloads::DownloadHandle()
    {
    return iHttpDownload;
    }

TBool CSVGTViewerAppDownloads::ImageLinkFlag() const
    {
    return iImageLinkFlag;
    }    
    
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::SetFileName
// This Function Sets the filename associated with the download
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDownloads::SetFileName( const TDesC& aFileName )
    {
    iFileName = aFileName;
    }
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::SetEmbeddedMode
// This function sets the EmbeddedMode flag associated with the download.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDownloads::SetEmbeddedMode( TBool aEmbeddedMode ) 
    {
    iEmbeddedMode = aEmbeddedMode;
    }
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::SetSynchronousMode
// This function sets the SynchronousMode flag associated with the download.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDownloads::SetSynchronousMode( TBool aSynchronousMode ) 
    {
    iSynchronousMode = aSynchronousMode;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::SetDownloadCompleted
// This function sets the Downloadcompleted flag associated with the download.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDownloads::SetDownloadCompleted( TBool aDownloadCompleted ) 
    {
    iDownloadCompleted = aDownloadCompleted;
    }
    
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::SetDownloadHandle
// This function sets the Download handle associated with the download.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDownloads::SetDownloadHandle( RHttpDownload* aDownloadHandle ) 
    {
    iHttpDownload = aDownloadHandle;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::SetImageLinkFlag
// Image flag to distinguish the Embeded & hyperlink.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDownloads::SetImageLinkFlag(TBool aLinkFlag)
    {
    iImageLinkFlag = aLinkFlag;
    }
    
// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::CSVGTViewerAppDownloads
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSVGTViewerAppDownloads::CSVGTViewerAppDownloads( TBool aIsEmbedded,
                    TBool aIsSynchronous , TBool aLinkFlag ) : 
                             iEmbeddedMode( aIsEmbedded ), 
                             iSynchronousMode( aIsSynchronous ),
                             iDownloadCompleted( EFalse ),
                             iImageLinkFlag(aLinkFlag)
    {
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDownloads::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDownloads::ConstructL()
    {
    }
//  End of File  
