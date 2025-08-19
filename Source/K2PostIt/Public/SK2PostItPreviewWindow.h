#pragma once

#include "Widgets/SWindow.h"

class SK2PostItPreviewWindow : public SWindow
{
	// why did epic make this 100??? It forces windows to always be "big" unless you do this
	FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
	{
		return FVector2D();
	}
};