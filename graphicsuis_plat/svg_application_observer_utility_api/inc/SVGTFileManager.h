/*
* Copyright (c) 2004, 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  File handling utility. 
*
*/



#ifndef SVGTFILEMANGER_H
#define SVGTFILEMANGER_H

//  INCLUDES
#include <f32file.h>
#include <e32base.h>
#include <SVGTUISaveListener.h>

// CONSTANTS
const TInt KSVGTOneSecond(1000000);
const TInt KSVGTOneMinute(60*KSVGTOneSecond);
const TInt KOneHourInSeconds(3600);
const TInt KSVGTPercent(100);

// CLASS DECLARATION

/**
*  CSVGTFileManager
*  File operations
*
*  @lib SVGTUIControl.lib
*  @since 3.0
*/
NONSHARABLE_CLASS(CSVGTFileManager) : public CActive, 
                                      public MFileManObserver
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CSVGTFileManager* NewL();
                 
        /**
        * Destructor.
        */
        IMPORT_C virtual ~CSVGTFileManager();

    public: // New functions
        
        /**
        * Starts async copy operation
        * @since 3.0
        * @param aCallback Callback to UI side
        * @param anOld source
        * @param aNew target
        * @return KErrDiskFull if mmc full, KErrNone if succesful.
        */
        IMPORT_C TInt StartCopyOperationL( MSVGTUISaveListener* aCallback,  
                                        const TDesC& anOld,const TDesC& aNew );
        
        /**
        * Starts async copy operation
        * @since 3.0
        * @param aCallback Callback to UI side
        * @param aFileHandle source
        * @param aNew target
        * @return KErrDiskFull if mmc full, KErrNone if succesful.
        */
        IMPORT_C TInt StartCopyOperationL( MSVGTUISaveListener* aCallback,  
                                       RFile& aFileHandle, const TDesC& aNew );

        /**
        * Cancels async copy operation
        * @since 3.0
        * @return void
        */
        IMPORT_C void CancelCopy();

        /**
        * Sets iMove if moving is preferred instead of copy.
        * @param Document handler EGenericParamAllowMove parameter value.
        * @since 2.8
        */ 
        IMPORT_C void SetAllowMove( TInt32 aValue ); 

   public: // Functions from MFileManObserver

       /** @see MFileManObserver::NotifyFileManOperation */
       MFileManObserver::TControl NotifyFileManOperation();

   private: // Constructors

        /**
        * C++ default constructor.
        */
        CSVGTFileManager();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private: // From CActive    
        
        /** @see CActive::RunL() */
        void RunL();
        
        /** @see CActive::DoCancel() */
        void DoCancel();

    private: // New fuctions

        /** 
        * Timer callback for UI progress update
        * @since 3.0
        * @param aPtr Pointer to callback class
        * @return KErrNone
        */
        static TInt ProgressUpdate(TAny* aPtr);

        /**
        * Do progress update 
        * @since 3.0
        * @return void
        */
        void DoProgressUpdate();
        
#ifdef SVGTVIEWERAPP_DBG_FLAG        
        void PrintDebugMsg( const TDesC&  aMsg );
#endif        
        

    private:    // Data
        TInt iFileSize;
        TInt iPosition;
        TBool iCancel;
        CFileMan* iFileMan;        
        MSVGTUISaveListener* iCallback; // not owned
        CPeriodic* iProgressUpdater;
        CEikonEnv* iEikEnv;
        TBool iMove; 
    };

#endif      // SVGTFILEMANGER_H 
            
// End of File
