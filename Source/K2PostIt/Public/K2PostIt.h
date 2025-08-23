// Unlicensed. This file is public domain.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FUICommandList;

#define LOCTEXT_NAMESPACE "K2PostIt"

// ================================================================================================

class FK2PostItModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;
};

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE