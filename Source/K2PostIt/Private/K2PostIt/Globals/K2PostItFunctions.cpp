// Unlicensed. This file is public domain.

#include "K2PostIt/Globals/K2PostItFunctions.h"

#define LOCTEXT_NAMESPACE "EdGraph"

// ================================================================================================

namespace K2PostIt
{
	void OnBrowserLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata)
	{
		const FString* url = Metadata.Find(TEXT("href"));

		if(url)
		{
			FPlatformProcess::LaunchURL(**url, nullptr, nullptr);
		}
	}
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE