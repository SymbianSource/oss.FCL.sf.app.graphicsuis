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



#include <SVGTUIDialog.hrh>
#include <eikenv.h>

#include "Svgtplugin.h"
#include "SvgRenderFrameControl.h"
#include "SvgScreenSaver.h"

// -----------------------------------------------------------------------------
// SvgtPluginNewp
// Creates the plugin.
// -----------------------------------------------------------------------------
//
NPError SvgtPluginNewp(NPMIMEType /*pluginType*/,
                       NPP aInstance, 
                       uint16 /*mode*/, 
                       CDesCArray* /*argn*/, 
                       CDesCArray* /*argv*/, 
                       NPSavedData* /*saved*/)
    {
    CSvgtPlugin* lSvgtPlugin=NULL;
    TRAPD(err,lSvgtPlugin=CSvgtPlugin::NewL(aInstance));

    if (err == KErrNoMemory)
        {
        return NPERR_OUT_OF_MEMORY_ERROR;
        }
    if (err != KErrNone)
        {
        return NPERR_MODULE_LOAD_FAILED_ERROR;
        }
    if ( aInstance )
        {
        aInstance->pdata = (void *) lSvgtPlugin;        
        }
    else
        {
        return NPERR_INVALID_INSTANCE_ERROR;
        }
    return NPERR_NO_ERROR;
    }


// -----------------------------------------------------------------------------
// SvgtPluginDestroy
// Destroy a plugin.
// -----------------------------------------------------------------------------
//
NPError SvgtPluginDestroy(NPP aInstance, NPSavedData** /*save*/)
    {
    if ( !aInstance )
        {
        return NPERR_GENERIC_ERROR;
        }
    CSvgtPlugin* lSvgtPlugin = (CSvgtPlugin *)aInstance->pdata;
    if ( lSvgtPlugin )
        {
        TRAPD(err,lSvgtPlugin->PrepareToExitL());
        if (err != KErrNone)
            {
            #ifdef _DEBUG
            RDebug::Print(_L("SvgtPluginDestroy Error when printing Warning"));
            #endif //_DEBUG
            }
        /**Ignoring the error*/
        delete lSvgtPlugin;
        lSvgtPlugin = NULL;
        }
    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// SvgtPluginSetWindow
// This sets the parent window of a plugin.
// -----------------------------------------------------------------------------
//
NPError SvgtPluginSetWindow(NPP aInstance, NPWindow* aWindow)
    {
    if ( !aInstance )
        {
        return NPERR_INVALID_INSTANCE_ERROR;
        }
    if ( !aWindow )
        {
        return NPERR_GENERIC_ERROR; 
        }
    CSvgtPlugin* lSvgtPlugin = (CSvgtPlugin *) aInstance->pdata;
    TUint lWidth = aWindow->width;
    TUint lHeight = aWindow->height;
   
    TRAPD(err,lSvgtPlugin->SetWindowL(aWindow,TRect(TSize(lWidth,lHeight))));
    if (err == KErrNoMemory)
        {
        return NPERR_OUT_OF_MEMORY_ERROR;
        }
        
    if (err != KErrNone)
        {
        return NPERR_GENERIC_ERROR;
        }
        
    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// SvgtPluginAsFile
// The name of the SVG file is set by AsFile
// -----------------------------------------------------------------------------
//
void SvgtPluginAsFile(NPP aInstance, NPStream* /*stream*/, const TDesC& aFname)
    {
    CSvgtPlugin* lSvgtPlugin = (CSvgtPlugin*)aInstance->pdata;
    TInt err = lSvgtPlugin->Control()->AsFile(aFname);
    if ( err != KErrNone )
        {
        // Destroy myself
        SvgtPluginDestroy( aInstance, NULL );
        }
    }




// -----------------------------------------------------------------------------
// SvgtPluginGetValue
// Generic hook to retrieve values/attributes from the plugin.
// -----------------------------------------------------------------------------
//
NPError SvgtPluginGetValue(NPP aInstance, 
                           NPPVariable aVariable, 
                           void* aRetvalue)
    {
    TInt ret = NPERR_NO_ERROR;
    if ( !aInstance )
        {
        ret = NPERR_INVALID_INSTANCE_ERROR;
        }
    CSvgtPlugin* lSvgtPlugin = (CSvgtPlugin *)aInstance->pdata;       
    if ( lSvgtPlugin && aRetvalue )
        {        
        if ( aVariable == NPPCheckFileIsValid )
            {             
            NPCheckFile* checkFile = (NPCheckFile*)aRetvalue;
            checkFile->fileValidityCheckError = KErrNone;     
            TRAPD(err, SvgtPluginCheckFileL( aRetvalue ) );
            if ( err != KErrNone )
                {
                ret=  NPERR_GENERIC_ERROR;
                }
            ret = NPERR_NO_ERROR;
            }    
        if ( aVariable == NPPScreenSaverGetLastError )
            { 
            TInt* error = (TInt*)aRetvalue;
            *error = lSvgtPlugin->Control()->GetLastError();
            ret =  NPERR_NO_ERROR;
            }               
        }
    else
        {
        ret =  NPERR_GENERIC_ERROR;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// SvgtPluginCheckFileL
// Helper function used to check the file's validity
// -----------------------------------------------------------------------------
//
void SvgtPluginCheckFileL( void* aRetvalue )
    {
    if ( !aRetvalue )
        {
        User::Leave( KErrArgument );
        }
    NPCheckFile* checkFile = (NPCheckFile*)aRetvalue;    
    TFileName fileName = checkFile->fileName->Des();
    RFile fileHandle;
    TInt err = fileHandle.Open( CEikonEnv::Static()->FsSession(), 
                                 fileName, 
                                 EFileShareAny );
    User::LeaveIfError( err );
                
    CleanupClosePushL( fileHandle );
    CSvgRenderFrameControl* control = new ( ELeave )
                      CSvgRenderFrameControl( fileHandle );
                      
    CleanupStack::PushL( control );                      
    TRAP( err, control->InitializeEngineL( ETrue, control ) ); /* CheckFile Mode */
    checkFile->fileValidityCheckError = err;
    CleanupStack::PopAndDestroy( control );
    CleanupStack::PopAndDestroy(&fileHandle); 
    }


// -----------------------------------------------------------------------------
// SvgtPluginSetValue
// Generic hook to set values/attributes within the plugin.
// -----------------------------------------------------------------------------
//
NPError SvgtPluginSetValue(NPP aInstance, 
                           NPNVariable aVariable, 
                           void* aRetvalue)
    {
    TInt ret = NPERR_NO_ERROR;
    if ( !aInstance )
        {
        ret = NPERR_INVALID_INSTANCE_ERROR;
        }
    CSvgtPlugin* lSvgtPlugin = (CSvgtPlugin *)aInstance->pdata;    
    
    if ( lSvgtPlugin )
        {
        switch( aVariable )
            {
            case NPNInteractionMode:
            // do nothing
            break;
            case NPNScreenSaverAnimationEvent:
                {
                NPAnimationEvent animEvent = ( NPAnimationEvent )
                                    (*(NPAnimationEvent*)aRetvalue);
                                    
                if ( animEvent == NPStartAnimation )
                    {
                    // Make the control Visible
                    lSvgtPlugin->Control()->MakeVisible( ETrue );
                    TRAPD(err, lSvgtPlugin->Control()->
                          SendCommandL(ESvgtDlgLoopOnCommand));
                    if ( err != KErrNone )
                        {
                        ret = NPERR_GENERIC_ERROR;            
                        }                                               
                    // Start the animation
                    TRAP( err, lSvgtPlugin->Control()->
                          SendCommandL( ESvgtDlgPlayCommand ) );
                    if ( err != KErrNone )
                        {
                        ret = NPERR_GENERIC_ERROR;
                        }        
                    }
                if ( animEvent == NPEndAnimation )
                    {
                    // Make the control invisible
                    lSvgtPlugin->Control()->MakeVisible( EFalse );
                    // Stop the animation
                    TRAPD( err, lSvgtPlugin->Control()->
                        SendCommandL( ESvgtDlgPauseCommand ) );
                    if ( err != KErrNone )
                        {
                        ret= NPERR_GENERIC_ERROR;            
                        }
                    }        
                }
            break;
            }       
        }
    return ret;
    }



// End of File
