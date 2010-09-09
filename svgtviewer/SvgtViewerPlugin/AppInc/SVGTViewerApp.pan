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
* Description:  Panic Function and Panic codes for SVGT Viewer
*
*/


#ifndef __SVGTVIEWERAPP_PAN__
#define __SVGTVIEWERAPP_PAN__

/** SVGTViewerApp application panic codes */
enum TSVGTViewerAppPanics
    {
    ESVGTViewerAppUi = 1
    // add further panics here
    };
    
 _LIT( applicationName, "SVGTViewerApp" );    

inline void Panic( TSVGTViewerAppPanics aReason )
    {   
    User::Panic( applicationName, aReason );
    }

#endif // __SVGTVIEWERAPP_PAN__

// End of File
