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
* Description:  This file implements the SVGT proxy for resolving plugins
*
*/


#include "SVGProxy.h"
#include <ecom.h>
#include <implementationproxy.h>
#include "SVGImageCodecUIDs.hrh"
#include "SVGConvert.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSvgDecodeConstruct::NewL
// Static constructor. Returns the pointer to the CSvgDecodeConstruct
// -----------------------------------------------------------------------------
//
CSvgDecodeConstruct* CSvgDecodeConstruct::NewL()
    {
    CSvgDecodeConstruct* self = new (ELeave) CSvgDecodeConstruct;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CSvgDecodeConstruct::NewPluginL
// Creates a new concrete CImageDecoderPlugin object.
// This is a pure virtual function that each derived class must implement.
// -----------------------------------------------------------------------------
//
CImageDecoderPlugin* CSvgDecodeConstruct::NewPluginL() const
    {
    return CSvgDecoder::NewL();
    }

// Exported proxy for instantiation method resolution
// Define the Implementation UIDs for SVG decoder
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(KSVGDecoderImplementationUidValue, CSvgDecodeConstruct::NewL)
    };

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }
