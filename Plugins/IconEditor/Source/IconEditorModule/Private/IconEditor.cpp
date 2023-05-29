// Copyright Epic Games, Inc. All Rights Reserved.

#include "IconEditor.h"
#include "IconEditorModule.h"
#include "IconEditorToolkit.h"
#include <Blutility/Public/EditorUtilityLibrary.h>
#include <Styling/SlateIconFinder.h>
#include <Modules/ModuleManager.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(IconEditor)

#define LOCTEXT_NAMESPACE "IconEditors"

void UIconEditor::GetObjectsToEdit(TArray<UObject*>& OutObjectsToEdit)
{
	OutObjectsToEdit.Add(IconEditorObject);
}

UIconEditor::UIconEditor()
{
	IconEditorObject = NewObject<UIconEditorAsset>();
	CreateEditorDirectories();
}

void UIconEditor::CreateEditorDirectories()
{
	//FFileManagerGeneric::Get()
}

void UIconEditor::AddToAssetList(TArray<FAssetData> InAssetData)
{
	MyToolkit.Pin()->AddAssetToList(InAssetData);
}

void UIconEditor::TestPrint()
{
	UE_LOG(LogTemp, Display, TEXT("Callback executed"));
}

TSharedPtr<FBaseAssetToolkit> UIconEditor::CreateToolkit()
{
	TSharedPtr<FIconEditorToolkit> Temp = MakeShared<FIconEditorToolkit>(this);
	MyToolkit = Temp;

	UToolMenus::Get()->RemoveEntry("ContentBrowser.AssetContextMenu.StaticMesh", "GetAssetActions", "IconEditor");

	// Extend the asset browser for static meshes
	{
		if (UToolMenu* ToolMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.StaticMesh"))
		{
			FToolMenuSection& Section = ToolMenu->FindOrAddSection("GetAssetActions");
			Section.AddDynamicEntry("IconEditor", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
				{
					const TAttribute<FText> Label = LOCTEXT("IconEditorEntry", "Add to Icon Editor");
					const TAttribute<FText> ToolTip = LOCTEXT("IconEditorEntryToolTip", "Adds the currently selected static meshes to the list of assets in the icon editor");
					const FSlateIcon Icon = FSlateIconFinder::FindIconForClass(UTexture2D::StaticClass());
					const FToolMenuExecuteAction UIAction = FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext& InMenuContext)
						{
							FModuleManager::Get().LoadModuleChecked<FIconEditorModule>("IconEditorModule").OpenIconEditor();
						});

					InSection.AddMenuEntry("IconEditor", Label, ToolTip, Icon, UIAction);
				}));
		}
	}


	return Temp;
}
 #undef LOCTEXT_NAMESPACE