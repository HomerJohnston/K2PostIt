// Unlicensed. This file is public domain.

#pragma once

#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

namespace K2PostIt
{
	namespace FileUtilities
	{
		K2POSTIT_API FString GetPluginFolder();

		K2POSTIT_API FString GetResourcesFolder();

		K2POSTIT_API FName GetTagConfigFileName();
	}
}

#undef LOCTEXT_NAMESPACE