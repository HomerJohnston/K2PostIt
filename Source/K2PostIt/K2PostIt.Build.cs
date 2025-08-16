// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class K2PostIt : ModuleRules
{
	public K2PostIt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
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
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                
				"UnrealEd",
				"PropertyEditor",
				"AssetTools",
				"DetailCustomizations",
                
				"KismetWidgets", 
                
				"EditorStyle",
				"GraphEditor",
                
				"EditorSubsystem",
				"InputCore",
                
				"PropertyEditor",
				"ToolMenus",
                
				"UMG",
				"UMGEditor",
                
				"DeveloperSettings",
                
				"BlueprintGraph", "LiveCoding",
                
				"EditorWidgets",
				"ToolWidgets",
				"KismetWidgets",
                
				"Projects",
                
				"AssetRegistry",
				"AssetDefinition", "WebBrowser"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		if (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion < 5)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"StructUtils" // This was rolled into engine core in 5.5
				}
			);
		}
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
