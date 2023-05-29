// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "AdvancedPreviewScene.h" // IWYU pragma: keep
#include "Tools/BaseAssetToolkit.h"
#include <AdvancedPreviewSceneModule.h>
#include <Templates/SharedPointer.h>
#include "IconEditorViewportClient.h"
#include <SceneOutliner/Public/ISceneOutlinerTreeItem.h>



class FAdvancedPreviewScene;
class FSpawnTabArgs;

class FEditorViewportClient;
class UAssetEditor;
class SIconEditorViewport;
class UTextureRenderTarget2D;
class UIconEditor;
class STransformGizmoNumericalUIOverlay;
class SSceneOutliner;

extern UIconEditorSettings* IconEditorSettings;

class FIconEditorToolkit : public FBaseAssetToolkit, public FGCObject
{
	friend class FIconEditorModule;
	friend class FIconEditorViewportClient;
	friend class UIconEditor;


public:
	explicit FIconEditorToolkit(UAssetEditor* InOwningAssetEditor);

	virtual TSharedPtr<FEditorViewportClient> CreateEditorViewportClient() const override;

#pragma region ToolkitInterface
	FName GetEditorName() const override { return FName("IconEditor"); };
	FName GetToolkitFName() const override { return FName("Icon Editor"); };
	FText GetToolkitName() const override { return FText::FromString("Icon Editor"); };
	FText GetToolkitToolTipText() const override { return FText::FromString("Editor to create icons from static meshes"); };
#pragma endregion ToolkitInterface

#pragma region GCInterface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override {} ;
	virtual FString GetReferencerName() const override { return TEXT("FIconEditorToolkit"); };
#pragma endregion GCInterface


	void SetMeshLocation(FVector::FReal NewValue, EAxis::Type Axis);
	void SetMeshLocation(FVector::FReal NewValue, ETextCommit::Type);

	void AddViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget) override;



protected:

	int32 width = IconEditorSettings->DefaultIconResolution.X;
	int32 height = IconEditorSettings->DefaultIconResolution.Y;
	TSharedPtr<class SNumericEntryBox<int32>> heightBox;
	TSharedPtr<class SNumericEntryBox<int32>> widthBox;

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	
	virtual void PostInitAssetEditor() override;

#pragma region AssetListFunctions
	void AddAssetToList(const FAssetData& InAssetData);
	void AddAssetToList(TArray<FAssetData> InAssetData);
	FReply RemoveAssetFromList(TSharedPtr<FAssetData> Asset);
	FReply ClearAssetList();
	void WidthChange(int32 InValue);

	TOptional<int32> GetWidth() const;
	TOptional<int32> GetHeight() const;
#pragma endregion AssetListFunctions
	
#pragma region ButtonFunctions
	FReply RenderCurrent();
	FReply ModeManagerButton();
	FReply TestButton1();
	FReply TestButton2();
	FReply TestButton3();
	FReply TestButton4();
	FReply SetLocation();
	FReply AddAdditionalActor();
#pragma endregion ButtonFunctions


	void OnClose() override;


private:
	/** Callback to detect changes in number of slot to keep gizmos in sync. */
	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent) const;

	
	TSharedRef<ITableRow> GenerateItemRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnListAssetChanged(TSharedPtr<FAssetData> SelectedAsset, ESelectInfo::Type SelectInfo);
	void OnAssetSelected(const FAssetData& AssetData);

	//TOptional<FVector::FReal> GetMeshLocation(EAxis::Type Axis) const { return IconEditorViewportClient->PreviewMeshComponent->GetComponentLocation()[Axis-1]; } ;
	 
	TSharedPtr<FUICommandList> PluginCommandList;

	TSharedPtr<STransformGizmoNumericalUIOverlay> GizmoNumericalUIOverlayWidget;


#pragma region TabSetup
	/** NAMES */
	static const FName PreviewSettingsTabID;
	static const FName SceneViewportTabID;
	static const FName AssetListTabID;
	static const FName PreviewSceneSettingsTabId;
	static const FName SettingsViewTabID;
	static const FName ActionButtonsTabID;
	
	/** SPAWNERS FUNCTIONS */
	TSharedRef<SDockTab> SpawnTab_PreviewSettings(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_SceneViewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_AssetList(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_PreviewSceneSettings(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_SettingsViewTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_ActionButtonsTab(const FSpawnTabArgs& Args);

	TSharedPtr<class IDetailsView> SettingsView;
	void OnActorPicked(AActor* PickedActor);
	UFUNCTION()
	void OnOutlinerChange(TSharedPtr<ISceneOutlinerTreeItem> OutlinerSelection, ESelectInfo::Type Type);

#pragma endregion TabSetup


	UIconEditor* MyEditor = nullptr;
	
	
	/** Scene in which the 3D preview of the asset lives. */
	TSharedPtr<FAdvancedPreviewScene> AdvancedPreviewScene;
	TSharedPtr<SIconEditorViewport> PreviewViewport = nullptr;
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetList;
	TSharedPtr<SObjectPropertyEntryBox> AssetSelectionBox;
	TSharedPtr<SSceneOutliner> Outliner;
	//TSharedPtr<ISceneOutliner> PickerOutliner;

	TSharedPtr<SWidget> AdvancedPreviewSettingsWidget;

	FAdvancedPreviewSceneModule::FOnPreviewSceneChanged OnPreviewSceneChangedDelegate;
	
	//UTextureRenderTarget2D RenderTexture;

    /** Typed pointer to the custom ViewportClient created by the toolkit. */
	mutable TSharedPtr<class FIconEditorViewportClient> IconEditorViewportClient;

	/** Object path of a mesh selected from the content to spawn a preview in the scene. */
	FString PreviewMeshObjectPath;

	/** Path of the static mesh used for previewing the definition in the asset editor. */
	/*UPROPERTY()
	FSoftObjectPath PreviewMeshPath;*/

};
