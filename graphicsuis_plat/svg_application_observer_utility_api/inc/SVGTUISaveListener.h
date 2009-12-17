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
* Description:  Callback class for UI save methods 
*
*/


#ifndef SVGTUISAVELISTENER_H
#define SVGTUISAVELISTENER_H
        
// CLASS DECLARATION

/**
*  MSVGTUISaveListener 
*  Callback class for UI save methods. File Manager will
*  use this callback to indicate the save status to the
*  SVGT Dialog. 
*
*  @lib SVGTUIControl.lib
*  @since 3.0
*/
NONSHARABLE_CLASS(MSVGTUISaveListener) 
    {

    public: // New functions
        
         /**
        * Called when buffering or saving position is changed
        * @since 3.0
        * @param  aPosition, New buffering/saving position (0-100)
        * @return void
        */
        virtual void SVGTPositionChanged( TInt aPosition ) = 0;
 
        /**
        * Called when saving is finished.
        * @since 3.0
        * @param  aError, KErrNone or one of the system wide error codes.
        * @return void
        */
        virtual void SVGTSavingDoneL( TInt aError ) = 0;
    
    };

#endif      // SVGTUISAVELISTENER_H  
            
// End of File
