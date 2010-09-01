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



#include "SvgtViewerAppView.h"
#include <AknUtils.h>

void CSvgtViewerAppView::ConstructL(const TRect& aRect)
    {
    CreateWindowL();

    SetRect(aRect);
    ActivateL();
    }

CSvgtViewerAppView::~CSvgtViewerAppView()
    {
    }
    
TInt CSvgtViewerAppView::CountComponentControls() const
	{
	return 0;
	}

CCoeControl* CSvgtViewerAppView::ComponentControl(TInt /* aIndex */) const
	{
    return NULL;
	}

void CSvgtViewerAppView::Draw(const TRect& /* aRect */) const
	{
	CWindowGc& gc = SystemGc();
	gc.Clear();
	}

void CSvgtViewerAppView::HandleResourceChange (TInt /* aType */)
    {
    TRect rect;
    AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, rect );
    SetRect( rect );
    }

//End of file

