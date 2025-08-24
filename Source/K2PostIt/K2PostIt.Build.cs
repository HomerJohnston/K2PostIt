// Unlicensed. This file is public domain.

using UnrealBuildTool;

public class K2PostIt : ModuleRules
{
	public K2PostIt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		bUseUnity = true;
		
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
				"AssetDefinition",
				"AssetRegistry",
				"AssetTools",
				"BlueprintGraph", 
				"DetailCustomizations",
				"DeveloperSettings",
				"EditorStyle",
				"EditorSubsystem",
				"EditorWidgets",
				"Engine",
				"GraphEditor",
				"InputCore",
				"KismetWidgets", 
				"LiveCoding",
				"Projects",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"ToolWidgets",
				"UMG",
				"UMGEditor",
				"UnrealEd",
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
			}
			);
	}
}
