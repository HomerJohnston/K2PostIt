// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// This file is formatted to be viewable on tabs, 4-space tabs. If you use another setting, sorry!

#include "K2PostIt/K2PostItStyle.h"

#include "ILiveCodingModule.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
//#include "K2PostIt/Globals/K2PostItFileUtilities.h"
//#include "K2PostItEditor/K2PostItDeveloperSettings.h"
#include "K2PostIt/K2PostItColor.h"

TArray<TStrongObjectPtr<UTexture2D>> FK2PostItStyle::Textures;
FDelegateHandle FK2PostItStyle::OnPatchCompleteHandle;

FK2PostItFonts K2PostItFonts;
FK2PostItBrushes K2PostItBrushes;
FK2PostItStyles K2PostItStyles;

#define K2POSTIT_QUOTE(X) #X

/** Makes a simple font definition copying default font */
#define K2POSTIT_DEFINE_FONT(NAME, STYLE, SIZE)\
	K2PostItFonts.NAME = DEFAULT_FONT(STYLE, SIZE);\
	FSlateFontInfo& NAME = K2PostItFonts.NAME

/** Loads a TTF from disk */
#define K2POSTIT_LOAD_FONT(NAME, RESOURCE_PATH, SIZE)\
	TSharedRef<FCompositeFont> SourceCompositeFont_##NAME = MakeShared<FStandaloneCompositeFont>();\
	SourceCompositeFont_##NAME->DefaultTypeface.AppendFont(TEXT("Regular"), K2PostIt::FileUtilities::GetResourcesFolder() / RESOURCE_PATH, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);\
	K2PostItFonts.NAME = FSlateFontInfo(SourceCompositeFont_##NAME, SIZE);\
	FSlateFontInfo& NAME = K2PostItFonts.NAME

/** Define a new brush */
#define K2POSTIT_DEFINE_BRUSH(TYPE, BRUSHNAME, FILENAME, EXTENSION, ...)\
	K2PostItBrushes.BRUSHNAME = K2POSTIT_QUOTE(BRUSHNAME);\
	StyleInstance->Set(K2POSTIT_QUOTE(BRUSHNAME), new TYPE(StyleInstance->RootToContentDir(FILENAME, TEXT(EXTENSION)), __VA_ARGS__));\
	const TYPE& BRUSHNAME = *static_cast<const TYPE*>(StyleInstance->GetBrush(K2POSTIT_QUOTE(BRUSHNAME)))

/** Define a new style */
#define K2POSTIT_DEFINE_STYLE(TYPE, STYLENAME, TEMPLATE, MODS)\
	K2PostItStyles.STYLENAME = K2POSTIT_QUOTE(STYLENAME);\
	StyleInstance->Set(K2POSTIT_QUOTE(STYLENAME), TYPE(TEMPLATE));\
	TYPE& STYLENAME = const_cast<TYPE&>(StyleInstance->GetWidgetStyle<TYPE>(K2POSTIT_QUOTE(STYLENAME)));\
	STYLENAME MODS
	
/** Used to copy an existing UE brush into K2PostIt style for easier use */
#define K2POSTIT_REDEFINE_UE_BRUSH(TYPE, K2POSTITNAME, UESTYLESET, UENAME, ...)\
	K2PostItBrushes.K2POSTITNAME = K2POSTIT_QUOTE(K2POSTITNAME);\
	const TYPE& K2POSTITNAME = *(new TYPE(UESTYLESET::GetBrush(UENAME)->GetResourceName().ToString(), __VA_ARGS__));\
	StyleInstance->Set(K2POSTIT_QUOTE(K2POSTITNAME), const_cast<TYPE*>(&K2POSTITNAME))



#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( StyleInstance->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( StyleInstance->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( StyleInstance->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

#define IMAGE_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define BOX_BRUSH_SVG( RelativePath, ... ) FSlateVectorBoxBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define BORDER_BRUSH_SVG( RelativePath, ... ) FSlateVectorBorderBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

#define CORE_IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( StyleInstance->RootToCoreContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define CORE_BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( StyleInstance->RootToCoreContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define CORE_BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( StyleInstance->RootToCoreContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

#define CORE_IMAGE_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush(StyleInstance->RootToCoreContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define CORE_BOX_BRUSH_SVG( RelativePath, ... ) FSlateVectorBoxBrush(StyleInstance->RootToCoreContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)
#define CORE_BORDER_BRUSH_SVG( RelativePath, ... ) FSlateVectorBorderBrush(StyleInstance->RootToCoreContentDir(RelativePath, TEXT(".svg")), __VA_ARGS__)

#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

#define LOCTEXT_NAMESPACE "K2PostItEditor"

TSharedPtr<FSlateStyleSet> FK2PostItStyle::StyleInstance = nullptr;

ISlateStyle& FK2PostItStyle::Get()
{
	TSharedPtr<FSlateStyleSet> FUFKYOU = StyleInstance;
	return *StyleInstance;
}

FK2PostItStyle::FK2PostItStyle()
{
	
}

FK2PostItStyle::~FK2PostItStyle()
{
	Textures.Empty();

#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::GetModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		LiveCoding->GetOnPatchCompleteDelegate().Remove(OnPatchCompleteHandle);
	}
#endif
	
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
}

FName FK2PostItStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("K2PostItEditorStyle"));
	return StyleSetName;
}

void FK2PostItStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}

#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		OnPatchCompleteHandle = LiveCoding->GetOnPatchCompleteDelegate().AddStatic(&FK2PostItStyle::OnPatchComplete);
	}
#endif
	Initialize_Internal();
}

void FK2PostItStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedRef<class FSlateStyleSet> FK2PostItStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetParentStyleName(FAppStyle::GetAppStyleSetName());
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("K2PostIt")->GetBaseDir() / TEXT("Resources"));
	Style->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	
	return Style;
}

#if WITH_LIVE_CODING
void FK2PostItStyle::OnPatchComplete()
{
//	if (UK2PostItDeveloperSettings::GetCloseAndReopenAssetsOnLiveCoding())
//	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		Initialize_Internal();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
//	}
}

const ISlateStyle* FK2PostItStyle::GetParentStyle()
{
	return &FAppStyle::Get();
}
#endif

#define K2POSTIT_COMMON_BRUSH "Common/ButtonHoverHint"
#define K2POSTIT_COMMON_MARGIN FMargin(4.0 / 16.0)
#define K2POSTIT_COMMON_PRESSED_PADDING FMargin(0, 1, 0, -1) // Push down by one pixel
#define K2POSTIT_COMMON_CHECKBOXSTYLE FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox")


void FK2PostItStyle::Initialize_Internal()
{
	if (!IsRunningCommandlet())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
	
	K2POSTIT_REDEFINE_UE_BRUSH(FSlateImageBrush,			None,							FAppStyle,	"NoBorder",					FVector2f(16, 16), K2PostItColor::Transparent);
	K2POSTIT_REDEFINE_UE_BRUSH(FSlateVectorImageBrush,		Icon_FilledCircle,				FAppStyle,	"Icons.FilledCircle",		FVector2f(16, 16));
	K2POSTIT_REDEFINE_UE_BRUSH(FSlateVectorImageBrush,		Icon_PlusSign,					FAppStyle,	"Icons.Plus",				FVector2f(16, 16));
	K2POSTIT_REDEFINE_UE_BRUSH(FSlateVectorImageBrush,		Icon_ProjectSettings_TabIcon,	FAppStyle,	"ProjectSettings.TabIcon",	FVector2f(16, 16));

	// ============================================================================================
	// FONTS
	// ============================================================================================
//	K2POSTIT_DEFINE_FONT(Font_DialogueText,		"Normal",	10);

	// ============================================================================================
	// BRUSHES
	// ============================================================================================
//	K2POSTIT_DEFINE_BRUSH(FSlateImageBrush,			Icon_AudioTime,					"DialogueNodeIcons/AudioTime", ".png",	FVector2f(16, 16));
	K2POSTIT_DEFINE_BRUSH(FSlateBoxBrush,			Border_K2PostItNode,				"Border_K2PostIt", ".tga", FMargin(4.0/8.0));
	K2POSTIT_DEFINE_BRUSH(FSlateBoxBrush,			Shadow_K2PostItNode,				"Shadow_K2PostIt", ".png", FMargin(22.0/64.0, 4.0/8.0, 42.0/64.0, 4.0/8.0));
	K2POSTIT_DEFINE_BRUSH(FSlateBoxBrush,			SelectionShadow_K2PostItNode,		"SelectionShadow_K2PostIt", ".tga", FMargin(22.0/64.0, 16.0/64.0, 42.0/64.0, 38.0/64.0));

	K2POSTIT_DEFINE_BRUSH(FSlateBoxBrush,			CodeHighlight,						"CodeHighlight_K2PostIt", ".png", FMargin(0.5));
	K2POSTIT_DEFINE_BRUSH(FSlateBoxBrush,			CodeHighlightBorder,				"CodeHighlightBorder_K2PostIt", ".png", FMargin(0.5));
	K2POSTIT_DEFINE_BRUSH(FSlateBoxBrush,			CodeHighlightFill,					"CodeHighlightFill_K2PostIt", ".png", FMargin(0.5));
	K2POSTIT_DEFINE_BRUSH(FSlateBoxBrush,			Separator,							"Box_1px_White", ".png", FMargin(0.5));

	K2POSTIT_DEFINE_BRUSH(FSlateBoxBrush,			PreviewPaneBorder,					"Border_MarkdownPreviewPane", ".png", FMargin(0.5));
	// ============================================================================================
	// BRUSHES - SVGs
	// ============================================================================================
//	K2POSTIT_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Chevron_Right,				"Icon_Chevron_Right", ".svg",			FVector2f(16, 16), K2PostItColor::White);
	K2POSTIT_DEFINE_BRUSH(FSlateVectorImageBrush,	Icon_Edit,						"Icon_Edit", ".svg",				FVector2f(32, 32), K2PostItColor::White);
	
	// ============================================================================================
	// SLIDER STYLES
	// ============================================================================================

	/*
	K2POSTIT_DEFINE_STYLE(FSliderStyle, SliderStyle_FragmentTimePadding, FSliderStyle::GetDefault(),
		.SetBarThickness(0.f)
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, K2PostItColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, K2PostItColor::LightGray))
	);
	*/
	
	// ============================================================================================
	// BUTTON STYLES
	// ============================================================================================

	K2POSTIT_DEFINE_STYLE(FButtonStyle, ButtonStyle_NoBorder, FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"), );
	K2POSTIT_DEFINE_STYLE(FButtonStyle, ButtonStyle_HoverHintOnly, FAppStyle::Get().GetWidgetStyle<FButtonStyle>("HoverHintOnly"), );
	K2POSTIT_DEFINE_STYLE(FButtonStyle, ButtonStyle_SimpleButton, FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"), );

	K2POSTIT_DEFINE_STYLE(FButtonStyle, ButtonStyle_EditButton, ButtonStyle_HoverHintOnly,
		//.SetNormal(CORE_BOX_BRUSH(K2POSTIT_COMMON_BRUSH, K2POSTIT_COMMON_MARGIN, K2PostItColor::Gray))
		//.SetHovered(CORE_BOX_BRUSH(K2POSTIT_COMMON_BRUSH, K2POSTIT_COMMON_MARGIN, K2PostItColor::White))
		//.SetPressed(CORE_BOX_BRUSH(K2POSTIT_COMMON_BRUSH, K2POSTIT_COMMON_MARGIN, K2PostItColor::DarkGray))
		.SetNormalForeground(K2PostItColor::Gray)
		.SetHoveredForeground(K2PostItColor::LightGray)
		.SetPressedForeground(K2PostItColor::Gray)
		.SetPressedPadding(K2POSTIT_COMMON_PRESSED_PADDING)
	);
	
	// ============================================================================================
	// COMBO BUTTON STYLES
	// ============================================================================================

	/*
	K2POSTIT_DEFINE_STYLE(FComboButtonStyle, ComboButtonStyle_K2PostItGameplayTagTypedPicker, FAppStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton"),
		.SetButtonStyle(ButtonStyle_TagButton)
		.SetDownArrowPadding(FMargin(0, 2, 0, 0))
		.SetDownArrowAlignment(VAlign_Top)
		);
	*/
	
	// ============================================================================================
	// CHECKBOX STYLES
	// ============================================================================================

	/*
	K2POSTIT_DEFINE_STYLE(FCheckBoxStyle, CheckBoxStyle_TypeSettingsOverride, K2POSTIT_COMMON_CHECKBOXSTYLE,
		.SetCheckBoxType(ESlateCheckBoxType::CheckBox)
		.SetForegroundColor(K2PostItColor::Gray_Trans) // Unchecked
		.SetHoveredForegroundColor(K2PostItColor::White) // Unchecked, Hovered
		.SetPressedForegroundColor(K2PostItColor::LightGray) // Unchecked, Pressed
		.SetCheckedForegroundColor(K2PostItColor::LightGray) // Checked
		.SetCheckedHoveredForegroundColor(K2PostItColor::White) // Checked, Hovered
		.SetCheckedPressedForegroundColor(K2PostItColor::LightGray) // Checked, Pressed
		
		.SetCheckedImage(Icon_Switch_On)
		.SetCheckedHoveredImage(Icon_Switch_On)
		.SetCheckedPressedImage(Icon_Switch_On)

		.SetUncheckedImage(Icon_Switch_Off)
		.SetUncheckedHoveredImage(Icon_Switch_Off)
		.SetUncheckedPressedImage(Icon_Switch_Off)
	);
	*/
	
	// ============================================================================================
	// SCROLLBAR STYLES
	// ============================================================================================

	/*
	K2POSTIT_DEFINE_STYLE(FScrollBarStyle, ScrollBarStyle_DialogueBox, FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar"),
		.SetThickness(2)
		.SetHorizontalBackgroundImage(Box_SolidBlack)
		.SetHorizontalBottomSlotImage(Box_SolidWhite)
		.SetDraggedThumbImage(Box_SolidWhite)
		.SetHoveredThumbImage(Box_SolidWhite)
		.SetNormalThumbImage(Box_SolidLightGray)
	);
	*/
	
	// ============================================================================================
	// TEXT BLOCK STYLES
	// ============================================================================================

	/*
	K2POSTIT_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_DialogueText, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_DialogueText)
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetFontSize(10)
	);
	*/

	int32 DefaultSize = FCoreStyle::RegularTextSize + 2;
	
	K2POSTIT_DEFINE_STYLE(FTextBlockStyle, TextStyle_Editor, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetFontSize(DefaultSize)
		);
	
	K2POSTIT_DEFINE_STYLE(FTextBlockStyle, TextStyle_Normal, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetFontSize(DefaultSize)
		);

	const FSlateFontInfo Consolas10  = FCoreStyle::GetDefaultFontStyle("Mono", 9);
	
	K2POSTIT_DEFINE_STYLE(FTextBlockStyle, TextStyle_CodeBlock, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetFont(Consolas10)
		.SetFontSize(DefaultSize)
		);
	
	K2POSTIT_DEFINE_STYLE(FTextBlockStyle, TextStyle_Underlined, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalUnderlinedText"),
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetFontSize(DefaultSize)
		);

	//StyleInstance->Set("RichTextBlock.TextHighlight", FTextBlockStyle(TextStyle_Normal));
	
	//StyleInstance->Set("RichTextBlock.Bold", FTextBlockStyle(TextStyle_Normal));

	//StyleInstance->Set("RichTextBlock.BoldHighlight", FTextBlockStyle(TextStyle_Normal));

	StyleInstance->Set("K2PostIt.Bold", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Bold", DefaultSize)));
	
	StyleInstance->Set("K2PostIt.Italic", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Italic", DefaultSize)));

	StyleInstance->Set("K2PostIt.Underline", FTextBlockStyle(TextStyle_Underlined)
		.SetFont(DEFAULT_FONT("Normal", DefaultSize)));

	StyleInstance->Set("K2PostIt.ItalicHighlight", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Italic", DefaultSize)));

	StyleInstance->Set("K2PostIt.Header1", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Bold", DefaultSize + 4)));
	
	StyleInstance->Set("K2PostIt.Header2", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Bold", DefaultSize + 2)));

	StyleInstance->Set("K2PostIt.Header3", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("BoldItalic", DefaultSize + 0)));

	StyleInstance->Set("K2PostIt.Bullet1", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Normal", DefaultSize))
		.SetColorAndOpacity(K2PostItColor::DimGray));
	
	StyleInstance->Set("K2PostIt.Bullet2", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Normal", DefaultSize))
		.SetColorAndOpacity(K2PostItColor::DimGray));

	StyleInstance->Set("K2PostIt.SmallFont", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Normal", DefaultSize * 0.5)));

	StyleInstance->Set("K2PostIt.Code", FTextBlockStyle(TextStyle_Normal)
		.SetFont(DEFAULT_FONT("Normal", DefaultSize)));

	// ============================================================================================
	// EDITABLE TEXT BLOCK STYLES
	// ============================================================================================

	/*
	K2POSTIT_DEFINE_STYLE(FEditableTextBoxStyle, EditableTextBoxStyle_Dialogue, FEditableTextBoxStyle::GetDefault(),
		.SetScrollBarStyle(ScrollBarStyle_DialogueBox) // This doesn't do dick, thanks Epic
		.SetTextStyle(TextBlockStyle_DialogueText)
		.SetForegroundColor(FSlateColor::UseForeground())
		.SetPadding(0)
		.SetBackgroundImageNormal(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageHovered(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageFocused(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageReadOnly(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundColor(FStyleColors::Recessed)
	);
	*/
	
	// ============================================================================================
	// PROGRESS BAR STYLES
	// ============================================================================================

	/*
	K2POSTIT_DEFINE_STYLE(FProgressBarStyle, ProgressBarStyle_FragmentTimePadding, FProgressBarStyle::GetDefault(),
		.SetBackgroundImage(None)
		.SetFillImage(Box_SolidWhite)
		.SetEnableFillAnimation(false)
	);
	*/
	
	StyleInstance->Set("K2PostItEditor.PluginAction", new FSlateVectorImageBrush(StyleInstance->RootToContentDir(L"K2PostItProjectSettings", TEXT(".svg")), CoreStyleConstants::Icon20x20));
}

#undef LOCTEXT_NAMESPACE