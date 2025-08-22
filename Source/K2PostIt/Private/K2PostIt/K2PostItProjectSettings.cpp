// Unlicensed. This file is public domain.

#include "K2PostIt/K2PostItProjectSettings.h"

#include "K2PostIt/Nodes/EdGraphNode_K2PostIt.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ------------------------------------------------------------------------------------------------

UK2PostItProjectSettings::UK2PostItProjectSettings()
{
	const float Alpha = 0.92;
	
	QuickColorPalette =
	{
		GetDefault<UEdGraphNode_K2PostIt>()->CommentColor, // Yellow
		{0.60, 1.00, 0.42, Alpha}, // Green
		{0.36, 0.63, 0.90, Alpha}, // Blue
		{0.90, 0.33, 0.09, Alpha}, // Orange
		{0.99, 0.25, 0.20, Alpha}, // Red
		{0.90, 0.90, 0.90, Alpha}, // White
		{0.30, 0.30, 0.30, Alpha}, // DarkGray
		{0.01, 0.01, 0.01, Alpha}, // Noir
	};
}

// ------------------------------------------------------------------------------------------------

const TArray<FLinearColor>& UK2PostItProjectSettings::GetQuickColorPaletteColors()
{
	return Get().QuickColorPalette;
}

// ------------------------------------------------------------------------------------------------

const UK2PostItProjectSettings& UK2PostItProjectSettings::Get()
{
	return *GetDefault<UK2PostItProjectSettings>();
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE