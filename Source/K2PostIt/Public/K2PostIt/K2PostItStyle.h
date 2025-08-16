// Creative commons. Do whatever you want with this file.

#pragma once

#include "Styling/SlateStyle.h"
#include "Delegates/IDelegateInstance.h"

#define LOCTEXT_NAMESPACE "K2PostItEditor"

// ==============================================

struct FK2PostItBrushes
{
	FName None;
	
	// Existing UnrealEd Icons
	FName Icon_FilledCircle;
	FName Icon_Chevron_Right;
	FName Icon_Caret_Right;
	FName Icon_PlusSign;
	FName Icon_ProjectSettings_TabIcon;

	FName Border_K2PostItNode;
	FName Shadow_K2PostItNode;
	FName SelectionShadow_K2PostItNode;

	FName CodeHighlight;
	FName CodeHighlightBorder;
	FName CodeHighlightFill;

	FName Separator;
};

struct FK2PostItStyles
{
	// Existing Unreal Editor styles for style consistency
	FName ButtonStyle_NoBorder;
	FName ButtonStyle_HoverHintOnly;
	FName ButtonStyle_SimpleButton;

	FName TextStyle_Editor;
	FName TextStyle_Normal;
	FName TextStyle_Underlined;
	FName TextStyle_CodeBlock;
};

struct FK2PostItFonts
{
};

extern FK2PostItFonts K2PostItFonts;
extern FK2PostItBrushes K2PostItBrushes;
extern FK2PostItStyles K2PostItStyles;

class FK2PostItStyle
{
public:
	static TArray<TStrongObjectPtr<UTexture2D>> Textures;
	
	static ISlateStyle& Get();

	static const FSlateBrush* GetImageBrush(FName BrushName)
	{
		return Get().GetBrush(BrushName);
	}
	
	FK2PostItStyle();
	virtual ~FK2PostItStyle();

	static FName GetStyleSetName();

	static void Initialize();

	static void ReloadTextures();
	
protected:
	static TSharedRef< class FSlateStyleSet > Create();
	
	static void Initialize_Internal();
	
	static void OnPatchComplete();
	static FDelegateHandle OnPatchCompleteHandle;

	static TSharedPtr<FSlateStyleSet> StyleInstance;

	static const ISlateStyle* GetParentStyle();
	
private:
	// DO NOT USE THIS FOR ANYTHING. It's a dumb macro placeholder.
	// FSlateImageBrush* TEMP;
};

#undef LOCTEXT_NAMESPACE