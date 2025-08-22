// Unlicensed. This file is public domain.

#pragma once

#include "Widgets/SWindow.h"

class SWindow_K2PostIt : public SWindow
{
	// why did epic make this 100??? It forces windows to always be "big" unless you do this
	FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
	{
		return FVector2D(0, 16);
	}
};