#
# Copyright (c) 2004,2005 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:  This is the makefile for compiling the icons for 
#               SVGTUIControl using data-caging paths.
#

ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=\epoc32\release\$(PLATFORM)\$(CFG)\z
else
ZDIR=\epoc32\data\z
endif

# ----------------------------------------------------------------------------
# Target Path and MIF file name
# ----------------------------------------------------------------------------

TARGETDIR=$(ZDIR)\resource\apps
ICONTARGETFILENAME=$(TARGETDIR)\svgtuicontrol.mif

HEADERDIR=\epoc32\include
HEADERFILENAME=$(HEADERDIR)\svgtuicontrol.mbg

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

# ----------------------------------------------------------------------------
# NOTE 1: DO NOT DEFINE MASK FILE NAMES! They are included automatically by
# MifConv if the mask depth is defined.
#
# NOTE 2: Usually, source paths should not be included in the bitmap
# definitions. MifConv searches for the icons in all icon directories in a
# predefined order, which is currently \s60\icons, \s60\bitmaps2, \s60\bitmaps.
# The directory \s60\icons is included in the search only if the feature flag
# __SCALABLE_ICONS is defined.
# ----------------------------------------------------------------------------

RESOURCE :
	mifconv $(ICONTARGETFILENAME) /h$(HEADERFILENAME)   \
                        /c8,8 qgn_indi_viewer_panning_down  \
                        /c8,8 qgn_indi_viewer_panning_left  \
                        /c8,8 qgn_indi_viewer_panning_right \
                        /c8,8 qgn_indi_viewer_panning_up    \
                        /c8,8 qgn_indi_viewer_pointer       \
                        /c8,8 qgn_indi_viewer_pointer_hand  \
                        /c8,8 qgn_indi_viewer_pointer_text  \
                        /c8,8 qgn_indi_cams_paused         
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(HEADERFILENAME)&& \
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing

# End of file	
