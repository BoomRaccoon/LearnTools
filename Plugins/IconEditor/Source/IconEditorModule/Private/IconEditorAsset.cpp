#include "IconEditorAsset.h"

void UIconEditorAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	//Super:PostEditChangeProperty(PropertyChangedEvent);
	UE_LOG(LogTemp, Display, TEXT("PostEditChangeProperty from IconEditorActor"));
}

void UIconEditorAsset::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	UE_LOG(LogTemp, Display, TEXT("PreEditChange from IconEditorActor"));
}
