// Copyright Epic Games, Inc. All Rights Reserved.

#include "SIconEditorViewportToolbar.h"
#include "SIconEditorViewport.h"
#include "PreviewProfileController.h"

#define LOCTEXT_NAMESPACE "IconEditorViewportToolBar"

void SIconEditorViewportToolBar::Construct(const FArguments& InArgs, TSharedPtr<SIconEditorViewport> InViewport)
{
	SCommonEditorViewportToolbarBase::Construct(
		SCommonEditorViewportToolbarBase::FArguments().AddRealtimeButton(false).PreviewProfileController(MakeShared<FPreviewProfileController>()),
		InViewport);
}

#undef LOCTEXT_NAMESPACE