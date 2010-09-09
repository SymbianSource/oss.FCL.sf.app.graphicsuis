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
* Description:  Mixin Class that defines the call back interface used
*                by application using the SVGT Control.
*
*/


#ifndef SVGTAPPOBSERVER_H
#define SVGTAPPOBSERVER_H

/**
*  MSvgtAppObserver is an interface class used by SVGT control to callback to
*   the application.
*  @lib SVGTUIControl.lib
*  @since 3.0
*/

class MSvgtAppObserver
{
    public: 

           
        /**
        * This method is called to retrieve the absolute path of an image gvien
        * the uri. The SVGT control requests the absolute path to provide the     
        * client with control of the location of the images.
        * @since 3.0
        * see MSvgRequestObserver::FetchImage
        */                                       
        virtual TInt  FetchImage( const TDesC& aUri, 
             RFs& aSession, RFile& aFileHandle ) = 0;                                  
                                  
        /**
        * This method is called to notify the client that a link has been
        * activated.
        *
        * @since 3.0
        * @param : aUri -- URI string of hyperlink
        * @return : For future use.  Value is ignored.
        */
        virtual TBool LinkActivated( const TDesC& aUri ) = 0 ;

         /**
         * This method is called to notify the client that a link has been
         * activated.  When Show is also present .
         *
         * @since 3.0
         * @param : aUri -- URI string of hyperlink
         * @param : aShow -- xlink:show value
         * @return : For future use.  Value is ignored.
         */
        virtual TBool LinkActivatedWithShow( const TDesC& aUri, 
                                             const TDesC& aShow ) = 0;
                                             
        /**
        * This method is called to notify the client that download menu has 
        * been selected in the UI dialog
        * @since 3.0     
        */                                             
        virtual void DisplayDownloadMenuL( void ) = 0;

        /**
        * This method is called to get the SMIL fit value to be used by the
        * engine.
        * @since 3.0     
        */                                             
        virtual void GetSmilFitValue( TDes& aSmilValue ) = 0;
                                                     
        /**
        * This method is called to check from the client if Save option
        * should be displayed.
        *
        * @since 3.0
        * @return : ETrue: Save can be shown. EFalse: Save can't be shown.
        */
        virtual TBool CanShowSave() = 0;

        /**
        * This method is called to check from the client if Saving 
        * is done already
        *
        * @since 3.0
        * @return : ETrue: The content is saved
        *           EFalse: content is not saved
        */
        virtual TBool IsSavingDone() = 0;
        
        /**
        * This method is called to check from the client if Downloads option
        * should be displayed.
        *
        * @since 3.0
        * @return : ETrue: Downloads can be shown. 
        *           EFalse: Downloads can't be shown.
        */
        virtual TBool CanShowDownload() const = 0;


        /**
        * This method is called to perform the save operation. 
        * Note the Save functionality depends on the client invoking
        * the dialog. E.g. Browser->SVGT-Viewer->Dialog then save
        * results in Move operation to save memory.
        * param aButtonId context in which save is called
        * @since 3.0
        * @return : void.
        */

        virtual void DoSaveL( TInt aButtonid ) = 0;

        /**
        * This method is called to retrieve the absolute path of an image given
        * the uri. The SVGT control requests the absolute path to provide the     
        * client with control of the location of the images. This function should
        * be an asynchronous, i.e. it should not block.
        * On completion of fetching of the file, it should notify the control
        * using AssignImageData() function
        * @since 3.1
        * see MSvgRequestObserver::ImageDataReference
        */                                       
        virtual TInt NewFetchImageData( const TDesC& aUri ) = 0;

        /**
        * This method is called instead of NewFetchImageData in case the image 
        * is defined as part of the SVG content itself i.e. inline using base-64
        * encoding for eg.
        * @since 3.1
        * see MSvgRequestObserver::ImageDataReference
        */                                       
        virtual void AssignEmbededDataL( const TDesC& aUri ) = 0;
        virtual TBool ExitWhenOrientationChange() = 0 ; 
};
#endif
// End of File
