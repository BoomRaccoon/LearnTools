// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class IconEditorRuntimeModule : ModuleRules
	{
		public IconEditorRuntimeModule(ReadOnlyTargetRules Target) : base(Target)
		{
			PublicIncludePaths.AddRange(
			new string[] {
			}
			);

			PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore"
			}
			);

			PrivateDependencyModuleNames.AddRange(
			new string[] {
				
			}
			);
		}

	}
}
