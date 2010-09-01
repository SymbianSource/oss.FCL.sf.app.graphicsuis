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



#ifndef SVGTVIEWERAPPDOWNLOADS_H
#define SVGTVIEWERAPPDOWNLOADS_H

//  INCLUDES
#include <downloadmgrclient.h>                        //RHttpDownload
#include <e32base.h>                                  //CBase


// CLASS DECLARATION

/**
*  This class declares the Downloads and the associated attributes.
*
*  @since 3.0
*/
class CSVGTViewerAppDownloads : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aLinkFlag Image flag to distinguish the Embeded & hyperlink.
        */
        static CSVGTViewerAppDownloads* NewL( TBool aIsEmbedded, 
                                              TBool aIsSynchronous,
                                              TBool aLinkFlag );

        /**
        * Two-phased constructor.
        * @param aLinkFlag Image flag to distinguish the Embeded & hyperlink.
        */
        static CSVGTViewerAppDownloads* NewLC( TBool aIsEmbedded, 
                    TBool aIsSynchronous, TBool aLinkFlag );

        
        /**
        * Destructor.
        */
        virtual ~CSVGTViewerAppDownloads();
        
        /**
        * returns link offset.
        */
        static TInt LinkOffset();

    public: // New functions
        
        /**
        * This Function returns the filename associated with the download
        * @return FileName
        */
        TFileName FileName() const;

        /**
        * This Function returns the flag iEmbeddedMode
        * associated with the download
        * @return TBool
        */
        TBool EmbeddedMode() const;

        /**
        * This Function returns the flag iSynchronousMode
        * associated with the download
        * @return TBool
        */
        TBool SynchronousMode() const;

        /**
        * This Function returns the flag iDownloadCompleted
        * associated with the download
        * @return TBool
        */
        TBool DownloadCompleted() const;
        
        /**
        * This Function returns the Link in the doubly linked list
        * associated with the download
        * @return TBool
        */
        TDblQueLink QueLink() const;

        /**
        * This Function returns the download handle
        * associated with the download
        * @return RHttpDownload
        */
        RHttpDownload* DownloadHandle();

        /**
        * This Function returns the Image Link flag status
        * associated with the download
        * @return TBool
        */
        TBool ImageLinkFlag() const;
        
        /**
        * This Function Sets the filename
        * associated with the download
        */
        void SetFileName( const TDesC& aFileName );

        /**
        * This Function Sets the flag EmbeddedMode
        * associated with the download
        */
        void SetEmbeddedMode( TBool aEmbeddedMode );


        /**
        * This Function Sets the flag SynchronousMode
        * associated with the download
        */
        void SetSynchronousMode( TBool aSynchronousMode );

        /**
        * This Function Sets the flag iDownloadCompleted
        * associated with the download
        */
        void SetDownloadCompleted( TBool aDownloadCompleted );
        
        /**
        * This Function Sets the Download Handle
        * associated with the download
        */
        void SetDownloadHandle( RHttpDownload* aDownloadHandle ); 

        /**
        * This function sets the image link flag.
        * @param aLinkFlag Image flag to distinguish the Embeded & hyperlink.
        */
        void SetImageLinkFlag(TBool aLinkFlag);
        
    private:

        /**
        * C++ default constructor.
        */
        CSVGTViewerAppDownloads( 
                                    TBool aIsEmbedded, 
                                    TBool aIsSynchronous,
                                    TBool aLinkFlag );


        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

        // Pointer to Download handle
        RHttpDownload* iHttpDownload;

        // Downloaded fileName
        TFileName iFileName;

        // Boolean variable to indicate whether the downloaded file 
        // has to be opened in embedded mode or independent mode.
        TBool iEmbeddedMode;

        // Boolean variable to indicate whether the download
        // has to be performed in a synchronous mode or asynchronous mode
        TBool iSynchronousMode;

        // Flag to indicate successful download completion
        TBool iDownloadCompleted;
        
        // Doubly linked list link
        TDblQueLink iLink;       

        //Image flag to distinguish the Embeded & hyperlink.
        //EFalse for FetchImage
        //ETrue for HyperLink.
        TBool iImageLinkFlag;    
    };

#endif      // SVGTVIEWERAPPDOWNLOADS_H
            
// End of File
