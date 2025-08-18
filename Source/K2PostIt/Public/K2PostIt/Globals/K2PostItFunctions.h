

#pragma once
#include "Framework/Text/SlateHyperlinkRun.h"

namespace K2PostIt
{
	static void OnBrowserLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata)
	{
		const FString* url = Metadata.Find(TEXT("href"));

		if(url)
		{
			FPlatformProcess::LaunchURL(**url, nullptr, nullptr);
		}
	}
}