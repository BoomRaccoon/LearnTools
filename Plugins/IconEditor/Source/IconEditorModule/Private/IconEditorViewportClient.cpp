// Copyright Epic Games, Inc. All Rights Reserved.

#include "IconEditorViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "IconEditorToolkit.h"
#include "ScopedTransaction.h"
#include <Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h>
#include <ImageWriteQueue/Public/ImageWriteBlueprintLibrary.h>
#include <Widgets/Notifications/SNotificationList.h>
#include <Framework/Notifications/NotificationManager.h>
#include <HighResScreenshot.h>
#include <ImagePixelData.h>
#include <ImageWriteTask.h>
#include "ImageWriteQueue.h"
#include <HDRHelper.h>
#include <Misc/FileHelper.h>
#include <EngineUtils.h>
#include <DummyViewport.h>
#include <Components/SceneCaptureComponent.h>
#include <Components/PostProcessComponent.h>
#include <Kismet/KismetRenderingLibrary.h>
#include <ILevelEditor.h>
#include <Kismet/GameplayStatics.h>
#include <UObject/SavePackage.h>
#include <AssetRegistry/AssetRegistryModule.h>
#include <Kismet/GameplayStatics.h>
#include <Engine/LevelStreamingDynamic.h>
#include <Engine/LevelScriptActor.h>
#include "IconEditorSettings.h"

#define LOCTEXT_NAMESPACE "IconEditorToolkit"

FIconEditorViewportClient::FIconEditorViewportClient(const TSharedRef<const FIconEditorToolkit>& InAssetEditorToolkit, FPreviewScene* InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewportWidget)
	: FEditorViewportClient(&InAssetEditorToolkit->GetEditorModeManager(), InPreviewScene, InEditorViewportWidget)
	, IconEditorToolkit(InAssetEditorToolkit)
{
	bUsingOrbitCamera = true;
	bIsRealtime = true;
	bCameraLock = false;
	ExposureSettings.bFixed = true;
	ExposureSettings.FixedEV100 = 0;
	WidgetMode = UE::Widget::WM_None;
	// Set if the grid will be drawn
	DrawHelper.bDrawGrid = true;

	GetWorld()->GetWorldSettings()->bEnableWorldComposition = true;

	IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("Streaminglvl.LogOwningWorld"),
			TEXT("You know"),
			FConsoleCommandDelegate::CreateRaw(this, &FIconEditorViewportClient::LogWorld)
		);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Streaminglvl.Log"),
		TEXT("Log all streaming levels with their state"),
		FConsoleCommandDelegate::CreateRaw(this, &FIconEditorViewportClient::GetAllSublevels)
	);


}

void FIconEditorViewportClient::GetAllSublevels()
{
	for (ULevelStreaming* Current : GetWorld()->GetStreamingLevels())
	{
		UE_LOG(LogTemp, Display, TEXT("Streaming level (%s) has state: %d"), *Current->GetName(), Current->GetLevelStreamingState());
	}
}

void FIconEditorViewportClient::LogWorld()
{
	UE_LOG(LogTemp, Display, TEXT("Name of the viewport world: %s"), *GetWorld()->GetName());
}

FIconEditorViewportClient::~FIconEditorViewportClient()
{
	if (ScopedTransaction != nullptr)
	{
		delete ScopedTransaction;
		ScopedTransaction = nullptr;
	}
}

void FIconEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	
	FEditorViewportClient::Draw(View, PDI);
	
	//PDI->DrawLine( FVector(0.f,0.f,0.f), FVector(0.f,0.f, 30.f), FLinearColor::Red, SDPG_Foreground );

	// Draw the object origin.
	//DrawCoordinateSystem(PDI, FVector::ZeroVector, FRotator::ZeroRotator, 20.f, SDPG_World, 1.f);

}


void FIconEditorViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	//FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
}

void FIconEditorViewportClient::SimpleScreenShot()
{

}

void FIconEditorViewportClient::BeginTransaction(FText Text)
{
	if (ScopedTransaction)
	{
		ScopedTransaction->Cancel();
		delete ScopedTransaction;
		ScopedTransaction = nullptr;
	}

	ScopedTransaction = new FScopedTransaction(Text);
	check(ScopedTransaction);
}

void FIconEditorViewportClient::EndTransaction()
{
	if (ScopedTransaction)
	{
		delete ScopedTransaction;
		ScopedTransaction = nullptr;
	}
}

void FIconEditorViewportClient::SetPreviewMesh(UStaticMesh* InStaticMesh)
{
	if(SelectedActor != nullptr)
		SelectedActor->StaticMeshComponent->SetStaticMesh(InStaticMesh);
	else
	{
		FActorSpawnParameters SpawnParams;
		FString aNameConcatWithID = "IconeEditorActor_" +FString::FromInt(creationNumber);
		SpawnParams.Name = FName(aNameConcatWithID);
		creationNumber++;
		SelectedActor = GetWorld()->SpawnActor<AIconEditorActor>(SpawnParams);
		SelectedActor->StaticMeshComponent->SetStaticMesh(InStaticMesh);
	}
	//FocusViewportOnBox(GetPreviewBounds());
}


void FIconEditorViewportClient::SetCaptureComponent()
{
	if (SceneCaptureComponent == nullptr)
	{
		SceneCaptureComponent = NewObject<USceneCaptureComponent2D>();
		SceneCaptureComponent->FOVAngle = 10.f;
		SceneCaptureComponent->bCaptureEveryFrame = false;
		SceneCaptureComponent->bAlwaysPersistRenderingState = true;
		SceneCaptureComponent->ShowFlags.SkyLighting = 1;
		SceneCaptureComponent->CompositeMode = ESceneCaptureCompositeMode::SCCM_Overwrite;
		SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;

		RenderTarget2D = NewObject<UTextureRenderTarget2D>();
		const UIconEditorSettings* IESettings = GetDefault<UIconEditorSettings>();
		RenderTarget2D->InitCustomFormat(IESettings->DefaultIconResolution.X, IESettings->DefaultIconResolution.Y, EPixelFormat::PF_FloatRGBA, true);
		RenderTarget2D->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
		RenderTarget2D->LODGroup = TEXTUREGROUP_UI;

		SceneCaptureComponent->TextureTarget = RenderTarget2D;

		ON_SCOPE_EXIT { PreviewScene->AddComponent(SceneCaptureComponent.Get(), FTransform::Identity); };
	}
}


void FIconEditorViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	FEditorViewportClient::ProcessClick(View, HitProxy, Key, Event, HitX, HitY);

	//if (FSlateApplication::Get().GetPlatformApplication()->GetModifierKeys().IsShiftDown())
	if (HitProxyCast<HActor>(HitProxy))
	{
		SelectedActor = Cast<AIconEditorActor>(HitProxyCast<HActor>(HitProxy)->Actor);
		SelectedActor->IsInPersistentLevel();
		WidgetMode = UE::Widget::WM_Translate;
		
#pragma region UpdateOutlinerSelection
		IconEditorToolkit.Pin().Get()->Outliner->ClearSelection();
		TArray<FSceneOutlinerTreeItemPtr> ActorItems;
		FSceneOutlinerTreeItemPtr ActorItem = IconEditorToolkit.Pin().Get()->Outliner->GetTreeItem(Cast<AActor>(SelectedActor));
		ActorItems.Add(ActorItem);
		IconEditorToolkit.Pin().Get()->Outliner->AddToSelection(ActorItems, ESelectInfo::Direct);
#pragma endregion UpdateOutlinerSelection
		IconEditorToolkit.Pin().Get()->DetailsView->SetObject(SelectedActor->StaticMeshComponent.Get());
	}
	else
		WidgetMode = UE::Widget::WM_None;
}


bool FIconEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	FEditorViewportClient::InputKey(EventArgs);

	if(!IsFlightCameraActive())
	{
		if (EventArgs.Key == EKeys::T)
			WidgetMode = UE::Widget::WM_Translate;
		if (EventArgs.Key == EKeys::R)
			WidgetMode = UE::Widget::WM_Rotate;
		if (EventArgs.Key == EKeys::S)
			WidgetMode = UE::Widget::WM_Scale;
	}
	if(EventArgs.Key == FName("Delete") && EventArgs.Event == IE_Pressed)
	{
		if (SelectedActor->GetLevel()->Actors.Num() == 1)
		{
			ULevel* ActorsLevel = SelectedActor->GetLevel();
			GetWorld()->DestroyActor(SelectedActor);
			int LevelIndex = 0;
			GetWorld()->RemoveLevel(ActorsLevel);
			ActorsLevel->BeginDestroy();

		}
		else

		{
			GetWorld()->DestroyActor(SelectedActor);
		}
		WidgetMode = UE::Widget::WM_None;
		Viewport->InvalidateHitProxy();
	}
	if(EventArgs.Key == FName("LeftMouseButton") && EventArgs.Event == IE_Pressed)
		bDuplicateOnNextDrag = true;
	if(EventArgs.Key == FName("LeftMouseButton") && EventArgs.Event == IE_Released)
		bDuplicateOnNextDrag = false;

	return false;
}


bool FIconEditorViewportClient::InputAxis(FViewport* InViewport, FInputDeviceId DeviceID, FKey Key, float Delta, float DeltaTime, int32 NumSamples /*= 1*/, bool bGamepad /*= false*/)
{
	//ModeTools->InputAxis(InViewport, InViewport, DeviceID.GetId(), Key, Delta, DeltaTime);

	return FEditorViewportClient::InputAxis(InViewport, DeviceID, Key, Delta, DeltaTime, NumSamples, bGamepad);
}

void FIconEditorViewportClient::TrackingStarted(const struct FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge)
{
	bIsManipulating = true;

	// Re-initialize new tracking only if a new button was pressed, otherwise we continue the previous one.
	if (InInputState.GetInputEvent() == IE_Pressed)
	{
		EInputEvent Event = InInputState.GetInputEvent();
		FKey Key = InInputState.GetKey();

		if (InInputState.IsAltButtonPressed() && bDraggingByHandle)
		{
			if (Event == IE_Pressed && (Key == EKeys::LeftMouseButton || Key == EKeys::RightMouseButton) && !bDuplicateActorsInProgress)
			{
				// Set the flag so that the actors actors will be duplicated as soon as the widget is displaced.
				bDuplicateOnNextDrag = true;
				bDuplicateActorsInProgress = true;
			}
		}
		else
		{
			bDuplicateOnNextDrag = false;
		}
	}
	
	//FEditorViewportClient::TrackingStarted(InInputState, bIsDraggingWidget, bNudge);	
}

void FIconEditorViewportClient::TrackingStopped()
{
	bIsManipulating = false;
	FEditorViewportClient::TrackingStopped();
}

bool FIconEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale)
{
	
	//bLockFlightCamera = false; // LOCK CAMERA SO IT DOESN'T MOVE WHILE CHANGING ACTOR TRANSFORMS

	if (IsAltPressed() && !Drag.IsNearlyZero() && bDuplicateOnNextDrag)
	{
		AddAdditionalActor(SelectedActor->StaticMeshComponent->GetStaticMesh());
		bDuplicateOnNextDrag = false;
	}
		

	//ModeTools->InteractiveToolsContext

	const bool LeftMouseButtonDown = InViewport->KeyState(EKeys::LeftMouseButton);
	const bool RightMouseButtonDown = InViewport->KeyState(EKeys::RightMouseButton);
	const bool MiddleMouseButtonDown = InViewport->KeyState(EKeys::MiddleMouseButton);
	if (CurrentAxis != EAxisList::None)
	{
		SelectedActor->SetActorLocation(SelectedActor->GetActorLocation() + Drag);
		SelectedActor->SetActorRotation(SelectedActor->GetActorRotation() + Rot);
		SelectedActor->SetActorScale3D(SelectedActor->GetActorScale3D() + Scale);
	}

	else
		return false;
	
	return true;
}

UE::Widget::EWidgetMode FIconEditorViewportClient::GetWidgetMode() const
{
	bool bIsWidgetValid = false;
	return WidgetMode;
}

bool FIconEditorViewportClient::CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const
{
	return	NewMode == UE::Widget::EWidgetMode::WM_Translate
			|| NewMode == UE::Widget::EWidgetMode::WM_TranslateRotateZ
			|| NewMode == UE::Widget::EWidgetMode::WM_Rotate;
}

void FIconEditorViewportClient::SetWidgetMode(UE::Widget::EWidgetMode NewMode)
{
	FEditorViewportClient::SetWidgetMode(NewMode);
	WidgetMode = NewMode;
}


void FIconEditorViewportClient::SetupRenderTarget()
{

}

void FIconEditorViewportClient::UpdateTextureTarget(int32 width, int32 height)
{	
	if (RenderTarget2D->SizeX != width && RenderTarget2D->SizeY != height)
	{
		RenderTarget2D = NewObject<UTextureRenderTarget2D>();
		RenderTarget2D->InitCustomFormat(width, height, EPixelFormat::PF_FloatRGBA, true);	
		RenderTarget2D->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
		RenderTarget2D->LODGroup = TEXTUREGROUP_UI;

		SceneCaptureComponent->TextureTarget = RenderTarget2D;
		SceneCaptureComponent->TextureTarget->SizeX = width;
		SceneCaptureComponent->TextureTarget->SizeY = height;
		//SceneCaptureComponent->ShowFlags = EngineShowFlags;
	}
	else
		return;
}



void FIconEditorViewportClient::SaveIcon(FString Path)
{
	//// Inform the user of the result of the operation
	//FNotificationInfo Info(FText::GetEmpty());
	//Info.ExpireDuration = 5.0f;
	//Info.bUseSuccessFailIcons = false;
	//Info.bUseLargeFont = false;
	
	FMinimalViewInfo ViewInfo = FMinimalViewInfo();
	ViewInfo.Location = GetViewTransform().GetLocation();
	ViewInfo.Rotation = GetViewTransform().GetRotation();
	SceneCaptureComponent->SetCameraView(ViewInfo);
	
	SceneCaptureComponent->ShowFlags = EngineShowFlags;
	SceneCaptureComponent->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	RenderTarget2D->TargetGamma = 2;

	SceneCaptureComponent->UpdateContent();
	SceneCaptureComponent->CaptureScene();

	FImageWriteOptions WriteOptions;
	WriteOptions.Format = EDesiredImageFormat::EXR;
	WriteOptions.bOverwriteFile = true;
	WriteOptions.bAsync = false;
	WriteOptions.bOverwriteFile = true;

	UKismetRenderingLibrary::ExportRenderTarget(GetWorld(), RenderTarget2D, FPaths::ProjectContentDir(), Path);
}

void FIconEditorViewportClient::AddAdditionalActor(UStaticMesh* ActorMesh, FVector SpawnPosition)
{
	FActorSpawnParameters SpawnParams;
	FString aNameConcatWithID = ActorMesh->GetName() + FString::FromInt(creationNumber);
	const FIconEditorToolkit* ToolkitObj = IconEditorToolkit.Pin().Get();
	SpawnParams.Name = FName(aNameConcatWithID);
	SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
	
	if (!ToolkitObj->MyEditor->AssetDataToStreamingLevel.Find(ToolkitObj->AssetList->GetSelectedItems()[0]))
	{

		FString ConentDirPath = FPaths::ProjectContentDir();
		FString AssetName = "World_" + FString::FromInt(creationNumber);
		FString PackageName = TEXT("/" + AssetName);



		UPackage* MyPackage = CreatePackage( *PackageName );
		LoadPackage(MyPackage, *PackageName, LOAD_EditorOnly);
		UWorld* NewWorld = NewObject<UWorld>(MyPackage, *AssetName, RF_Public | RF_Standalone);	
		NewWorld->InitializeNewWorld();
		TSoftObjectPtr<UWorld> NWPtr = NewWorld;
		FLatentActionInfo LatentInfo;
		ULevelStreamingDynamic* StreamingLevel = NewObject<ULevelStreamingDynamic>(GetWorld(), FName(PackageName));
		StreamingLevel->SetWorldAsset(NWPtr);	
		StreamingLevel->SetShouldBeLoaded(true);
		StreamingLevel->bInitiallyLoaded = true;
		
		SpawnParams.Owner = Cast<AActor>(StreamingLevel->GetLevelScriptActor());

		SelectedActor = NewWorld->SpawnActor<AIconEditorActor>(SpawnParams);
		SelectedActor->StaticMeshComponent->SetStaticMesh(ActorMesh);
		SelectedActor->SetActorLabel(ActorMesh->GetName(), false);

		ToolkitObj->MyEditor->AssetDataToStreamingLevel.Add(ToolkitObj->AssetList->GetSelectedItems()[0], StreamingLevel);
		GetWorld()->AddStreamingLevel(StreamingLevel);
		UGameplayStatics::LoadStreamLevelBySoftObjectPtr(GetWorld(), StreamingLevel->GetWorldAsset(), true, true, {});
		GetWorld()->RefreshStreamingLevels({StreamingLevel});
		
		//FAssetRegistryModule::AssetCreated(NewWorld);
		//NewWorld->MarkPackageDirty();
		//FString FilePath = ConentDirPath + PackageName.RightChop(1) + FPackageName::GetAssetPackageExtension();
		//FSavePackageArgs SaveArgs;
		//SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		//bool bSuccess = UPackage::SavePackage(MyPackage, NewWorld, *FilePath, SaveArgs);
		
	}
	else
	{
		TSharedPtr<FAssetData> AssetData = ToolkitObj->AssetList->GetSelectedItems()[0];
		ULevelStreaming* Level= ToolkitObj->MyEditor->AssetDataToStreamingLevel.FindRef(AssetData);

		UWorld* SubWorld = Level->GetWorldAsset().Get();
		SelectedActor = SubWorld->SpawnActor<AIconEditorActor>(SpawnParams);
		SelectedActor->StaticMeshComponent->SetStaticMesh(ActorMesh);
		SelectedActor->SetActorLabel(ActorMesh->GetName(), false);
	}


	creationNumber++;
	Viewport->InvalidateHitProxy();
}

//void FIconEditorViewportClient::AddAdditionalActor(UStaticMesh* ActorMesh)
//{
//	
//	
//	FActorSpawnParameters SpawnParams;
//	FString aNameConcatWithID = "IconeEditorActor_" + FString::FromInt(actorID);
//	SpawnParams.Name = FName(aNameConcatWithID);
//	actorID++;
//
//	AIconEditorActor* ActorToAdd = GetWorld()->SpawnActor<AIconEditorActor>(PreviewActor->GetActorLocation(), PreviewActor->GetActorRotation(), SpawnParams);
//	ActorToAdd->StaticMeshComponent->SetStaticMesh(ActorMesh);
//
//	AdditionalActors.Add(ActorToAdd);
//}


#undef LOCTEXT_NAMESPACE
