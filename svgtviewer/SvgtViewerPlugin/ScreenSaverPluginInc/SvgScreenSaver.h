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
* Description:  SVGScreenSaver provides an implementation of ScreenSaverPlugin
*                for Svg files.
*
*/


#ifndef SVGSCREENSAVER_H
#define SVGSCREENSAVER_H

#include <coecntrl.h>
#include <PluginAdapterInterface.h>
#include <SVGTAppObserver.h>
#include <NPNExtensions.h>

const TInt KMaxFileSize=10000;  
class CSVGTCustControl;
class CEikonEnv;


/**
*  CSvgScreenSaver provides an implementation of MSvgtAppObserver
*  It makes use of RenderFrames Control or SVGTCustControl to render
*  the SVGT content.
*  @lib SVGTScreenSaverPlugin.lib
*  @since 3.1
*/
class CSvgScreenSaver: public CBase,
                       public MSvgtAppObserver,
                       public MPluginNotifier                 
    {    
    
    public:             
                      
        
        /**
        * Default Constructor
        * @since 3.1
        */  
        CSvgScreenSaver();
        
        /**
        * Destructor.
        */
        virtual ~CSvgScreenSaver(); 
        
        /**
        * 2nd phase constructor
        * @since 3.1
        */
        void ConstructL( const CCoeControl* aParent );
        
                
        /**
        * Creates an SVG Custom Control or RenderFrames Control 
        * for the file
        * @param aFileName 
        * @since 3.1
        */        
        TInt AsFile(const TDesC& aFileName);    
        
        /**
        * Sets the rect for SVG Custom Control or RenderFrames Control         
        * for the file
        * @param aRect
        * @since 3.1
        */  
        void SetRect(const TRect& aRect);   
        
        /**
        * Sends the command to SVG Custom Control or RenderFrames Control         
        * @param aCommand command to be sent to the control
        * @since 3.1
        */  
        void SendCommandL( TInt aCommand );
        
        /**
        * makes the control to be (in)visible        
        * @param aVisible ETrue makes the control visible
        *                 EFalse makes the control invisible
        * @since 3.1
        */
        void MakeVisible( TBool aVisible );
        
        /**
        * gets the last error encountered by the Control        
        * @since 3.1
        */
        TInt GetLastError() const;
        
    public: // Functions From MSvgtAppObserver
            
        /**
        * From MSvgtAppObserver 
        * @see MSvgtAppObserver::FetchImage
        */
        virtual TInt FetchImage( const TDesC& aUri, RFs& aSession, 
                                 RFile& aFileHandle );
                                 
        /**
        * From MSvgtAppObserver 
        * @see MSvgtAppObserver::GetSmilFitValue
        */
        virtual  void GetSmilFitValue(TDes& aSmilValue);
        
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::LinkActivated
        */
        virtual TBool LinkActivated( const TDesC& aUri );

        
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::LinkActivatedWithShow
        */
        virtual TBool LinkActivatedWithShow( const TDesC& aUri, 
                                             const TDesC& aShow );
               
        /**
        * This function is called by the application to enable/disable the
        * downloads menu in the options menu
        * @since 3.0
        * @param aAddDownloadMenu enable/disable the downloads menu  
        */   
                                 
        void SetDownloadMenu( TBool aAddDownloadMenu );
        
        /**
        * This function is called by the Ui Dialog to enable/disable the
        * downloads menu in the options menu
        * @since 3.0
        * @return iDownloadMenuEnabled enable/disable the downloads menu  
        */   
        virtual TBool CanShowDownload() const;
        

        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::DisplayDownloadMenuL
        */ 
        virtual void DisplayDownloadMenuL();   
                                                           
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::CanShowSave
        */
        virtual TBool CanShowSave();
        
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::IsSavingDone
        */
        virtual TBool IsSavingDone();

        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::DoSaveL
        */ 
        virtual void DoSaveL( TInt aCommandId );         
               
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::NewFetchImageData
        */ 
        virtual TInt NewFetchImageData( const TDesC& aUri );

        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::AssignEmbededDataL
        */ 
        virtual void AssignEmbededDataL( const TDesC& aUri );
        
        /**
        * From MSvgtAppObserver
        * @see MSvgtAppObserver::ExitWhenOrientationChange
        */
        virtual TBool ExitWhenOrientationChange();
 
    private: // In MPluginNotify in S60
        TInt NotifyL(TNotificationType aCallType, void* aParam);    

    private:                                
        
        // Pointer to the Svg control               
        CCoeControl* iSvgControl;
        
        // Pointer to the Parent Control
        const CCoeControl* iParent;
        
        // file Handle of the file displayed
        RFile iFileHandle;
        
        // environment pointer
        CEikonEnv* iEikEnv;
        
        // current size of the control
        TRect iRect;
        
        // flag to indicate whether render frames control or
        // Custom control is used.
        TBool iIsRenderFrameControl;
        
        // Name of the Svg file
        TFileName iFileName;
        
        // last error encountered by the SVG Plugin
        TInt iLastError;
    };

#endif //  SVGSCREENSAVER_H
// End of File
