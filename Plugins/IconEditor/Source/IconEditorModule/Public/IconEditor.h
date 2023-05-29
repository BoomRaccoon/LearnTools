// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Tools/UAssetEditor.h"
#include "IconEditorToolkit.h"
#include "IconEditorAsset.h"
#include "IconEditor.generated.h"



class FBaseAssetToolkit;
class FIconEditorToolkit;

UCLASS(Transient)
class UIconEditor : public UAssetEditor
{
	friend class FIconEditorToolkit;
	friend class FIconEditorViewportClient;

	GENERATED_BODY()
public:
	UIconEditor();
	void CreateEditorDirectories();
	void AddToAssetList(TArray<FAssetData> InAssetAdata);
	TWeakPtr<FIconEditorToolkit> MyToolkit = nullptr;

	void TestPrint();

protected:
	virtual void GetObjectsToEdit(TArray<UObject*>& OutObjectsToEdit) override;


	TMap< TSharedPtr<FAssetData>, ULevelStreaming* > AssetDataToStreamingLevel;
	TArray<TSharedPtr<FAssetData>> AssetsSharedPtrArray;
	UIconEditorAsset* IconEditorObject = nullptr;

	virtual TSharedPtr<FBaseAssetToolkit> CreateToolkit() override;

private:
	
};
