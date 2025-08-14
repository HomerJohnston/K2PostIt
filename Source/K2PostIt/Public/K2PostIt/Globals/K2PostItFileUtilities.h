// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"

namespace K2PostIt
{
	namespace FileUtilities
	{
		K2POSTIT_API FString GetPluginFolder();

		K2POSTIT_API FString GetResourcesFolder();

		K2POSTIT_API FName GetTagConfigFileName();
	}
}
