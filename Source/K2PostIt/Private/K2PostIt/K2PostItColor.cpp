// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "K2PostIt/K2PostItColor.h"

FLinearColor K2PostItColor::Desaturate(FLinearColor InColor, float Desaturation)
{
	float Lum = InColor.GetLuminance();
	return FMath::Lerp(InColor, FLinearColor(Lum, Lum, Lum, InColor.A), Desaturation);
}

FLinearColor K2PostItColor::Darken(FLinearColor InColor, float Darken)
{
	//return FLinearColor::LerpUsingHSV(InColor, K2PostItColor::Black, Darken);
	
	return FLinearColor(Darken * Darken * InColor.R, Darken * Darken * InColor.G, Darken * Darken * InColor.B, InColor.A);
}
