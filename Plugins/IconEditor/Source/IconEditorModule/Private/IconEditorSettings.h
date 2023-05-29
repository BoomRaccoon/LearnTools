#pragma once

#include "IconEditorSettings.generated.h"

class FPoint;
class FilePath;

UIconEditorSettings* IconEditorSettings;

/** Settings for the Enhanced Input Editor Subsystem that are persistent between a project's users */
UCLASS(Config=Editor, defaultconfig, meta = (DisplayName = "Icon Editor"))
class UIconEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public: 
	UIconEditorSettings();

	UPROPERTY(config, EditAnywhere, Category = Default, meta=(ContentDir, LongPackageName))
	FDirectoryPath DefaultIconSavePath;
	
	UPROPERTY(config, EditAnywhere, Category = Default)
	FString Prefix;

	UPROPERTY(config, EditAnywhere, Category = Default)
	FString Suffix;

	/** The default player input class that the Enhanced Input Editor subsystem will use. */
	UPROPERTY(config, EditAnywhere, Category = Default)
	FUintPoint DefaultIconResolution = (512,512);

	
public:
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

};
