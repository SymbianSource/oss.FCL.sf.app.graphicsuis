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


#include <implementationproxy.h>
#include <CEcomBrowserPluginInterface.h>

#include "Svgtplugin.h"

const TInt KPluginVersion = 1;

// -----------------------------------------------------------------------------
// CSvgtPluginEcomMain::NewL
// -----------------------------------------------------------------------------
//  
CSvgtPluginEcomMain* CSvgtPluginEcomMain::NewL(TAny* aInitParam)
    {
    if ( !aInitParam )
        {
        User::Leave(KErrArgument);
        }
        
    // aInitParam has both the NPNImplementation Functions implemented by
    // the ScreensaverAnimPlugin and the functions  implemented by the 
    // SVGPlugin for animation plugin.  
        
    TFuncs* funcs = REINTERPRET_CAST( TFuncs*, aInitParam);
    CSvgtPluginEcomMain* self = new(ELeave)
                                CSvgtPluginEcomMain(funcs->iNetscapeFuncs);
    CleanupStack::PushL(self);
    self->ConstructL(funcs->iPluginFuncs);
    CleanupStack::Pop( self );
    Dll :: SetTls ( (void*) self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSvgtPluginEcomMain::ConstructL
// -----------------------------------------------------------------------------
//  
void CSvgtPluginEcomMain::ConstructL(NPPluginFuncs* aPluginFuncs)
    {
    InitializeFuncs(aPluginFuncs);
    }

// -----------------------------------------------------------------------------
// CSvgtPluginEcomMain::CSvgtPluginEcomMain
// -----------------------------------------------------------------------------
//  
CSvgtPluginEcomMain::CSvgtPluginEcomMain(NPNetscapeFuncs* aNpf) : 
                            CEcomBrowserPluginInterface(),iNpf(aNpf)
    {
    }

// -----------------------------------------------------------------------------
// CSvgtPluginEcomMain::~CSvgtPluginEcomMain
// -----------------------------------------------------------------------------
// 
CSvgtPluginEcomMain::~CSvgtPluginEcomMain()
    {    
    }
      
const TImplementationProxy KImplementationTable[] =
    {
    {{KSvgtPluginImplementationValue}, 
    (TProxyNewLPtr) CSvgtPluginEcomMain::NewL}
    };

// -----------------------------------------------------------------------------
// ImplementationGroupProxy
// -----------------------------------------------------------------------------
// 
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(KImplementationTable) / sizeof(TImplementationProxy);
    return KImplementationTable;
    }

// -----------------------------------------------------------------------------
// InitializeFuncs
// -----------------------------------------------------------------------------
// 
EXPORT_C NPError InitializeFuncs(NPPluginFuncs* aPpf)
    {
    if ( !aPpf )
        {
        return NPERR_GENERIC_ERROR;
        }
    aPpf->size = sizeof(NPPluginFuncs);
    aPpf->version = KPluginVersion;
    aPpf->newp          = NewNPP_NewProc(SvgtPluginNewp);
    aPpf->destroy       = NewNPP_DestroyProc(SvgtPluginDestroy);
    aPpf->setwindow     = NewNPP_SetWindowProc(SvgtPluginSetWindow);
    aPpf->newstream     = NULL;
    aPpf->destroystream = NULL;
    aPpf->asfile        = NewNPP_StreamAsFileProc(SvgtPluginAsFile);
    aPpf->writeready    = NULL;
    aPpf->write         = NULL;
    aPpf->print         = NULL;
    aPpf->event         = NULL;
    aPpf->urlnotify     = NULL;
    aPpf->javaClass     = NULL;
    aPpf->getvalue      = NewNPP_GetValueProc(SvgtPluginGetValue);
    aPpf->setvalue      = NewNPP_SetValueProc(SvgtPluginSetValue);
    return NPERR_NO_ERROR;
    
    }

// -----------------------------------------------------------------------------
// NPP_Shutdown
// -----------------------------------------------------------------------------
// 
EXPORT_C void NPP_Shutdown(void)
    {
    CSvgtPluginEcomMain* npm = (CSvgtPluginEcomMain*) Dll :: Tls ();
    delete npm;
    Dll :: SetTls ( NULL );
    }


// End of File
