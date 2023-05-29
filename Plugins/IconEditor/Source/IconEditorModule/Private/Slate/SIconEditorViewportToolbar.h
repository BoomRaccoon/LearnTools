// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SCommonEditorViewportToolbarBase.h"

class SIconEditorViewport;

class SIconEditorViewportToolBar : public SCommonEditorViewportToolbarBase
{
public:
	SLATE_BEGIN_ARGS(SIconEditorViewportToolBar)
	{}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SIconEditorViewport> InViewport);
};
