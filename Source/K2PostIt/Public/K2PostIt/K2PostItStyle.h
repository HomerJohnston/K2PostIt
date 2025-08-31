// Unlicensed. This file is public domain.

#pragma once

#include "Styling/SlateStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "Delegates/IDelegateInstance.h"

#define LOCTEXT_NAMESPACE "K2PostIt"

// ================================================================================================

struct FK2PostItBrushes
{
	FName None;
	
	// Existing UnrealEd Icons
	FName Icon_FilledCircle;
	FName Icon_Chevron_Right;
	FName Icon_Caret_Right;
	FName Icon_PlusSign;
	FName Icon_ProjectSettings_TabIcon;
	FName Icon_Edit;

	FName Border_K2PostItNode;
	FName Shadow_K2PostItNode;
	FName SelectionShadow_K2PostItNode;

	FName CodeHighlight;
	FName CodeHighlightBorder;
	FName CodeHighlightFill;

	FName Separator;

	FName PreviewPaneBorder;

	FName Overlay_QuickColorPaletteShadow;
};

extern FK2PostItBrushes K2PostItBrushes;

// ================================================================================================

struct FK2PostItStyles
{
	// Existing Unreal Editor styles for style consistency
	FName ButtonStyle_NoBorder;
	FName ButtonStyle_HoverHintOnly;
	FName ButtonStyle_SimpleButton;

	FName ButtonStyle_EditButton;

	FName TextStyle_Editor;
	FName TextStyle_Normal;
	FName TextStyle_Underlined;
	FName TextStyle_CommonURL;
	FName TextStyle_CodeBlock;

	FName K2PostItCommonHyperlink;

	FName TextBoxStyle_CommentEditor;
};

extern FK2PostItStyles K2PostItStyles;

// ================================================================================================

struct FK2PostItFonts
{
};

extern FK2PostItFonts K2PostItFonts;

// ================================================================================================

class FK2PostItStyle
{
	
public:
	FK2PostItStyle();
	
	virtual ~FK2PostItStyle();

public:
	static ISlateStyle& Get();

	static const FSlateBrush* GetImageBrush(FName BrushName);

	static FName GetStyleSetName();

	static void Initialize();

	static void ReloadTextures();
	
protected:
	static TSharedRef< class FSlateStyleSet > Create();
	
	static void SetupStyles();
	
	static void OnPatchComplete();

	static const ISlateStyle* GetParentStyle();
	
	static TArray<TStrongObjectPtr<UTexture2D>> Textures;

	static FDelegateHandle OnPatchCompleteHandle;

	static TSharedPtr<FSlateStyleSet> StyleInstance;

};

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE