#pragma once

#include "IconEditorAsset.generated.h"


UCLASS()
class UIconEditorAsset : public UObject
{
	GENERATED_BODY()
public:

	TMap<FAssetData, TWeakObjectPtr<ULevel>> ActorLevelMap;


	TArray<UWorld*> AdditionalWorlds;
	TMap<FAssetData, TArray<class AIconEditorActor>> AdditionalActors;

	TArray<FAssetData> AssetsArray;


	void PreEditChange(FProperty* PropertyAboutToChange) override;
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
};
