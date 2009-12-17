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
* Description:  SVGT proxy for resolving plugin.
*
*/


#ifndef SVGDECODECONSTRUCT_H
#define SVGDECODECONSTRUCT_H

#include <imageconstruct.h>

class CSvgDecodeConstruct : public CImageDecodeConstruct
    {
public:
    /**
    * Static constructor. Creates and returns a pointer to CSvgDecodeConstruct 
    * @since 3.1
    * @return Pointer to the decode construct instance.
    */
    
    static CSvgDecodeConstruct* NewL();

    // From CImageDecodeConstruct
    /**
	* Creates a new concrete CImageDecoderPlugin object.
	* This is a pure virtual function that each derived class must implement.
	* @return A pointer to a fully constructed CImageDecoderPlugin.
	*/
	
    CImageDecoderPlugin* NewPluginL() const;
    };

#endif //SVGDECODECONSTRUCT_H