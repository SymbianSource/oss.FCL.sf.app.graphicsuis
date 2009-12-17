/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Used to decorate the control on the navi pane
*
*/


//////////////////////////////////////////////////////////////////////////////
//
// -----> CSvgtDecoratorControl(definition)
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SVGTDECORATORCONTROL_H
#define SVGTDECORATORCONTROL_H


#include <eikbctrl.h>
#include <barsread.h>

class CAknStylusPopUpMenu;
class CAknNavigationControlContainer;
class CEikImage;
class CEikLabel;
class CGulIcon;

enum TSvgtDecoratorControlId
    {
    EMuteImageCtrlId,
    ETextCtrlId,
    ELastCtrl
    };
    
class CSvgtDecoratorControl : public CEikBorderedControl
	{
public: //class methods

    /**
    * default constructor.
    */
	CSvgtDecoratorControl();

    /**
    * Twon phase constructor.
    */
	void ConstructL();
	
    /**
    * Destructor.
    */
	~CSvgtDecoratorControl();

    /**
    * Sets the visibility of the mute icon in the navi pane.
    * @since 3.2
    * @param aVolumeMuted whether to show the mute icon in the navi pane (True/False);
    */
    void ShowVolumeMutedIcon( TBool aVolumeMuted );

    /**
    * Sets the progress text in the navi pane.
    * @since 3.2
    * @param aText Progress text;
    */
    void SetTextL( const TDesC& aText);
    
protected:    //From CCoeControl
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

private:    //From CCoeControl
	void         Draw(const TRect& aRect) const;
	
    TInt         CountComponentControls() const;

	CCoeControl* ComponentControl(TInt aIndex) const;
	
	void         SizeChanged();

private:
    CEikImage* iImg;
    CEikLabel* iLabel;
    CGulIcon* iMutedIcon;
    CAknNavigationControlContainer* iNaviPane;
	};


#endif //SVGTDECORATORCONTROL_H