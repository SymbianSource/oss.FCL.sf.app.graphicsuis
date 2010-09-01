/*
* Copyright (c) 2004,2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This header file enlists the application class interface of
*                the SVGT Viewer App
*
*/



#ifndef __SVGTVIEWERAPPAPPLICATION_H__
#define __SVGTVIEWERAPPAPPLICATION_H__

//  INCLUDES
#include <aknapp.h>

// CLASS DECLARATION

/**
*  Application Class for SVGT Viewer.
*  An instance of CSVGTViewerAppApplication is the Application object
*   for the SVGTViewerApp application.
*  @since 3.0
*/

class CSVGTViewerAppApplication : public CAknApplication
    {
    public: // Functions from base classes
        /**
        * From CAknApplication Return the application DLL UID value.
        * @since 3.0
        * @return TUid The UID of this Application/Dll
        */
        TUid AppDllUid() const;

    protected:  // Functions from base classes
        /**
        * From CAknApplication Create a CApaDocument object and return a
        *  pointer to it.
        * @since 3.0
        * @return CApaDocument* - A pointer to the created document
        */
        CApaDocument* CreateDocumentL();
    private:
};
#endif // __SVGTVIEWERAPPAPPLICATION_H__
// End of File
