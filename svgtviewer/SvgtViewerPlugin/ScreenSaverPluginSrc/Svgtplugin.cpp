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
* Description:  SVGT Plugin Implementation source file
 *
*/



#include <e32std.h>
#include <eikenv.h>
#include <barsread.h>
#include <fbs.h>
#include <gdi.h>
#include <f32file.h>

#include "Svgtplugin.h"
#include "SvgScreenSaver.h"


// -----------------------------------------------------------------------------
// CSvgtPlugin::NewL
// -----------------------------------------------------------------------------
//
CSvgtPlugin* CSvgtPlugin::NewL(NPP anInstance)
    {
    CSvgtPlugin *self = new (ELeave) CSvgtPlugin(anInstance);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSvgtPlugin::~CSvgtPlugin
// -----------------------------------------------------------------------------
//
CSvgtPlugin::~CSvgtPlugin()
    {
    if ( iControl )
        {
        delete iControl;        
        }    
    }
    
// -----------------------------------------------------------------------------
// CSvgtPlugin::PrepareToExitL
// -----------------------------------------------------------------------------
//
void CSvgtPlugin::PrepareToExitL()
    {
    if ( iPluginAdapter )
        {
        iPluginAdapter->PluginFinishedL();
        }
    if ( iControl )
        {
        delete iControl;
        iControl=NULL;
        }
    }

// -----------------------------------------------------------------------------
// CSvgtPlugin::ConstructL
// -----------------------------------------------------------------------------
//
void CSvgtPlugin::ConstructL()
    {      
    }

// -----------------------------------------------------------------------------
// CSvgtPlugin::SetWindowL
// Sets the parent control
// -----------------------------------------------------------------------------
//
void CSvgtPlugin::SetWindowL(const NPWindow *aWindow,const TRect& aRect)
    {
    if ( !aWindow )
        {
        User::Leave( KErrArgument );
        }
    iPluginAdapter = static_cast<MPluginAdapter*>(aWindow->window);
    CCoeControl* lParentControl = iPluginAdapter->GetParentControl();

    if ( !iControl )
        {
        // Control is created here as Parent Control is available
        // only in SetWindowL
        iControl = new (ELeave) CSvgScreenSaver;
        iControl->ConstructL( lParentControl );
        iPluginAdapter->SetPluginNotifier(iControl);
        }

    iControl->SetRect(aRect);
    }

// -----------------------------------------------------------------------------
// CSvgtPlugin::CSvgtPlugin
// Default constructor
// -----------------------------------------------------------------------------
//
CSvgtPlugin::CSvgtPlugin(NPP anInstance)
    {
    iScreenSaverMode = EFalse;
    this->iInstance=anInstance;
    }


// -----------------------------------------------------------------------------
// CSvgtPlugin::Control
// returns the control
// -----------------------------------------------------------------------------
//
CSvgScreenSaver* CSvgtPlugin::Control() const
    {
    ASSERT( iControl );
    return iControl;
    }
    
// -----------------------------------------------------------------------------
// CSvgtPlugin::Instance
// returns the instance
// -----------------------------------------------------------------------------
//
NPP CSvgtPlugin::Instance() const
    {
    return iInstance;
    }
 

// End of File
