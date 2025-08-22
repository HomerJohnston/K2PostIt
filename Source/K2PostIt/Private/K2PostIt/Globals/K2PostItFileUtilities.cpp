// Unlicensed. This file is public domain.

#include "K2PostIt/Globals/K2PostItFileUtilities.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ------------------------------------------------------------------------------------------------

FString K2PostIt::FileUtilities::GetPluginFolder()
{
	static FString PluginDir;

	if (PluginDir.IsEmpty())
	{
		PluginDir = IPluginManager::Get().FindPlugin(TEXT("K2PostIt"))->GetBaseDir();
	}

	return PluginDir;
}

// ------------------------------------------------------------------------------------------------

FString K2PostIt::FileUtilities::GetResourcesFolder()
{
	return GetPluginFolder() / "Resources";
}

// ------------------------------------------------------------------------------------------------

FName K2PostIt::FileUtilities::GetTagConfigFileName()
{
	return FName("K2PostItGameplayTags.ini");
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE