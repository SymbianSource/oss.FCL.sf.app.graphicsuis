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
* Description:  Application Class for SVGT Viewer
*
*/



// INCLUDE FILES
#include <eikstart.h>
#include "SVGTViewerAppApplication.h"
#include "SVGTViewerAppDocument.h"

// Constants

// UID for the application; this should correspond to the uid defined in the mmp file
const TUid KUidSVGTViewerAppApp = { 0x101F874A };

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTViewerAppApplication::AppDllUid
// Reimplements CApaApplication::AppDllUid inorder to return SVGT Viewer App's
// UID
// Returns: TUid ; SVGT Viewer App UID
// -----------------------------------------------------------------------------
//
TUid CSVGTViewerAppApplication::AppDllUid() const
    {
    // Return the UID for the SVGTViewerApp application
    return KUidSVGTViewerAppApp;
    }

// -----------------------------------------------------------------------------
// CSVGTViewerAppApplication::CreateDocumentL
// Invokes the factory function for the Document Class. Called by the
// framework.
// Returns: CApaDocument* ; Pointer to the created Document Class
// -----------------------------------------------------------------------------
//
CApaDocument* CSVGTViewerAppApplication::CreateDocumentL()
    {
    // Create an SVGTViewerApp document, and return a pointer to it
    return ( static_cast< CApaDocument* >(
        CSVGTViewerAppDocument::NewL( *this ) ) );
    }

// End of File
