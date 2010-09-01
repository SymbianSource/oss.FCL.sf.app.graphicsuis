/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Application viewer for SVGT Viewer App.
*
*/



#ifndef __SVGTVIEWERAPPVIEW_H__
#define __SVGTVIEWERAPPVIEW_H__

#include <AknControl.h>

//
// class CSvgtViewerAppView
//

class CSvgtViewerAppView : public CAknControl
    {
    public:
        void ConstructL(const TRect& aRect);
        ~CSvgtViewerAppView();
    
    private: // from CCoeControl
        void Draw(const TRect& /*aRect*/) const;
        TInt CountComponentControls() const;
        CCoeControl* ComponentControl(TInt aIndex) const;
        void HandleResourceChange (TInt aType);
    };


#endif //__SVGTVIEWERAPPVIEW_H__
