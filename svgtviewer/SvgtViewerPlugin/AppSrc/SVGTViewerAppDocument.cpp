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
* Description:  Document for SVGT Viewer
*
*/

#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <AiwGenericParam.h>
#include <AiwGenericParam.hrh>

#include "SVGTViewerAppAppUi.h"
#include "SVGTViewerAppDocument.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::CSVGTViewerAppDocument
// Default Constructor for CSVGTViewerAppDocument.
// -----------------------------------------------------------------------------
//
CSVGTViewerAppDocument::CSVGTViewerAppDocument( 
    CEikApplication& aApp ) : // Reference to application object
        CAiwGenericParamConsumer( aApp ),
        iIsFileHandleValid( EFalse )
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::ConstructL
// Second Phase constructor for CSVGTViewerAppDocument.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDocument::ConstructL()
    {
    iIsFileFullPathValid = EFalse;   
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::NewL
// Factory function for creation of CSVGTViewerAppDocument.
// Returns: CSVGTViewerAppDocument*  ; Pointer to created object
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTViewerAppDocument* CSVGTViewerAppDocument::NewL( 
    CEikApplication& aApp ) // Reference to application object
    {
    CSVGTViewerAppDocument* self = NewLC( aApp );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::NewL
// Factory function for creation of CSVGTViewerAppDocument. Also pushes the
// object on the cleanup stack.
// Returns: CSVGTViewerAppDocument*  ; Pointer to created object
//          Leaves if error occurs during creation.
// -----------------------------------------------------------------------------
//
CSVGTViewerAppDocument* CSVGTViewerAppDocument::NewLC( 
    CEikApplication& aApp ) // Reference to application object
    {
    CSVGTViewerAppDocument* self = new ( ELeave )
        CSVGTViewerAppDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// Destructor
CSVGTViewerAppDocument::~CSVGTViewerAppDocument()
    {
    // Close the file handle and session
    iSvgtContentFileHandle.Close();
    delete iFileFullPath;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::GetSvgtContentFileHandle
// This function is a get function to get the file-handle of the file content. 
// This is used by AppUI.
// Returns: File handle of content by reference .
// -----------------------------------------------------------------------------
//
RFile& CSVGTViewerAppDocument::GetSvgtContentFileHandle() 
    {
    return iSvgtContentFileHandle;
    }
 
// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::IsFileHandleValid()
// Used by UI class to check if file's handle is valid
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppDocument::IsFileHandleValid() const
    {
    return iIsFileHandleValid;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::IsFileFullPathAvailable()
// Used by UI class to check if file's full-path available.
// -----------------------------------------------------------------------------
//
TBool CSVGTViewerAppDocument::IsFileFullPathAvailable() const
    {
    return iIsFileFullPathValid;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::GetFileFullPath()
// Used by UI class to get file's full-path.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDocument::GetFileFullPath(TFileName& filename)
    {
    if ( iFileFullPath )
        {
        filename = iFileFullPath->Des();
        }
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::GetParentAppUid()
// Used by UI class to get parent application's uid.
// -----------------------------------------------------------------------------
//
TUid CSVGTViewerAppDocument::GetParentAppUid() const
	{
	return iParentUid;
	}
// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::OpenFileL
// Reimplements CAknDocument::OpenFileL inorder to support opening of SVGT
// files through doc-handler framework.
// Returns: NULL.
// -----------------------------------------------------------------------------
//
CFileStore* CSVGTViewerAppDocument::OpenFileL( TBool aDoOpen,     // Indicates
                                                                  // Whether doc
                                                                  // is to be
                                                                  // created
                                                                  // or opened.
                                         const TDesC& aFilename,  // SVGT
                                                                  // Filename
                                         RFs& aFs )               // File Srvr
                                                                  // Session
    {
    // In this function we store the file-name inside the document.
    // The opening of the document is performed by AppUI after 
    // initialisation.
    if ( aDoOpen )
        {
        CSVGTViewerAppAppUi* AppUi = static_cast<CSVGTViewerAppAppUi*>(iAppUi);
        AppUi->SetAllowMove(0);

        const CAiwGenericParamList* params = GetInputParameters();
        if ( params )
            {
            TInt findIndex;
            const TAiwGenericParam* param = params->FindFirst(findIndex,
                EGenericParamAllowMove);

            if ( param && findIndex != KErrNotFound )
                {
                TInt32 value = 0;
                param->Value().Get(value);
                AppUi->SetAllowMove(value);
                }            
            }
         
        // Get Move parameter
        if ( params && params->Count() )
            {        
            TInt index = 0;
            const TAiwGenericParam* param = NULL;

            param = params->FindFirst(index, EGenericParamApplication, EVariantTypeTUid);

            if ( index >= 0 && param )
                {
                param->Value().Get(iParentUid);              
                }
            }
        }
    
    // Function is called with Full-path.
    iIsFileFullPathValid = ETrue;
    // Store the File's fullpath.
    iFileFullPath = aFilename.AllocL();

    // Open File Handle for the file
    TInt openError = iSvgtContentFileHandle.Open( aFs, aFilename, 
        EFileShareAny );
    if ( openError != KErrNone )
        {
        User::Leave( openError );
        }
    iIsFileHandleValid = ETrue;
    
    return NULL;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::OpenFileL
// Reimplements CAknDocument::OpenFileL inorder to support opening of SVGT
// files through doc-handler framework.
// Returns: NULL.
// -----------------------------------------------------------------------------
//
void CSVGTViewerAppDocument::OpenFileL(
    CFileStore*& aFileStore, // Pointer to reference of filestore
    RFile& aFile ) // File handle of content.
    {
    // Set the docstore pointer to NULL so that the framework
    // does not try to open the file by filename
    aFileStore = NULL;
    
    // In this function we store the file-name inside the document.
    // The opening of the document is performed by AppUI after 
    // initialisation.
    CSVGTViewerAppAppUi* AppUi = static_cast<CSVGTViewerAppAppUi*>(iAppUi);
    AppUi->SetAllowMove(0);

    const CAiwGenericParamList* paramList = GetInputParameters();
    if ( paramList )
        {
        TInt findIndex;
        const TAiwGenericParam* param = paramList->FindFirst(findIndex,
               EGenericParamAllowMove);

        if ( param && findIndex != KErrNotFound )
            {
            TInt32 value = 0;
            param->Value().Get(value);
            AppUi->SetAllowMove(value);
            }            
        }
         

    if ( paramList && paramList->Count() )
        {
        TInt index = 0;
        const TAiwGenericParam* param = NULL;
        param = paramList->FindFirst(index, EGenericParamApplication, EVariantTypeTUid);

        if ( index >= 0 && param )
            {
            param->Value().Get(iParentUid);              
            }
        }
    // Function is not called with Full-path.
    iIsFileFullPathValid = EFalse;
    
    // File-handle is valid
    iIsFileHandleValid = ETrue;
    
    // Make a copy for our use later.
    iSvgtContentFileHandle.Duplicate( aFile );

    // Close the original file handle
    aFile.Close();

    return;
    }
// -----------------------------------------------------------------------------
// CSVGTViewerAppDocument::CreateAppUiL
// Called by the framework to create the AppUI.
// Returns: CEikAppUi* ; Pointer to the created object.
//          Leaves if error.
// -----------------------------------------------------------------------------
//
CEikAppUi* CSVGTViewerAppDocument::CreateAppUiL()
    {
    // Create the application user interface, and return a pointer to it
    // the framework takes ownership of this object
    return ( static_cast < CEikAppUi* >
            ( new ( ELeave ) CSVGTViewerAppAppUi ) );
    }

// End of File
