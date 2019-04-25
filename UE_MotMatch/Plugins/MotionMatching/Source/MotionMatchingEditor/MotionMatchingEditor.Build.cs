// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MotionMatchingEditor : ModuleRules
{
	public MotionMatchingEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
                "MotionMatchingEditor/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                "MotionMatchingEditor/Private",
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Json",
                "Slate",
                "SlateCore",
                "Engine",
                "InputCore",
                "UnrealEd", // for FAssetEditorManager
				"KismetWidgets",
                "Kismet",  // for FWorkflowCentricApplication
				"PropertyEditor",
                "RenderCore",
                "ContentBrowser",
                "WorkspaceMenuStructure",
                "EditorStyle",
                "MeshPaint",
                "EditorWidgets",
                "Projects",
                "MotionMatching",
                "AnimGraph",
                "BlueprintGraph",
                "AssetRegistry",
                "AdvancedPreviewScene",
				// ... add private dependencies that you statically link with here ...	
			}
			);


        PrivateIncludePathModuleNames.AddRange(
       new string[] {
                "Settings",
                "IntroTutorials",
                "AssetTools",
                "LevelEditor"
       }
   );

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "AssetTools"
            }
        );
    }
}
