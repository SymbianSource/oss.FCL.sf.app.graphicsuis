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
* Description:  Application Entry point for SVGT Viewer App.
*
*/



// INCLUDE FILES
#include <eikstart.h>
#include "SVGTViewerAppApplication.h"

// -----------------------------------------------------------------------------
// NewApplication implements...
// Creates the SVGT Viewer Application.
// -----------------------------------------------------------------------------
//
LOCAL_C CApaApplication* NewApplication()
    {
    return ( static_cast<CApaApplication*>( new CSVGTViewerAppApplication ) );
    }

// -----------------------------------------------------------------------------
// E32Main implements...
// Executable entry point.
// -----------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }

// End of File
