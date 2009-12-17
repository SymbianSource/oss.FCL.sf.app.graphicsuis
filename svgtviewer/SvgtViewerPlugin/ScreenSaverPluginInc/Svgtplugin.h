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
* Description:  SVGT Plugin Implementation header file
 *
*/


#ifndef SVGTPLUGIN_H
#define SVGTPLUGIN_H

#include <e32std.h>
#include <coecntrl.h>
#include <coemain.h>
#include <fbs.h>
#include <barsread.h>


/**
* Note: Need to ensure that EPOC is defined before including these headers...
*/
#if defined(__SYMBIAN32__) && !defined(EPOC)
    #define EPOC
#endif

#include <npupp.h>
#include <PluginAdapterInterface.h>

//This is to include ECom Style Plugin Specific headers
#include <CEcomBrowserPluginInterface.h>

//Implementation id Svgt Ecom Plugin
const TInt KSvgtPluginImplementationValue = 0x102071E6; 

class   CSvgScreenSaver;
class   CSvgtPlugin;

/**
* This function initializes the NPPluginFuncs with the
* SVG plugin Netscape interface implementation functions
*/
IMPORT_C NPError InitializeFuncs(NPPluginFuncs* aPpf);

/**
* This class is specific to ECom Style Plugin.
* This is used for passing plugin specific initialization information to 
* and from Screensaver Animation Plugin.
*/
class CSvgtPluginEcomMain :  public CEcomBrowserPluginInterface
    {
    public: 
        /**
        * Two-phased constructor.
        * @param aInitParam Has Pointer to NPNImplementationFuncs and
        *                       Pointer to PluginFuncs
        * @since 3.1
        */
        static CSvgtPluginEcomMain* NewL( TAny* aInitParam );
        
        /**
        * Destructor.
        * @since 3.1
        */
        virtual ~CSvgtPluginEcomMain();
        
        /**
        * Returns the iNpf.
        * @since 3.1
        */
        NPNetscapeFuncs* Funcs() const {return iNpf;}
        
    private:
        /**
        * Constructor.
        * @since 3.1
        */
        CSvgtPluginEcomMain( NPNetscapeFuncs* aNpf );
        
        /**
        * Second-phase constructor.
        * @since 3.1
        */
        void ConstructL( NPPluginFuncs* aPluginFuncs );
        
    private:
        // NPN implementation function pointers
        NPNetscapeFuncs* iNpf;
    };

/*******************************************************
Class CSvgtPlugin
********************************************************/

class CSvgtPlugin : public CBase
    {
public:
    /**
    * Two-phased constructor.
    * @since 3.1
    */
    static CSvgtPlugin* NewL(NPP anInstance);
    
    /**
    * Destructor.
    * @since 3.1
    */
    ~CSvgtPlugin();
    
    /**
    * Second-phase constructor.
    * @since 3.1
    */
    void ConstructL();
    
    /**
    * Called before the Plugin Exits.
    * @since 3.1
    */    
    void PrepareToExitL();
    
    /**
    * returns the Control.
    * @since 3.1
    */    
    CSvgScreenSaver* Control() const; 
    
    /**
    * returns the instance.
    * @since 3.1
    */    
    NPP Instance() const;    
        
    /**
    * Called by the SetWindow Netscape API.
    * @param aWindow - structure having the parent control
    * @param aRect - size of the control
    * @since 3.1
    */    
    void SetWindowL(const NPWindow* aWindow,const TRect& aRect);

private:
    /**
    * Default constructor.
    * @since 3.1
    */    
    CSvgtPlugin(NPP anInstance);
 
private:
    // Handle to the plugin Adapter
    MPluginAdapter*             iPluginAdapter;        
    
    // Pointer to the SVG control
    CSvgScreenSaver*            iControl;
    
    // Plugin instance
    NPP                         iInstance;       
    
    // Screensaver mode flag
    TBool                       iScreenSaverMode;
    };


/******************************************************************************
*
*    Plugin API methods
*
******************************************************************************/

/**
* Create a plugin.
*/
NPError SvgtPluginNewp(NPMIMEType /*pluginType*/,
                       NPP aInstance, 
                       uint16 /*mode*/, 
                       CDesCArray* /*argn*/, 
                       CDesCArray* /*argv*/, 
                       NPSavedData* /*saved*/);
/**
* Destroy a plugin.
*/
NPError SvgtPluginDestroy(NPP aInstance, NPSavedData** /*save*/);

/**
* This is the parent window of a plugin.
*/
NPError SvgtPluginSetWindow(NPP aInstance, NPWindow* aWindow);

/**
* A data stream has been fully saved to a file.
*/
void SvgtPluginAsFile(NPP aInstance, 
                      NPStream* /*stream*/, 
                      const TDesC& aFname);


/**
* Generic hook to retrieve values/attributes from the plugin.
*/
NPError SvgtPluginGetValue(NPP aInstance, 
                           NPPVariable aVariable, 
                           void* aRetvalue);

/**
* Generic hook to set values/attributes within the plugin.
*/              
NPError SvgtPluginSetValue(NPP aInstance, 
                           NPNVariable aVariable, 
                           void* aRetvalue);              

/**
* Check File Validity 
*/
void SvgtPluginCheckFileL( void* aRetvalue );


#endif // SVGTPLUGIN_H
// End of File
