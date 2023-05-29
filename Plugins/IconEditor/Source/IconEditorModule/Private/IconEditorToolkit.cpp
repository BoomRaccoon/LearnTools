// Copyright Epic Games, Inc. All Rights Reserved.

#include "IconEditorToolkit.h"
#include "AssetEditorModeManager.h"
#include "Engine/StaticMesh.h"
#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"
#include "IconEditorViewportClient.h"
#include "Viewports.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"
#include "Slate/SIconEditorViewport.h"
#include <Runtime/Slate/Public/Widgets/Layout/SScrollBox.h>
#include <Editor/UnrealEd/Public/EditorViewportTabContent.h>
#include <Editor/AdvancedPreviewScene/Public/AdvancedPreviewSceneModule.h>
#include <Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h>
#include <Toolkits/ToolkitManager.h>
#include <Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h>
#include <../Private/ComponentTransformDetails.h>
#include <Widgets/Input/SVectorInputBox.h>
#include <Selection.h>
#include <EditorViewportCommands.h>
#include <../Plugins/Experimental/MeshModelingToolsetExp/Source/ModelingEditorUI/Public/STransformGizmoNumericalUIOverlay.h>
#include <Widgets/Input/SNumericEntryBox.h>
#include <SceneOutliner/Public/SSceneOutliner.h>
#include <SceneOutliner/Public/ActorBrowsingMode.h>
#include <SceneOutliner/Public/ActorBrowsingMode.h>
#include <SceneOutliner/Public/SceneOutlinerPublicTypes.h>
#include <SceneOutliner/Public/SceneOutlinerPublicTypes.h>
#include <SceneOutliner/Public/SSceneOutliner.h>
#include <SceneOutliner/Public/ActorTreeItem.h>
#include <SceneOutliner/Public/SceneOutlinerModule.h>
#include <Widgets/Notifications/SNotificationList.h>
#include <Framework/Notifications/NotificationManager.h>
#include <HAL/FileManagerGeneric.h>
#include <Kismet/GameplayStatics.h>
#include <Engine/LevelStreamingDynamic.h>
#include <AssetRegistry/AssetRegistryModule.h>
#include <UObject/SavePackage.h>
#include <Engine/LevelScriptActor.h>


#define LOCTEXT_NAMESPACE "IconEditorToolkit"


#pragma region ListFunctions
TSharedRef<ITableRow> FIconEditorToolkit::GenerateItemRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SSpacer)
			]
			+SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(FText::FromName(Item->AssetName))
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.OnClicked(this, &FIconEditorToolkit::RemoveAssetFromList, Item)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Delete"))
				]
			]
		];	
}

void FIconEditorToolkit::OnListAssetChanged(TSharedPtr<FAssetData> SelectedAsset, ESelectInfo::Type SelectInfo)
{
	if (SelectInfo != ESelectInfo::Direct)
	{
		if ( !MyEditor->AssetDataToStreamingLevel.Find(SelectedAsset) )
		{
			FActorSpawnParameters SpawnParams;
			FString aNameConcatWithID = SelectedAsset->GetAsset()->GetName() + FString::FromInt(IconEditorViewportClient->creationNumber);
			SpawnParams.Name = FName(aNameConcatWithID);
			SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
			

			FString ConentDirPath = FPaths::ProjectContentDir();
			FString AssetName = "World_" + FString::FromInt(IconEditorViewportClient->creationNumber);
			FString PackageName = TEXT("/" + AssetName);

			UPackage* MyPackage = CreatePackage(*PackageName);	
			LoadPackage(MyPackage, *PackageName, LOAD_EditorOnly);
			UWorld* NewWorld = /*= UWorld::CreateWorld(EWorldType::Editor, true);*/
			NewObject<UWorld>(MyPackage, *AssetName, RF_Public | RF_Standalone);
			NewWorld->WorldType = EWorldType::Editor;
			NewWorld->InitializeNewWorld();

			TSoftObjectPtr<UWorld> NWPtr = NewWorld;
			FLatentActionInfo LatentInfo;
			ULevelStreamingDynamic* StreamingLevel = NewObject<ULevelStreamingDynamic>(IconEditorViewportClient->GetWorld(), FName(PackageName));
			StreamingLevel->SetWorldAsset(NWPtr);
			StreamingLevel->SetShouldBeLoaded(true);
			StreamingLevel->bInitiallyLoaded = true;

			SpawnParams.Owner = Cast<AActor>(StreamingLevel->GetLevelScriptActor());
			IconEditorViewportClient->SelectedActor = NewWorld->SpawnActor<AIconEditorActor>(SpawnParams);
			IconEditorViewportClient->SelectedActor->StaticMeshComponent->SetStaticMesh(Cast<UStaticMesh>(SelectedAsset->GetAsset()));
			IconEditorViewportClient->SelectedActor->SetActorLabel(Cast<UStaticMesh>(SelectedAsset->GetAsset())->GetName(), false);

			MyEditor->AssetDataToStreamingLevel.Add(SelectedAsset, StreamingLevel);
			IconEditorViewportClient->GetWorld()->AddStreamingLevel(StreamingLevel);
			ULevelStreaming* LastLevel = nullptr;
			for (ULevelStreaming* Current : IconEditorViewportClient->GetWorld()->GetStreamingLevels())
			{
				if(Current->GetLevelStreamingState() == ELevelStreamingState::LoadedVisible)
					LastLevel = Current;

			}
			if (  LastLevel )
			{
				//IconEditorViewportClient->GetWorld()->RemoveStreamingLevel(LastLevel);
				FLatentActionInfo LInfo;
				LInfo.UUID = FMath::Rand();
				LastLevel->SetShouldBeVisible(false);
				UGameplayStatics::UnloadStreamLevel(IconEditorViewportClient->GetWorld(), LastLevel->GetWorldAssetPackageFName(), LInfo, true);
			}
			UGameplayStatics::LoadStreamLevelBySoftObjectPtr(IconEditorViewportClient->GetWorld(), StreamingLevel->GetWorldAsset(), true, true, {});
			
			IconEditorViewportClient->GetWorld()->RefreshStreamingLevels();

			//FAssetRegistryModule::AssetCreated(NewWorld);
			//NewWorld->MarkPackageDirty();
			//FString FilePath = ConentDirPath + PackageName.RightChop(1) + FPackageName::GetAssetPackageExtension();
			//FSavePackageArgs SaveArgs;
			//SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			//bool bSuccess = UPackage::SavePackage(MyPackage, NewWorld, *FilePath, SaveArgs);


			IconEditorViewportClient->creationNumber++;
		}
		else
		{
			ULevelStreaming* LastLevel = *MyEditor->AssetDataToStreamingLevel.Find(AssetList->GetSelectedItems()[0]);
			ULevelStreaming* NewLevel = *MyEditor->AssetDataToStreamingLevel.Find(SelectedAsset);
			UGameplayStatics::LoadStreamLevelBySoftObjectPtr(IconEditorViewportClient->GetWorld(), NewLevel, true, true, {});
			//IconEditorViewportClient->GetWorld()->RemoveStreamingLevel(LastLevel);
			LastLevel->SetShouldBeVisible(false);
			UGameplayStatics::UnloadStreamLevelBySoftObjectPtr(IconEditorViewportClient->GetWorld(), LastLevel->GetWorldAsset(), {}, true);
			IconEditorViewportClient->GetWorld()->RefreshStreamingLevels( {LastLevel, NewLevel} );
			
		}

		IconEditorViewportClient->Viewport->InvalidateHitProxy();
	}
}

void FIconEditorToolkit::OnAssetSelected(const FAssetData& InAssetData)
{
	MyEditor->IconEditorObject->AssetsArray.AddUnique(InAssetData);
	AddAssetToList(MyEditor->IconEditorObject->AssetsArray.Last());
}

void FIconEditorToolkit::AddAssetToList(const FAssetData& InAssetData)
{
	if (IconEditorViewportClient->PreviewMeshComponent == nullptr)
		IconEditorViewportClient->SetPreviewMesh(Cast<UStaticMesh>(InAssetData.GetAsset()));
	MyEditor->IconEditorObject->AssetsArray.AddUnique(InAssetData);
	if (MyEditor->IconEditorObject->AssetsArray.Num() != MyEditor->AssetsSharedPtrArray.Num())
	{
		MyEditor->AssetsSharedPtrArray.AddUnique(MakeShared<FAssetData>(MyEditor->IconEditorObject->AssetsArray.Last()));
		AssetList->RequestListRefresh();
	}

}

void FIconEditorToolkit::AddAssetToList(TArray<FAssetData> InAssetData)
{
	for (FAssetData CurrentAsset : InAssetData)
	{
		MyEditor->IconEditorObject->AssetsArray.AddUnique(CurrentAsset);
		if(MyEditor->IconEditorObject->AssetsArray.Num() != MyEditor->AssetsSharedPtrArray.Num())
			MyEditor->AssetsSharedPtrArray.AddUnique(MakeShared<FAssetData>(MyEditor->IconEditorObject->AssetsArray.Last()));
	}
	AssetList->RequestListRefresh();

	/*if(IconEditorViewportClient->PreviewMeshComponent == nullptr)
	{
		IconEditorViewportClient->SetPreviewMesh(Cast<UStaticMesh>(InAssetData[0].GetAsset()));
		AssetList->SetSelection(MyEditor->AssetsSharedPtrArray[0]);
	}*/



	for (UObject* CurrentObject : GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->GetAllEditedAssets())
	{
		if (GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(CurrentObject, false)->GetEditorName().IsEqual(FName("IconEditor")))
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(CurrentObject, true);
			break;
		}
	}

}

FReply FIconEditorToolkit::RemoveAssetFromList(TSharedPtr<FAssetData> Asset)
{
	bool bSelectedItemRemoved = true;
	TSharedPtr<FAssetData> SelectedRow;
	
	if(AssetList->GetSelectedItems().Num() > 0)
		SelectedRow = AssetList->GetSelectedItems()[0];

	
	if (SelectedRow)
		bSelectedItemRemoved = MyEditor->AssetsSharedPtrArray.IndexOfByKey(Asset) == MyEditor->AssetsSharedPtrArray.IndexOfByKey(SelectedRow) ;

	int RemovedAssetIndex = MyEditor->AssetsSharedPtrArray.IndexOfByKey(Asset);
	MyEditor->AssetsSharedPtrArray.Remove(Asset);
	MyEditor->IconEditorObject->AssetsArray.Remove(*Asset.Get());


	if (bSelectedItemRemoved)
	{
		if(RemovedAssetIndex < MyEditor->AssetsSharedPtrArray.Num()) // 
			AssetList->SetSelection(MyEditor->AssetsSharedPtrArray[RemovedAssetIndex]);
		else if(RemovedAssetIndex - 1 >= 0)
			AssetList->SetSelection(MyEditor->AssetsSharedPtrArray[RemovedAssetIndex - 1]);
		else
		{
			IconEditorViewportClient->SetPreviewMesh(nullptr);
			IconEditorViewportClient->WidgetMode = UE::Widget::WM_None;
		}
	}

	AssetList->RequestListRefresh();
	if(IconEditorViewportClient->SelectedActor->StaticMeshComponent->GetStaticMesh() != nullptr)
		IconEditorViewportClient->WidgetMode = UE::Widget::WM_Translate;
	else
		IconEditorViewportClient->SetPreviewMesh(nullptr);

	return FReply::Handled();
}

FReply FIconEditorToolkit::ClearAssetList()
{
	;
	//IconEditorViewportClient->PreviewScene->RemoveComponent(IconEditorViewportClient.Get()->PreviewMeshComponent.Get());
	
	IconEditorViewportClient->GetWorld()->ClearStreamingLevels();
	
	MyEditor->AssetsSharedPtrArray.Empty();
	MyEditor->IconEditorObject->AssetsArray.Empty();

	AssetList->RequestListRefresh();

	return FReply::Handled();
	
}
#pragma endregion ListFunctions


#pragma region TABSETUP

void FIconEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FBaseAssetToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(ViewportTabID);

	InTabManager->RegisterTabSpawner(SceneViewportTabID, FOnSpawnTab::CreateSP(this, &FIconEditorToolkit::SpawnTab_SceneViewport))
		.SetDisplayName(LOCTEXT("Viewport", "Viewport"))
		.SetGroup(AssetEditorTabsCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(AssetListTabID, FOnSpawnTab::CreateSP(this, &FIconEditorToolkit::SpawnTab_AssetList))
		.SetDisplayName(LOCTEXT("AssetList", "Asset List"))
		.SetGroup(AssetEditorTabsCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Visibility"));

	InTabManager->RegisterTabSpawner(PreviewSceneSettingsTabId, FOnSpawnTab::CreateSP(this, &FIconEditorToolkit::SpawnTab_PreviewSceneSettings))
		.SetDisplayName(LOCTEXT("PreviewSceneTab", "Preview Scene Settings"))
		.SetGroup(AssetEditorTabsCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(SettingsViewTabID, FOnSpawnTab::CreateSP(this, &FIconEditorToolkit::SpawnTab_SettingsViewTab))
		.SetDisplayName(LOCTEXT("SettingsTab", "Default Settings"))
		.SetGroup(AssetEditorTabsCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
	
	InTabManager->RegisterTabSpawner(ActionButtonsTabID, FOnSpawnTab::CreateSP(this, &FIconEditorToolkit::SpawnTab_ActionButtonsTab))
		.SetDisplayName(LOCTEXT("ActionButtons", "Main Editor Functions"))
		.SetGroup(AssetEditorTabsCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FIconEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InbTabManager)
{
	InbTabManager->UnregisterTabSpawner(AssetListTabID);
	InbTabManager->UnregisterTabSpawner(PreviewSceneSettingsTabId);
	InbTabManager->UnregisterTabSpawner(SceneViewportTabID);
	InbTabManager->UnregisterTabSpawner(SettingsViewTabID);
}

TSharedRef<SDockTab> FIconEditorToolkit::SpawnTab_SceneViewport(const FSpawnTabArgs& Args)
{

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab).Label(LOCTEXT("ViewportTab_Title", "Viewport"));

	PreviewViewport = SNew(SIconEditorViewport, StaticCastSharedRef<FIconEditorToolkit>(AsShared()), AdvancedPreviewScene.Get());

	SpawnedTab->SetContent(PreviewViewport.ToSharedRef());
	//ViewportTabContent->Initialize(ViewportDelegate, SpawnedTab, LayoutId);

	return SpawnedTab;
}


TSharedRef<SDockTab> FIconEditorToolkit::SpawnTab_SettingsViewTab(const FSpawnTabArgs& Args)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs SettingsViewArgs;
	SettingsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	SettingsViewArgs.bHideSelectionTip = true;
	SettingsView = PropertyEditorModule.CreateDetailView(SettingsViewArgs);
	SettingsView->SetObject(IconEditorSettings);

	TSharedPtr<SDockTab> SettingnsTab = SNew(SDockTab)
		.Label(LOCTEXT("BaseDetailsTitle", "Details"))
		[
			SettingsView.ToSharedRef()
		];

	return SettingnsTab.ToSharedRef();

}

TSharedRef<SDockTab> FIconEditorToolkit::SpawnTab_ActionButtonsTab(const FSpawnTabArgs& Args)
{
	TSharedPtr<SDockTab> SettingnsTab = SNew(SDockTab)
		.Label(LOCTEXT("ActionsButtons", ""))
		[

#pragma region ButtonRowBottom
#pragma region Resolution
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Width: "))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(widthBox, SNumericEntryBox<int32>)
				.Value_Lambda([this]() {return width; })
				.MinValue(32)
				.MaxValue(1024)
				.MaxSliderValue(1024)
				.AllowSpin(true)
				.OnValueChanged_Lambda([this](int32 InValue){width = InValue;})
				.OnValueCommitted_Lambda([this](const int32& InValue, ETextCommit::Type CommitType)
				{
					width = InValue;
					IconEditorViewportClient->UpdateTextureTarget(width, height);
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(10.0f, 0.0f, 0.0f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Height: "))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([this]() {return height; })
				.MaxSliderValue(1024)
				.MinValue(32)
				.MaxValue(1024)
				.AllowSpin(true)
				.OnValueChanged_Lambda([this](int32 InValue){height = InValue;})
				.OnValueCommitted_Lambda([this](const int32& InValue, ETextCommit::Type CommitType)
				{
					height = InValue;
					IconEditorViewportClient->UpdateTextureTarget(width, height);
					UE_LOG(LogTemp, Display, TEXT("Height: %u"), height);
				})
				.OnUndeterminedValueCommitted_Lambda([this](FText InText, ETextCommit::Type CommitType)
				{
					UE_LOG(LogTemp, Display, TEXT("Undetermined value provided (%s)"), *InText.ToString());
				})
			]
		]
#pragma endregion Resolution


#pragma region TestButtons
	+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Text(FText::FromString("Take simple screenshot"))
		.OnClicked(this, &FIconEditorToolkit::TestButton1)
		]

	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Text(FText::FromString("Test2"))
		.OnClicked(this, &FIconEditorToolkit::TestButton2)
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Text(FText::FromString("Test3"))
		.OnClicked(this, &FIconEditorToolkit::TestButton3)
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Text(FText::FromString("Test4"))
		.OnClicked(this, &FIconEditorToolkit::TestButton4)
		]
		]
#pragma endregion TestButtons

	+ SVerticalBox::Slot()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Text(FText::FromString("Add additional actor"))
			.OnClicked(this, &FIconEditorToolkit::AddAdditionalActor)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Text(FText::FromString("Clear asset list"))
			.OnClicked(this, &FIconEditorToolkit::ClearAssetList)
		]
	]

	+ SVerticalBox::Slot()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Text(FText::FromString("Icon from current"))
			.OnClicked(this, &FIconEditorToolkit::RenderCurrent)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(FText::FromString("Icon for all"))
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
		]
	]
#pragma endregion ButtonRowBottom
		
		];

	return SettingnsTab.ToSharedRef();

}



TSharedRef<SDockTab> FIconEditorToolkit::SpawnTab_PreviewSceneSettings(const FSpawnTabArgs& Args)
{
	TSharedRef<SWidget> InWidget = SNullWidget::NullWidget;
	if (PreviewViewport.IsValid())
	{
		FAdvancedPreviewSceneModule& AdvancedPreviewSceneModule = FModuleManager::LoadModuleChecked<FAdvancedPreviewSceneModule>("AdvancedPreviewScene");
		InWidget = AdvancedPreviewSceneModule.CreateAdvancedPreviewSceneSettingsWidget(AdvancedPreviewScene.ToSharedRef(), IconEditorViewportClient->PreviewMeshComponent.Get(), TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>(), TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>());
	}

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("PreviewSceneSettingsTab", "Preview Scene Settings"))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SBox)
				[ InWidget ]
			]
			//+ SVerticalBox::Slot()
			//[
			//	SNew(SScrollBox)
			//	+ SScrollBox::Slot()
			//	[
			//		StructureDetailsView->GetWidget().ToSharedRef()
			//	]
			//]

			//+SVerticalBox::Slot()
			//[
			//	SNew(SSceneOutliner, FSceneOutlinerInitializationOptions())

			//]
		];
	//DetailsView->SetObject(IconEditorViewportClient->SelectedActor->StaticMeshComponent.Get());

	return SpawnedTab;
}

void FIconEditorToolkit::OnActorPicked(AActor * PickedActor)
{
	IconEditorViewportClient->SelectedActor = Cast<AIconEditorActor>(PickedActor);
}


TSharedRef<SDockTab> FIconEditorToolkit::SpawnTab_AssetList(const FSpawnTabArgs& Args)
{

	FCreateSceneOutlinerMode MF = FCreateSceneOutlinerMode::CreateLambda([this](SSceneOutliner* Outliner)
		{
			FActorModeParams Params;
			Params.SceneOutliner = Outliner;
			Params.SpecifiedWorldToDisplay = IconEditorViewportClient->GetWorld();
			Params.bHideEmptyFolders = true;
			return new FActorMode(Params);
		});

	
	FSceneOutlinerInitializationOptions SceneOptions{};
	SceneOptions.ModeFactory = MF;
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");


	const TSharedPtr<SDockTab> AssetListTab = SNew(SDockTab)
		.Label(LOCTEXT("AssetListTitle", "Asset List"))
		.ShouldAutosize(false)
		[
			SNew(SVerticalBox)
#pragma region Heading
			+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString("List of Assets"))
			]
#pragma endregion Heading
#pragma region AssetScrollList
			+SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SAssignNew(Outliner, SSceneOutliner, SceneOptions)
					//.OnOutlinerTreeSelectionChanged(this, &FIconEditorToolkit::OnOutlinerSelectionChanged)
				]
				+SHorizontalBox::Slot()
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(AssetList, SListView<TSharedPtr<FAssetData>>)
						.ItemHeight(24)
						.ListItemsSource(&MyEditor->AssetsSharedPtrArray)
						.OnGenerateRow(this, &FIconEditorToolkit::GenerateItemRow)
						.OnSelectionChanged(this, &FIconEditorToolkit::OnListAssetChanged)
					]
				]

			]
#pragma endregion AssetScrollList
			+ SVerticalBox::Slot()
			.FillHeight(.1f)
			[
				SNew(SHorizontalBox)
//				+ SHorizontalBox::Slot()
//				.FillWidth(0.3f)
//				[
//					SNew(SButton)
////					.OnClicked(this, &FIconEditorToolkit::AddAssetToList)
//					[
//						SNew(SImage)
//						.Image(FAppStyle::GetBrush("Icons.Add"))
//					]
//				]
				+ SHorizontalBox::Slot()
				[
					SAssignNew(AssetSelectionBox, SObjectPropertyEntryBox)
					.AllowedClass(UStaticMesh::StaticClass())
					.DisplayCompactSize(true)
					.DisplayThumbnail(false)
					.DisplayBrowse(false)
					.OnObjectChanged(this, &FIconEditorToolkit::OnAssetSelected)
				]
			]
		];

		Outliner->GetOnItemSelectionChanged().AddRaw(this, &FIconEditorToolkit::OnOutlinerChange);

		return AssetListTab.ToSharedRef();
}

#pragma endregion TABSETUP

//----------------------------------------------------------------------//
// FIconEditorToolkit
//----------------------------------------------------------------------//
FIconEditorToolkit::FIconEditorToolkit(UAssetEditor* InOwningAssetEditor)
	: FBaseAssetToolkit(InOwningAssetEditor)
{
	FPreviewScene::ConstructionValues PreviewSceneArgs;
	AdvancedPreviewScene = MakeShared<FAdvancedPreviewScene>(PreviewSceneArgs);

	//AdvancedPreviewScene->SetSkyCubemap();
	MyEditor = Cast<UIconEditor>(InOwningAssetEditor);

	// Apply small Z offset to not hide the grid
	constexpr float DefaultFloorOffset = 1.0f;
	AdvancedPreviewScene->SetFloorOffset(DefaultFloorOffset);
	const TSharedRef<FGlobalTabmanager>& GTabManager = FGlobalTabmanager::Get();

#pragma region CreateLayout
// Setup our default layout
	StandaloneDefaultLayout = FTabManager::NewLayout(FName("IconEditorLayout7"))
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(2.0f)
					->AddTab(SceneViewportTabID, ETabState::OpenedTab)
					->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(.4f)
						->AddTab(ActionButtonsTabID, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->AddTab(AssetListTabID, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->AddTab(PreviewSceneSettingsTabId, ETabState::OpenedTab)
						->AddTab(DetailsTabID, ETabState::OpenedTab)
						->SetForegroundTab(DetailsTabID)
					)
				)
			)
		);
#pragma endregion CreateLayout
}

void FIconEditorToolkit::OnOutlinerChange(TSharedPtr<ISceneOutlinerTreeItem> OutlinerSelection, ESelectInfo::Type Type)
{
	if (OutlinerSelection)
	{
		IconEditorViewportClient->SelectedActor = Cast<AIconEditorActor>(OutlinerSelection.Get()->CastTo<FActorTreeItem>()->Actor.Get());
		DetailsView->SetObject(IconEditorViewportClient->SelectedActor->StaticMeshComponent.Get());
		IconEditorViewportClient->WidgetMode = UE::Widget::WM_Translate;
		IconEditorViewportClient->Viewport->InvalidateHitProxy();
	}
}

const FName FIconEditorToolkit::SceneViewportTabID(TEXT("IconEditorToolkit_Viewport"));
const FName FIconEditorToolkit::PreviewSettingsTabID(TEXT("IconEditorToolkit_Preview"));
const FName FIconEditorToolkit::PreviewSceneSettingsTabId(TEXT("IconEditor_PreviewScene"));
const FName FIconEditorToolkit::AssetListTabID(TEXT("IconEditorToolkit_AssetList"));
const FName FIconEditorToolkit::SettingsViewTabID(TEXT("IconEditorToolkit_SettingsView"));
const FName FIconEditorToolkit::ActionButtonsTabID(TEXT("IconEditorToolkit_ActionButtons"));

TSharedPtr<FEditorViewportClient> FIconEditorToolkit::CreateEditorViewportClient() const
{
	// Set our advanced preview scene in the EditorModeManager
	//StaticCastSharedPtr<FAssetEditorModeManager>(EditorModeManager)->SetPreviewScene(AdvancedPreviewScene.Get());

	// Create and setup our custom viewport client
	IconEditorViewportClient = MakeShared<FIconEditorViewportClient>(SharedThis(this), AdvancedPreviewScene.Get());

	IconEditorViewportClient->ViewportType = LVT_Perspective;
	IconEditorViewportClient->SetViewLocation(EditorViewportDefs::DefaultPerspectiveViewLocation);
	IconEditorViewportClient->SetViewRotation(EditorViewportDefs::DefaultPerspectiveViewRotation);

	return IconEditorViewportClient;
}

void FIconEditorToolkit::PostInitAssetEditor()
{
	// Register to be notified when properties are edited
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FIconEditorToolkit::OnPropertyChanged);
	DetailsView->SetObject(IconEditorSettings);
}

void FIconEditorToolkit::SetMeshLocation(FVector::FReal NewValue, EAxis::Type Axis)
{
	UE_LOG(LogTemp, Display, TEXT("Value of %d change"), Axis);
	FVector NewLocation = IconEditorViewportClient->PreviewMeshComponent->GetRelativeLocation();
	NewLocation[Axis-1] = NewValue;
	IconEditorViewportClient->PreviewMeshComponent->SetRelativeLocation(NewLocation);
	
}

void FIconEditorToolkit::SetMeshLocation(FVector::FReal NewValue, ETextCommit::Type)
{
	UE_LOG(LogTemp, Display, TEXT("Value change commited"));
}

FReply FIconEditorToolkit::TestButton1()
{
	//IconEditorViewportClient->SimpleScreenShot();
	// Inform the user of the result of the operation
	FNotificationInfo Info(FText::GetEmpty());
	Info.ExpireDuration = 5.0f;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = false;

	TSharedPtr<SNotificationItem> SaveMessagePtr = FSlateNotificationManager::Get().AddNotification(Info);
	SaveMessagePtr->SetText(NSLOCTEXT("IconEditor", "WeDidSomething", "We did something"));
	SaveMessagePtr->SetCompletionState(SNotificationItem::CS_Success);


	return FReply::Handled();
}

FReply FIconEditorToolkit::TestButton2()
{
	for (AActor* Current : IconEditorViewportClient->GetWorld()->GetLevel(0)->Actors)
	{
		UE_LOG(LogTemp, Display, TEXT("Actor name: %s"), *Current->GetActorNameOrLabel());
	}

	return FReply::Handled();
}


FReply FIconEditorToolkit::TestButton3()
{
	//IconEditorViewportClient->GetWorld()->GetLevel(1)->bIsVisible = false;
	UE_LOG(LogTemp, Display, TEXT("Number of selected Levels: %d"), IconEditorViewportClient->GetWorld()->GetSelectedLevels().Num());
	for (ULevel* Current : IconEditorViewportClient->GetWorld()->GetLevels())
	{
		Current->IsPersistentLevel();
	}

	UWorld* IEWorld = IconEditorViewportClient->GetWorld();

	FLatentActionInfo LInfo;
	LInfo.UUID = FMath::Rand();

	UGameplayStatics::UnloadStreamLevel(IconEditorViewportClient->GetWorld(),IEWorld->GetStreamingLevels().Last()->GetWorldAssetPackageFName(), {}, true);
	IconEditorViewportClient->GetWorld()->RefreshStreamingLevels();

 	return FReply::Handled();
}

FReply FIconEditorToolkit::TestButton4()
{
	for (ULevelStreaming* Current : IconEditorViewportClient->GetWorld()->GetStreamingLevels())
	{
		UE_LOG(LogTemp, Display, TEXT("Current level (%s) states: %d"), *Current->GetName(), Current->GetLevelStreamingState());
	}

	UE_LOG(LogTemp, Display, TEXT("Number of actors: %d"), IconEditorViewportClient->GetWorld()->GetActorCount());

	return FReply::Handled();
}

void TestPrint()
{
	UE_LOG(LogTemp, Display, TEXT("Callback executed"));
}


FReply FIconEditorToolkit::AddAdditionalActor()
{

	TSharedPtr<FAssetData> Curr = AssetList->GetSelectedItems()[0];
	UStaticMesh* CurrMesh = Cast<UStaticMesh>(Curr.Get()->GetAsset());
	
	IconEditorViewportClient->AddAdditionalActor(CurrMesh);

	return FReply::Handled();
}

FReply FIconEditorToolkit::RenderCurrent()
{
	UE_LOG(LogTemp, Display, TEXT("Should have Captured the scene"));
	
	FString Path = FString("Icon3.exr");
	IconEditorViewportClient->SaveIcon(Path);
	return FReply::Handled();
}

FReply FIconEditorToolkit::ModeManagerButton()
{
	USelection* SelectedStuff = EditorModeManager->GetSelectedActors();
	UE_LOG(LogTemp, Display, TEXT("There are %u objects selected")); SelectedStuff->Num();
	for (int i = 0; i < SelectedStuff->Num(); ++i)
	{
		UE_LOG(LogTemp, Display, TEXT("%s"), *SelectedStuff->GetSelectedObject(i)->GetName());
	}	

	return FReply::Handled();
}

void FIconEditorToolkit::AddViewportOverlayWidget(TSharedRef<SWidget> InViewportOverlayWidget)
{
	GizmoNumericalUIOverlayWidget = SNew(STransformGizmoNumericalUIOverlay)
		.DefaultLeftPadding(15)
		// Position above the little axis visualization
		.DefaultVerticalPadding(75)
		.bPositionRelativeToBottom(true);
	FBaseAssetToolkit::AddViewportOverlayWidget(GizmoNumericalUIOverlayWidget.ToSharedRef());

}


void FIconEditorToolkit::OnClose()
{
	UToolMenus::Get()->RemoveEntry("ContentBrowser.AssetContextMenu.StaticMesh", "GetAssetActions", "IconEditor");


	// Extend the asset browser for static meshes
	{
		if (UToolMenu* ToolMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.StaticMesh"))
		{
			FToolMenuSection& Section = ToolMenu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry("IconEditor", FNewToolMenuSectionDelegate::CreateLambda([this](FToolMenuSection& InSection)
				{
					const TAttribute<FText> Label = LOCTEXT("IconEditorEntry", "Open Icon Editor");
					const TAttribute<FText> ToolTip = LOCTEXT("IconEditorEntryToolTip", "Opens an editor for static meshes to create icons");
					const FSlateIcon Icon = FSlateIconFinder::FindIconForClass(UTexture2D::StaticClass());
					const FToolMenuExecuteAction UIAction = FToolMenuExecuteAction::CreateLambda([this](const FToolMenuContext& InMenuContext)
						{
							FModuleManager::Get().LoadModuleChecked<FIconEditorModule>("IconEditorModule").OpenIconEditor();
						});

					InSection.AddMenuEntry("IconEditor", Label, ToolTip, Icon, UIAction);
				}));
		}
	}

	FModuleManager::Get().LoadModuleChecked<FIconEditorModule>("IconEditorModule").ResetIconEditor();

	//AdvancedPreviewScene.Reset();
	PreviewViewport.Reset();
	AssetList.Reset();
	AssetSelectionBox.Reset();
	MyEditor->IconEditorObject->ActorLevelMap.Empty();

	FString EditorDirPath = FPaths::ProjectContentDir() + "z_TempIconsDir/";
	FFileManagerGeneric::Get().DeleteDirectory(*EditorDirPath, false, true);	
	int32 LastError = FPlatformMisc::GetLastError();
	TCHAR ErrorBuffer[1024];
	ErrorBuffer[0] = 0;
	UE_LOG(LogTemp, Error, TEXT("Delete error: %s (%i)"), FPlatformMisc::GetSystemErrorMessage(ErrorBuffer, 1024, LastError), LastError);
	/*for(ULevel* Current : ViewportClient->GetWorld()->GetLevels())
	{
		ViewportClient->GetWorld()->RemoveLevel(Current);
	}*/

	FBaseAssetToolkit::OnClose();
}


void FIconEditorToolkit::OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent) const
{
	/*if (ObjectBeingModified == nullptr || ObjectBeingModified != GetEditingObject())
	{
		return;
	}*/
}

//ENGINE_API void InitCustomFormat(uint32 InSizeX, uint32 InSizeY, EPixelFormat InOverrideFormat, bool bInForceLinearGamma);
//
///** Initializes the render target, the format will be derived from the value of bHDR. */
//ENGINE_API void InitAutoFormat(uint32 InSizeX, uint32 InSizeY);
//
///** Resizes the render target without recreating the FTextureResource.  Will not flush commands unless the render target resource doesnt exist */
//ENGINE_API void ResizeTarget(uint32 InSizeX, uint32 InSizeY);


//AdvancedPreviewSettingsWidget = AdvancedPreviewSceneModule.CreateAdvancedPreviewSceneSettingsWidget(GetStaticMeshViewport()->GetPreviewScene(), nullptr, TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>(), TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>(), Delegates);
//
//if (PreviewSceneDockTab.IsValid())
//{
//	PreviewSceneDockTab.Pin()->SetContent(AdvancedPreviewSettingsWidget.ToSharedRef());
//}



#undef LOCTEXT_NAMESPACE
