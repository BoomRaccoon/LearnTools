// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class FIconEditorViewportClient;
class FIconEditorToolkit;
class SIconEditorViewportToolBar;
class FAdvancedPreviewScene;


class SIconEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	
	SLATE_BEGIN_ARGS(SIconEditorViewport) {}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs, const TSharedRef<FIconEditorToolkit>& InAssetEditorToolkit,FAdvancedPreviewScene* InPreviewScene);
	virtual ~SIconEditorViewport() override {}

	// ~ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	// ~End of ICommonEditorViewportToolbarInfoProvider interface

	/** Returns the preview scene being renderd in the viewport */
	FAdvancedPreviewScene* GetPreviewScene() { return PreviewScene; }


	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

protected:

	// ~SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	// ~End of SEditorViewport interface

	/** The viewport toolbar */
	TSharedPtr<SIconEditorViewportToolBar> ViewportToolbar;

	/** Viewport client */
	TSharedPtr<FIconEditorViewportClient> ViewportClient;

	/** The preview scene that we are viewing */
	FAdvancedPreviewScene* PreviewScene;

	/** Asset editor toolkit we are embedded in */
	TWeakPtr<FIconEditorToolkit> AssetEditorToolkitPtr;
};
