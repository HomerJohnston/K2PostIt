
#pragma once

#include "Cheater.generated.h"

UCLASS()
class UK2PostItCheater : public UK2Node
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 WTF;
	
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
};