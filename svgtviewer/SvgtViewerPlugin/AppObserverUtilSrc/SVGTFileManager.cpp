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
* Description:  File operations, used when "Save" option is selected.
*
*/



// INCLUDE FILES

#include <flogger.h>
#include <eikenv.h>
#include <pathinfo.h>
#include <sysutil.h>

#include "SVGTViewerAppDbgFlags.hrh"
#include "SVGTFileManager.h"
#include "SVGTUISaveListener.h"


#ifdef SVGTVIEWERAPP_DBG_FLAG    
_LIT( KFileLoggingDir, "SVGTViewer" );
_LIT( KFileLog, "SaveLog.txt" );
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSVGTFileManager::CSVGTFileManager
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSVGTFileManager::CSVGTFileManager() : CActive(0)
    {
    }

    
// -----------------------------------------------------------------------------
// CSVGTFileManager::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSVGTFileManager::ConstructL()
    {
    iEikEnv = CEikonEnv::Static();    
    iFileMan = CFileMan::NewL( iEikEnv->FsSession(),this );
    iProgressUpdater =  CPeriodic::NewL( 0 );
    // Add this active object to the scheduler.
    CActiveScheduler::Add( this );    
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CSVGTFileManager* CSVGTFileManager::NewL()
    {
    CSVGTFileManager* self = new( ELeave ) CSVGTFileManager();   
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::~CSVGTFileManager   
// Destructor
// -----------------------------------------------------------------------------
//
EXPORT_C CSVGTFileManager::~CSVGTFileManager()
    {
    delete iFileMan;
    delete iProgressUpdater;
    // Reset the callback implementation pointer
    iCallback = NULL;
    // Reset the environment pointer
    iEikEnv = NULL;
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::StartCopyOperationL
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CSVGTFileManager::StartCopyOperationL( 
    MSVGTUISaveListener* aCallback, const TDesC& anOld, const TDesC& aNew )
    {
    if (IsActive())
        {
        return KErrInUse;
        }

    iPosition = 0;
    iCancel = EFalse;
    RFile lFile;
    TInt RetVal = KErrNone;//return value
    iCallback = aCallback;   
    RetVal = lFile.Open(iEikEnv->FsSession(),anOld, EFileRead); 
    if ( RetVal != KErrNone)
        {
        // if there was a problem opening this file then this should return here 
        //itself. Or else RTIY-6JNVHS error would occur.
        return RetVal ;
        } 
    lFile.Size(iFileSize);//get the file size into iFileSize
    lFile.Close();//no filehandle should be open while doing a "move"        
     
   
#ifdef SVGTVIEWERAPP_DBG_FLAG    
    _LIT( errorMsg2, "Size read");   
    PrintDebugMsg( errorMsg2 );
#endif
        
    // Check disk space
    TParsePtrC parse(aNew);

#ifndef RD_MULTIPLE_DRIVE
    if( parse.Drive().CompareF( PathInfo::MemoryCardRootPath().Left(2) ) == 0 )
        {
        if (SysUtil::MMCSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),iFileSize))
            {            
            return KErrDiskFull; // UI shows note
            }
        }
    else
        {
        if (SysUtil::FFSSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),iFileSize))
            {
            User::Leave(KErrDiskFull); // Phone memory full, Avkon shows note
            }
        }  
#else
//********************** Added the support for multiple drive ************************
    TInt intDrive;
    TChar ch = parse.Drive()[0];
    
    User::LeaveIfError( RFs::CharToDrive(ch,intDrive) );
    
    if( SysUtil::DiskSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),
                                iFileSize, intDrive ))
        {
        User::Leave( KErrDiskFull); // Disk full, Avkon shows note
        }
    
//************************************************************************************        

#endif    

    // start copying
    if( iMove )
        {
        User::LeaveIfError(
            iFileMan->Move(anOld,aNew,CFileMan::EOverWrite,iStatus));
        }
    
    if ( !iProgressUpdater->IsActive() )
        {
        // start progress updater
        iProgressUpdater->Start(KSVGTOneSecond,KSVGTOneSecond, 
                    TCallBack(CSVGTFileManager::ProgressUpdate, this));

        }    
    
   
    SetActive();

    return RetVal;
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::StartCopyOperationL
//   Overloaded version with RFile Input.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CSVGTFileManager::StartCopyOperationL( 
    MSVGTUISaveListener* aCallback, RFile& aSrcHandle, const TDesC& aNew )
    {
    TFileName sourceFileName;
    aSrcHandle.FullName( sourceFileName );
    aSrcHandle.Size(iFileSize);
    if ( iMove )
        {
        aSrcHandle.Close();
        return StartCopyOperationL( aCallback, sourceFileName, aNew );   
        }
    else
        {
    
        if (IsActive())
            {
            return KErrInUse;
            }

        iPosition = 0;
        iCancel = EFalse;

        iCallback = aCallback;   
            
        
        // Check disk space
        TParsePtrC parse(aNew);

#ifndef RD_MULTIPLE_DRIVE
        if( parse.Drive().CompareF( PathInfo::MemoryCardRootPath().Left(2) ) == 0 )
            {
            if (SysUtil::MMCSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),iFileSize))
                {       
                return KErrDiskFull; // UI shows note
                }
            }
        else
            {
            if (SysUtil::FFSSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),iFileSize))
                {
                User::Leave(KErrDiskFull); // Phone memory full, Avkon shows note
                }
            }  
#else
//********************** Added the support for multiple drive ************************
    TInt intDrive;
    TChar ch = parse.Drive()[0];
    
    User::LeaveIfError( RFs::CharToDrive(ch,intDrive) );
    
    if( SysUtil::DiskSpaceBelowCriticalLevelL(&(iEikEnv->FsSession()),
                                iFileSize, intDrive ))
        {
        User::Leave( KErrDiskFull); // Disk full, Avkon shows note
        }
    
//************************************************************************************        

#endif 
        
        User::LeaveIfError(
                iFileMan->Copy(aSrcHandle,aNew,CFileMan::EOverWrite,iStatus));
          
        
        if ( !iProgressUpdater->IsActive() )
            {
            // start progress updater
            iProgressUpdater->Start(KSVGTOneSecond,KSVGTOneSecond, 
                        TCallBack(CSVGTFileManager::ProgressUpdate, this));

            } 
        SetActive();
        return KErrNone;

        }       
    }
// -----------------------------------------------------------------------------
// CSVGTFileManager::DoCancel
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTFileManager::CancelCopy()
    {
    iCancel = ETrue;
    iProgressUpdater->Cancel();
    Cancel();
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::SetAllowMove
// -----------------------------------------------------------------------------
//
EXPORT_C void CSVGTFileManager::SetAllowMove( TInt32 aValue )
    {
    iMove = aValue; 
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::RunL
// -----------------------------------------------------------------------------
//
void CSVGTFileManager::RunL() 
    {   
    iProgressUpdater->Cancel();

    if (iCallback)
        {
        if (iCancel)
            {
            iCallback->SVGTSavingDoneL(KErrCancel);
            }
        else
            {
            iCallback->SVGTSavingDoneL(iStatus.Int());
            }
        }
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::NotifyFileManOperation
// -----------------------------------------------------------------------------
//
MFileManObserver::TControl CSVGTFileManager::NotifyFileManOperation()
    {
    if (iCancel)
        {
        return MFileManObserver::ECancel;
        }
#ifdef SVGTVIEWERAPP_DBG_FLAG    
    TBuf<40> msg;
    _LIT( errorMsg, "NotifyFileManOperation %d");
    msg.Format(errorMsg, iFileMan->BytesTransferredByCopyStep() );
    PrintDebugMsg( msg );
#endif    
    iPosition += iFileMan->BytesTransferredByCopyStep();
    return MFileManObserver::EContinue;
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::DoCancel
// -----------------------------------------------------------------------------
//
void CSVGTFileManager::DoCancel()
    {
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::ProgressUpdate
// -----------------------------------------------------------------------------
//
TInt CSVGTFileManager::ProgressUpdate(TAny* aPtr)
    {
    if ( aPtr )
        {
        static_cast<CSVGTFileManager*>(aPtr)->DoProgressUpdate();        
        }
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSVGTFileManager::DoProgressUpdate
// -----------------------------------------------------------------------------
//
void CSVGTFileManager::DoProgressUpdate()
    {
    if (iCallback)
        {    
        iCallback->SVGTPositionChanged((iPosition*KSVGTPercent)/iFileSize);
        }
    }
#ifdef SVGTVIEWERAPP_DBG_FLAG    
// -----------------------------------------------------------------------------
// CSVGTFileManager::PrintDebugMsg
// -----------------------------------------------------------------------------
//
void CSVGTFileManager::PrintDebugMsg( const TDesC&  aMsg )
    {
#ifdef _DEBUG        
    RFileLogger::Write( KFileLoggingDir, KFileLog, 
                        EFileLoggingModeAppend, aMsg );
#endif    
    }
#endif    

//  End of File  
