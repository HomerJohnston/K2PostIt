// Unlicensed. This file is public domain.

#include "K2PostIt/K2PostItActionDatabase.h"

#include "BlueprintNodeSpawner.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ------------------------------------------------------------------------------------------------

UBlueprintNodeSpawner* FK2PostItBlueprintNodeSpawnerFactory::MakeK2PostItNodeSpawner()
{
	return nullptr;
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE