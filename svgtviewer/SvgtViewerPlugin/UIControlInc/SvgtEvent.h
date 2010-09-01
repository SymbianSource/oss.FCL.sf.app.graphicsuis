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


/*
************************************************************************************
*   SvgtEvent 
************************************************************************************
*/


#ifndef SVGTEVENT_H
#define SVGTEVENT_H

#include <e32base.h>

class CSvgtEvent : public CBase
    {
    public: //Data
    enum TSvgtEventType
        {
        ESvgtEventEmbededImage,
        ESvgtEventFetchImage,
        ESvgtEventLinkActivated,
        ESvgtEventLinkActivatedWithShow,
        ESvgtEventRedraw,
        ESvgtEventInvalid
        };
        
    public: //class methods
        CSvgtEvent(TSvgtEventType aEventType, const TDesC& aUri);
        ~CSvgtEvent();
        TPtrC ImageUri() const;
        TSvgtEventType EventType() const;
    private:
        HBufC* iUri;
        TSvgtEventType iEventType;
    };

class CSvgtEventEmbededImage : public CSvgtEvent
    {
    public:
        CSvgtEventEmbededImage( const TDesC& aUri);
        
    };
    
class CSvgtEventFetchImage : public CSvgtEvent
    {
    public:
        CSvgtEventFetchImage( const TDesC& aUri);
    };

class CSvgtEventLinkActivated : public CSvgtEvent
    {
    public:
        CSvgtEventLinkActivated( const TDesC& aUri);

    };

class CSvgtEventLinkActivatedWithShow : public CSvgtEvent
    {
    public:
        CSvgtEventLinkActivatedWithShow( const TDesC& aUri, const TDesC& aShow);
        TPtrC Show() const;
        
    private:
        HBufC* iShow;
    };

class CSvgtEventRedraw : public CSvgtEvent
    {
    public:
        CSvgtEventRedraw();
    private:
        using CSvgtEvent::ImageUri;
    };
    
#endif //SVGTEVENT_H