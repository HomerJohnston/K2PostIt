
#pragma once

#include "Cheater.generated.h"

UCLASS()
class UK2PostItCheater : public UK2Node
{
	GENERATED_BODY()

	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};