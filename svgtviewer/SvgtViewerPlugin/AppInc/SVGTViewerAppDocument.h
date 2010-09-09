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
* Description:  This header file enlists the Document class interface of
*                the SVGT Viewer App
*
*/



#ifndef __SVGTVIEWERAPPDOCUMENT_H__
#define __SVGTVIEWERAPPDOCUMENT_H__

// INCLUDES
#include <AknDoc.h>
#include <GenericParamConsumer.h>

// FORWARD DECLARATIONS
class CSVGTViewerAppAppUi;
class CEikApplication;

// CLASS DECLARATION

/**
*  Document Class for SVGT Viewer
*  An instance of class CSVGTViewerAppDocument is the Document part of the
*  SVGT Viewer.
*  @since 3.0
*/
class CSVGTViewerAppDocument : public CAiwGenericParamConsumer
    {
    public:  // Constructors and destructor
        /**
        * Two-phased constructor.
        * @param aApp Reference to application object
        * @return CSVGTViewerAppDocument*.
        */
        static CSVGTViewerAppDocument* NewL( CEikApplication& aApp );

        /**
        * Two-phased constructor.
        * @param aApp Reference to application object
        * @return CSVGTViewerAppDocument*.        
        */
        static CSVGTViewerAppDocument* NewLC( CEikApplication& aApp );

        // Destructor
        virtual ~CSVGTViewerAppDocument();
    public: // New functions
        /**
        * GetSvgtContentFileHandle() This function is a get function to 
        * get the file-handle of the file content. This is used by AppUI
        * @since 3.0
        */
        RFile& GetSvgtContentFileHandle();
        
        /**
        * IsFileHandleValid() This function is a get function to 
        * get the flag whether File's handle is valid
        * AppUi uses this method.
        * @since 3.0
        */
        TBool IsFileHandleValid() const;

        /**
        * IsFileFullPathAvailable() This function is a get function to 
        * get the flag whether File's fullpath is available. I.e.
        * file is public. AppUi uses this method.
        * @since 3.0
        */
        TBool IsFileFullPathAvailable() const;

        /**
        * GetFileFullPath() This function is a get function to 
        * get the File's fullpath. AppUi uses this method.
        * @since 3.0
        */
        void GetFileFullPath(TFileName& filename); 
        
        /**
        * GetParentAppUid() This function is a get function to 
        * get the Parent Application's Uid. AppUi uses this method.
        * @since 3.0
        */
        TUid GetParentAppUid() const; 
        

    public:  // Functions from base classes
        /**
        * From CAknDocument This function is used by the Doc Handler to open
        *  SVGT content.
        * @since 3.0
        * @param aDoOpen Boolean indicates whether Document is to be
        *  opened/Created.
        * @param aFilename Descriptor indicates SVGT File path
        * @param aFs File Server Session handle
        * @return CFileStore *.
        */
        virtual CFileStore* OpenFileL( TBool aDoOpen, const TDesC& aFilename,
                                         RFs& aFs );

        /**
        * From CAknDocument This function is used by the Doc Handler to open
        *  the document passed by the framework as a file-handle.
        * @since 3.0
        * @see CAknDocument::OpenFileL
        *         */
        virtual void OpenFileL( CFileStore*& aFileStore, RFile& aFile);
        
        /**
        * From CAknDocument This function is called by the framework to
        *  create the AppUI for SVGT Viewer.
        * @since 3.0
        * @return A pointer to the created instance of the AppUi created
        */
        CEikAppUi* CreateAppUiL();
    private:

        /**
        * C++ default constructor.
        */
        CSVGTViewerAppDocument( CEikApplication& aApp );

        /**
        * Second phase construction of a CSVGTViewerAppDocument object.
        */
        void ConstructL();
     
     private:   
              
        // File handle to the SVGT content
        RFile iSvgtContentFileHandle;

        //File Path if the file launched with full filepath.
        HBufC* iFileFullPath;

        //Was file launched with full-path.
        TBool iIsFileFullPathValid;
        
        //OpenFileL updates this flag after getting the file handle.
        TBool iIsFileHandleValid;
        
        //Uid of the parent application
        TUid iParentUid;

    };


#endif // __SVGTVIEWERAPPDOCUMENT_H__
// End of File
