// Unlicensed. This file is public domain.

#include "K2PostIt/K2PostItProjectSettings.h"

#include "K2PostIt/Nodes/EdGraphNode_K2PostIt.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ================================================================================================

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

	DefaultCommentColor = GetDefault<UEdGraphNode_K2PostIt>()->CommentColor;
}

// ------------------------------------------------------------------------------------------------

TArray<FLinearColor> UK2PostItProjectSettings::GetQuickColorPaletteColors()
{
	TArray<FLinearColor> Colors = Get().QuickColorPalette;

	if (!Get().bExcludeDefaultFromQuickPalette)
	{
		const FLinearColor& Default = Get().DefaultCommentColor;

		auto CheckColor = [&Default] (const FLinearColor& Color)
		{
			return
				FMath::IsNearlyEqual(Color.R, Default.R, 0.01)
				 && FMath::IsNearlyEqual(Color.G, Default.G, 0.01)
				 && FMath::IsNearlyEqual(Color.B, Default.B, 0.01)
				 && FMath::IsNearlyEqual(Color.A, Default.A, 0.01);
		};

		if (!Colors.ContainsByPredicate(CheckColor))
		{
			Colors.Add(Default);
		}
	}
	
	return Colors;
}

// ------------------------------------------------------------------------------------------------

const FLinearColor& UK2PostItProjectSettings::GetDefaultCommentColor()
{
	return Get().DefaultCommentColor;
}

// ------------------------------------------------------------------------------------------------

const UK2PostItProjectSettings& UK2PostItProjectSettings::Get()
{
	return *GetDefault<UK2PostItProjectSettings>();
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE