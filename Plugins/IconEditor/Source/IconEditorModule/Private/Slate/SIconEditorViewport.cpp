// Copyright Epic Games, Inc. All Rights Reserved.

#include "SIconEditorViewport.h"
#include "IconEditorViewportClient.h"
#include "SIconEditorViewportToolbar.h"
#include "IconEditorToolkit.h"
#include "Framework/Application/SlateApplication.h"
#include <UnrealClient.h>

#define LOCTEXT_NAMESPACE "IconEditorViewport"

void SIconEditorViewport::Construct(const FArguments& InArgs, const TSharedRef<FIconEditorToolkit>& InAssetEditorToolkit,FAdvancedPreviewScene* InPreviewScene)
{
	PreviewScene = InPreviewScene;
	AssetEditorToolkitPtr = InAssetEditorToolkit;


	SEditorViewport::Construct(
		SEditorViewport::FArguments()
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
	);

	// restore last used feature level
	UWorld* World = PreviewScene->GetWorld();
	//if (World != nullptr)
	//{
	//	World->ChangeFeatureLevel(GWorld->FeatureLevel);
	//}


}

TSharedRef<FEditorViewportClient> SIconEditorViewport::MakeEditorViewportClient()
{
	ViewportClient = StaticCastSharedPtr<FIconEditorViewportClient>(AssetEditorToolkitPtr.Pin()->CreateEditorViewportClient());
	ViewportClient->SetCaptureComponent();
	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SIconEditorViewport::MakeViewportToolbar()
{
	//return TSharedPtr<SWidget>(nullptr);
	return SAssignNew(ViewportToolbar, SIconEditorViewportToolBar, SharedThis(this));
}

TSharedRef<SEditorViewport> SIconEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SIconEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SIconEditorViewport::OnFloatingButtonClicked()
{
}

FReply SIconEditorViewport::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE