// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class IconEditorModule : ModuleRules
	{
		public IconEditorModule(ReadOnlyTargetRules Target) : base(Target)
		{

            // ...
            // Get the engine path. Ends with "Engine/"
            //string engine_path = System.IO.Path.GetFullPath(Target.RelativeEnginePath);            
            //string render_core_path = engine_path + "Source/Runtime/RenderCore/Private";
            
			// Now get the base of UE4's modules dir (could also be Developer, Editor, ThirdParty)
            //string editor_path = engine_path + "Editor/";

            // now you can include the module's private paths!
            // as an example, you can expose UE4's abstraction of D3D11, located in Source/Runtime/Windows/D3D11RHI
			//PrivateIncludePaths.Add(srcrt_path + "LevelEditor/Private/");
            //PublicIncludePaths.Add(render_core_path);
            //PublicIncludePaths.Add(srcrt_path + "Windows/D3D11RHI/Private/Windows");

            PublicIncludePaths.AddRange(
			new string[] {
			}
			);

            PrivateIncludePaths.AddRange(
            new string[] {
				"IconEditorModule/Private",
				"IconEditorModule/Public"
            }
            );

            PublicDependencyModuleNames.AddRange(
			new string[] {
			}
			);

			PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"InputCore",
				"AdvancedPreviewScene",
				"Blutility",
				"ImageWriteQueue",
				"ComponentVisualizers",
				"ContentBrowser",
				"RenderCore",
				"DetailCustomizations",
				"EditorInteractiveToolsFramework",
				"Engine",
				"ModelingEditorUI",
				"SceneOutliner",
				"IconEditorRuntimeModule",
				"ImageWriteQueue",
				"InteractiveToolsFramework",
				"PropertyEditor",
				"DeveloperSettings",
				"Slate",
				"LevelEditor",
				"SlateCore",
				"ToolMenus",
				"UnrealEd",
            }
			);
		}

	}
}
