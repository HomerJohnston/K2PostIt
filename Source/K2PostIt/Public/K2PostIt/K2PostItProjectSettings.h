// Unlicensed. This file is public domain.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "K2PostItProjectSettings.generated.h"

UCLASS(Config = Editor, DefaultConfig, DisplayName = "K2 PostIt")
class UK2PostItProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UK2PostItProjectSettings();
	
protected:
	/** Controls available colors from the quick-colors palette. Editing these will not affect any existing nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "K2 PostIt")
	TArray<FLinearColor> QuickColorPalette;

	/** Controls the initial color for a comment node. */
	UPROPERTY(Config, EditAnywhere, Category = "K2 PostIt")
	FLinearColor DefaultCommentColor;

	/** By default, the default comment color will be added to the quick palette (if it isn't already in it). */
	UPROPERTY(Config, EditAnywhere, Category = "K2 PostIt")
	bool bExcludeDefaultFromQuickPalette = false;
	
	/** If set, markdown will be normally disabled on all comment nodes, and a bool will be placed on nodes to selectively enable markdown rendering, instead of selectively disabling. */
	UPROPERTY(Config, EditAnywhere, Category = "K2 PostIt")
	bool bDisableMarkdownByDefault = false;

public:
	static TArray<FLinearColor> GetQuickColorPaletteColors();

	static const FLinearColor& GetDefaultCommentColor();
	
	UFUNCTION()
	static bool GetMarkdownDisabledByDefault() { return Get().bDisableMarkdownByDefault; }
	
protected:
	static const UK2PostItProjectSettings& Get();
};