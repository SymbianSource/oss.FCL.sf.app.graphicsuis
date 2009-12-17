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
// -----> CSvgtDecoratorControl (implementation)
//
//////////////////////////////////////////////////////////////////////////////
#include "SvgtDecoratorControl.h"
#include <eikenv.h>

#include <aknnavi.h>
#include <AknsUtils.h>
#include <gulicon.h>
#include <eikspane.h>
#include <avkon.mbg>
#include <avkon.hrh>
#include <eikimage.h>
#include <eiklabel.h>

_LIT(KAvkonMifFileName, "z:\\resource\\apps\\avkon2.mif");

// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::CSvgtDecoratorControl
// default constructor
// -----------------------------------------------------------------------------
CSvgtDecoratorControl::CSvgtDecoratorControl()
	{
	}

// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::ConstructL
// 2nd phase constructor
// -----------------------------------------------------------------------------
void CSvgtDecoratorControl::ConstructL()
	{
	iNaviPane = static_cast<CAknNavigationControlContainer*>
	    ( CEikStatusPaneBase::Current()->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) );
    //Create icon for drawing the pointer
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    TRgb txtColor;
    AknsUtils::GetCachedColor( skin, txtColor, KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG2 );
    
    iMutedIcon = AknsUtils::CreateGulIconL( 
            skin,
            KAknsIIDQgnIndiMupSpeakerMuted,
            KAvkonMifFileName,
            EMbmAvkonQgn_indi_speaker_muted,
            EMbmAvkonQgn_indi_speaker_muted_mask );
    
    iImg = new (ELeave) CEikImage;
    iImg->SetContainerWindowL(*this);
    iImg->SetPictureOwnedExternally(ETrue);
        
    iLabel = new (ELeave) CEikLabel;
    iLabel->SetContainerWindowL(*this);
    iLabel->SetFont(iEikonEnv->DenseFont());
    iLabel->OverrideColorL( EColorLabelText, txtColor );
    iLabel->SetTextL(KNullDesC);
    
    ShowVolumeMutedIcon(EFalse);
    TRect rt(iNaviPane->Rect().iTl, iNaviPane->Rect().Size());
	SetRect(rt);
	
	ActivateL();	
	}

// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::~CSvgtDecoratorControl
// Destructor
// -----------------------------------------------------------------------------
CSvgtDecoratorControl::~CSvgtDecoratorControl()
	{
	delete iImg;
	delete iLabel;

	delete iMutedIcon;
	}



// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::CountComponentControls
// From CCoeControl
// -----------------------------------------------------------------------------
TInt CSvgtDecoratorControl::CountComponentControls() const
	{
	return ELastCtrl;
	}

// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::ComponentControl
// From CCoeControl
// -----------------------------------------------------------------------------
CCoeControl* CSvgtDecoratorControl::ComponentControl(TInt aIndex) const
	{
	CCoeControl* control = NULL;
	switch (aIndex)
	    {
	    case EMuteImageCtrlId:
	        control = static_cast<CCoeControl*>(iImg);
	        break;
	    case ETextCtrlId:
	        control = static_cast<CCoeControl*>(iLabel);
	        break;
	    default:
	        break;    
	    }
	return control;
	}

// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::SizeChanged
// From CCoeControl
// -----------------------------------------------------------------------------
void CSvgtDecoratorControl::SizeChanged()
    {
	CCoeControl::SizeChanged();
	
	const TInt KBitmapWidth = 40;
	const TInt naviWidth = iNaviPane->Rect().Width();
	const TInt naviHeight = iNaviPane->Rect().Height() ;

    AknIconUtils::SetSize( iMutedIcon->Bitmap(), TSize(KBitmapWidth, naviHeight));
    AknIconUtils::SetSize( iMutedIcon->Mask(), TSize(KBitmapWidth, naviHeight));
	
	TInt imgWidth = KBitmapWidth;
	TInt lblWidth = naviWidth - imgWidth;
	
	TPoint imgPos;
	TPoint lblPos(imgWidth, 0);
	
	iImg->SetExtent(imgPos, TSize(imgWidth, naviHeight));
	iLabel->SetExtent(lblPos, TSize(lblWidth, naviHeight));

	}
	

// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::Draw
// From CCoeControl
// -----------------------------------------------------------------------------
void CSvgtDecoratorControl::Draw(const TRect& aRect) const
	{
	CEikBorderedControl::Draw(aRect);
	}


// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::OfferKeyEventL
// From CCoeControl
// -----------------------------------------------------------------------------
TKeyResponse CSvgtDecoratorControl::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	return CCoeControl::OfferKeyEventL(aKeyEvent, aType);//EKeyWasNotConsumed;
	}


// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::SizeChanged
// Sets the visibility of the mute icon in the navi pane.
// -----------------------------------------------------------------------------
void CSvgtDecoratorControl::ShowVolumeMutedIcon( TBool aVolumeMuted )
    {
    if(aVolumeMuted)
        {
        iImg->SetPicture(iMutedIcon->Bitmap(), iMutedIcon->Mask());
        }
    else
        {
        
        iImg->SetPicture(NULL, NULL);
        }    
//    iImg->DrawDeferred()
    }

// -----------------------------------------------------------------------------
// CSvgtDecoratorControl::SetTextL
// Sets the progress text in the navi pane.
// -----------------------------------------------------------------------------
void CSvgtDecoratorControl::SetTextL( const TDesC& aText)
    {
    iLabel->SetTextL( aText );
//    iLabel->DrawDeferred()
    }
