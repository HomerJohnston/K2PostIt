// Unlicensed. This file is public domain.

#include "K2PostIt/K2PostItColor.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ------------------------------------------------------------------------------------------------

FLinearColor K2PostItColor::Desaturate(FLinearColor InColor, float Desaturation)
{
	float Lum = InColor.GetLuminance();
	return FMath::Lerp(InColor, FLinearColor(Lum, Lum, Lum, InColor.A), Desaturation);
}

// ------------------------------------------------------------------------------------------------

FLinearColor K2PostItColor::Darken(FLinearColor InColor, float Darken)
{
	return FLinearColor(
		Darken * Darken * InColor.R,
		Darken * Darken * InColor.G,
		Darken * Darken * InColor.B,
		InColor.A);
}

// ------------------------------------------------------------------------------------------------

const FLinearColor K2PostItColor::GetNominalFontColor(FLinearColor NodeColor, FLinearColor DarkNodeFontColor, FLinearColor LightNodeFontColor)
{
	float Luminance = NodeColor.GetLuminance();
	if (Luminance < 0.1)
	{
		float A = 10.0f * FMath::Clamp((0.1 - Luminance), 0.0, 1.0);

		float B = FMath::Lerp(1.0, 0.8, A);

		return DarkNodeFontColor * B;
	}

	return LightNodeFontColor;
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE