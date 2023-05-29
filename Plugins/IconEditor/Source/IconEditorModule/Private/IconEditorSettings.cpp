#include "IconEditorSettings.h"

UIconEditorSettings::UIconEditorSettings()
{
	IconEditorSettings=this;
}

void UIconEditorSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	IconEditorSettings->SaveConfig();
}

