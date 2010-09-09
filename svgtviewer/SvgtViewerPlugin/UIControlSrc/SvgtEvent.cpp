/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/

#include "SvgtEvent.h"

CSvgtEvent::CSvgtEvent(CSvgtEvent::TSvgtEventType aEventType,  const TDesC& aUri):
                            iEventType (aEventType)
    {
    iUri = aUri.Alloc();
    }

CSvgtEvent::~CSvgtEvent()
    {
    delete iUri;
    }
    
CSvgtEvent::TSvgtEventType CSvgtEvent::EventType() const
    {
    return iEventType;
    }

TPtrC CSvgtEvent::ImageUri() const
    {
    return TPtrC( iUri->Des() );
    }

CSvgtEventEmbededImage::CSvgtEventEmbededImage( const TDesC& aUri):
                        CSvgtEvent(CSvgtEvent::ESvgtEventEmbededImage, aUri)
    {
    
    }
CSvgtEventFetchImage::CSvgtEventFetchImage( const TDesC& aUri):
                        CSvgtEvent(CSvgtEvent::ESvgtEventFetchImage, aUri)
    {
    }
    
CSvgtEventLinkActivated::CSvgtEventLinkActivated( const TDesC& aUri):
                        CSvgtEvent(CSvgtEvent::ESvgtEventLinkActivated, aUri)
    {
    }

CSvgtEventLinkActivatedWithShow::CSvgtEventLinkActivatedWithShow( const TDesC& aUri, 
                                const TDesC& aShow): 
                                CSvgtEvent(CSvgtEvent::ESvgtEventLinkActivatedWithShow, aUri)
    {
    iShow = aShow.Alloc();
    }
    
TPtrC CSvgtEventLinkActivatedWithShow::Show() const
    {
    return TPtrC(iShow->Des());
    }

CSvgtEventRedraw::CSvgtEventRedraw() : CSvgtEvent( CSvgtEvent::ESvgtEventRedraw, KNullDesC )
    {
    }

//End of file
