// Copyright Epic Games, Inc. All Rights Reserved.

#include "IconEditorModule.h"

#include "PropertyEditorModule.h"
//#include "IconEditorTypeActions.h"
#include "IconEditorStyle.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include <Editor/ContentBrowser/Public/ContentBrowserModule.h>
#include <Editor/Blutility/Public/EditorUtilityLibrary.h>
#include <Styling/SlateIconFinder.h>


#define LOCTEXT_NAMESPACE "IconEditors"


void FIconEditorModule::StartupModule()
{
	FIconEditorStyle::Get();
	
	IconEditorSettings = GetMutableDefault<UIconEditorSettings>();

	FContentBrowserModule& CBModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& AssetViewContextExtender = CBModule.GetAllAssetViewContextMenuExtenders();
	FContentBrowserMenuExtender_SelectedAssets CustomAssetViewExtender;
	CustomAssetViewExtender.BindRaw(this, &FIconEditorModule::MyAssetViewExtender);
	AssetViewContextExtender.Add(CustomAssetViewExtender);


	// Extend the asset browser for static meshes to include the conversion to skelmesh.
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
					OpenIconEditor();
				});

			InSection.AddMenuEntry("IconEditor", Label, ToolTip, Icon, UIAction);
		}));
	}

	//IConsoleManager::Get().RegisterConsoleCommand

}


TSharedRef<FExtender> FIconEditorModule::MyAssetViewExtender(const TArray<FAssetData>& AssetInfo)
{
	TSharedRef<FExtender> MenuExtender(new (FExtender));
	MenuExtender->AddMenuExtension(
		FName("Delete"),
		EExtensionHook::After,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateLambda([this](FMenuBuilder& InMenuBuilder)
		{
			UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			InMenuBuilder.AddSubMenu(
				FText::FromString("Extra tools"),
				FText::FromString("Submenu for tools created by Vladislav Foerster"),				
				FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InMenuBuilder)
				{
					const FUIAction Action(
						FExecuteAction::CreateLambda([this]()
						{
							OpenIconEditor();
						})
					);
					InMenuBuilder.AddMenuEntry(FText::FromString("Icon Editor"), FText::FromString("Icon Editor"), FSlateIcon(), Action);
				})
			);
		}));
	
	return MenuExtender;
}


void FIconEditorModule::OpenIconEditor()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	TArray<FAssetData> SelectedAssetData = UEditorUtilityLibrary::GetSelectedAssetData();

	for (FAssetData CurrentAsset : UEditorUtilityLibrary::GetSelectedAssetData())
	{
		if (!CurrentAsset.AssetClassPath.GetAssetName().IsEqual("StaticMesh"))
			SelectedAssetData.Remove(CurrentAsset);
	}

	/* if there aren't any assets being edited we know we can create an IconEditor */
	if (IconEditor.Get() == nullptr)
	{
		IconEditor = NewObject<UIconEditor>(AssetEditorSubsystem, NAME_None, RF_Transient);
		IconEditor->Initialize();
	}
	
	/* at this point an icon editor exists and the assets will be added */
	if(IconEditor.Get())
		IconEditor->AddToAssetList(SelectedAssetData);
	IconEditor->MyToolkit.Pin().Get()->DetailsView->SetObject(IconEditor->MyToolkit.Pin().Get()->IconEditorViewportClient->SelectedActor);
}

///////////////////////////////////////////////////////////////////

void FIconEditorModule::ShutdownModule()
{
	FIconEditorStyle::Shutdown();
}


IMPLEMENT_MODULE(FIconEditorModule, IconEditorModule)

#undef LOCTEXT_NAMESPACE
