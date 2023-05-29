// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EditorViewportClient.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "IconEditorActor.h"

#include "IconEditorViewportClient.generated.h"

class FIconEditorToolkit;
class UIconEditorComponent;
class UStaticMeshComponent;
class FPreviewScene;
class SEditorViewport;
class AActor;
class UIconEditorDefinition;
class FScopedTransaction;
class USceneCaptureComponent;

namespace UE::IconEditors::Editor
{
	struct FSelectedItem;
};

UCLASS()
class AIconEditorActor : public AActor
{
    GENERATED_BODY()


public:
    AIconEditorActor()
    {
        // Create a static mesh component
        StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
        RootComponent = StaticMeshComponent.Get();
    }

	~AIconEditorActor()
	{
		UE_LOG(LogTemp, Display, TEXT("An IconEditorActor got destroyed"));
	}

	UPROPERTY()
    TSoftObjectPtr<UStaticMeshComponent> StaticMeshComponent;
};



class FIconEditorViewportClient	: public FEditorViewportClient
{
	friend class FIconEditorToolkit;

public:
	UStaticMesh* CurrentMesh;
	AIconEditorActor* SelectedActor = nullptr;
	AIconEditorActor* PersistantActor = nullptr;
	TArray<AIconEditorActor*> AdditionalActors;
	TObjectPtr<UTextureRenderTarget2D> RenderTarget2D;

	inline static int creationNumber = 0;
	inline static bool worldCreated = false;

public:
	explicit FIconEditorViewportClient(const TSharedRef<const FIconEditorToolkit>& InAssetEditorToolkit, FPreviewScene* InPreviewScene = nullptr, const TWeakPtr<SEditorViewport>& InEditorViewportWidget = nullptr);
	void GetAllSublevels();
	void LogWorld();
	virtual ~FIconEditorViewportClient() override;

	void SetPreviewMesh(UStaticMesh* InStaticMesh);
	void SetCaptureComponent();
	void UpdateTextureTarget(int32 width, int32 height);

	void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	bool InputAxis(FViewport* InViewport, FInputDeviceId DeviceID, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override;

	void TrackingStarted(const struct FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge) override;
	void TrackingStopped() override;

protected:
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawCanvas( FViewport& InViewport, FSceneView& View, FCanvas& Canvas ) override;

	void SaveIcon(FString Path);
	void AddAdditionalActor(UStaticMesh* ActorMesh, FVector SpawnPosition = FVector(0,0,0));

private:
	/** Weak pointer to the preview Mesh component added to the preview scene */
	TObjectPtr<UStaticMeshComponent> PreviewMeshComponent = nullptr;

	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent = nullptr;

	/** Weak pointer back to asset editor we are embedded in */
	TWeakPtr<const FIconEditorToolkit> IconEditorToolkit;

	/** Pointer to currently active transaction */
	FScopedTransaction* ScopedTransaction = nullptr;

	/** True if currently using the transform widget */
	bool bIsManipulating = false;
	bool bDuplicateActorsInProgress = false;
	bool bDuplicateOnNextDrag = false;
	

	/** Currently active transform widget coord system. */
	ECoordSystem WidgetCoordSystemSpace = COORD_World;
	/** Currently active transform widget type. */
	UE::Widget::EWidgetMode WidgetMode = UE::Widget::WM_Rotate;
	/** Cached widget location (updated from slots and annotations before manipulating the gizmo) */
	mutable FVector CachedWidgetLocation = FVector::ZeroVector;

private:

	// Utility functions to return the modifier key states
	bool IsAltPressed() const { return Viewport->KeyState(EKeys::LeftAlt) || Viewport->KeyState(EKeys::RightAlt); };
	bool IsCtrlPressed() const { return Viewport->KeyState(EKeys::LeftControl) || Viewport->KeyState(EKeys::RightControl); };
	bool IsShiftPressed() const { return Viewport->KeyState(EKeys::LeftShift) || Viewport->KeyState(EKeys::RightShift); };
	bool IsCmdPressed() const { return Viewport->KeyState(EKeys::LeftCommand) || Viewport->KeyState(EKeys::RightCommand); };

	void SimpleScreenShot();

	void BeginTransaction(FText Text);
	void EndTransaction();

	void SetupRenderTarget();

	bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;
	FVector GetWidgetLocation() const override { if (SelectedActor) return SelectedActor->GetActorLocation(); else return {0,0,0};  };

	virtual UE::Widget::EWidgetMode GetWidgetMode() const override;
	virtual void SetWidgetMode(UE::Widget::EWidgetMode NewMode) override;
	virtual bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override;
	FMatrix GetWidgetCoordSystem() const override { return FMatrix::Identity; };
	virtual ECoordSystem GetWidgetCoordSystemSpace() const override { return ECoordSystem::COORD_World; };
	virtual void SetWidgetCoordSystemSpace(ECoordSystem NewCoordSystem) override { /* Empty, we support only world for the time being */ };
};
