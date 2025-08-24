// Unlicensed. This file is public domain.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "SlateGlobals.h"
#include "Styling/SlateColor.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Framework/SlateDelegates.h"
#include "Framework/Text/TextLayout.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

class FActiveTimerHandle;
class IBreakIterator;
class SEditableTextBox;
class SHorizontalBox;
class SMultiLineEditableTextBox;
class STextBlock;

DECLARE_DELEGATE_OneParam(FOnBeginTextEdit, const FText&)

/**
 * Slate's InlineEditableTextBlock's are double selectable to go from a STextBlock to become SEditableTextBox.
 */
class SK2PostItTitleTextBlock: public SInlineEditableTextBlock
{
	SLATE_BEGIN_ARGS( SK2PostItTitleTextBlock )
		: _Text()
		, _EmptyText()
		, _Style( &FCoreStyle::Get().GetWidgetStyle<FInlineEditableTextBlockStyle>("InlineEditableTextBlockStyle") )
		, _Font()
		, _ColorAndOpacity()
		, _ShadowOffset()
		, _ShadowColorAndOpacity()
		, _HighlightText()
		, _WrapTextAt(0.0f)
		, _AutoWrapNonEditText(false)
		, _AutoWrapMultilineEditText(false)
		, _Justification(ETextJustify::Left)
		, _LineBreakPolicy()
		, _IsReadOnly(false)
		, _MultiLine(false)
		, _ModiferKeyForNewLine(EModifierKey::None)
		, _OverflowPolicy()
	{
	}

		/** The text displayed in this text block */
		SLATE_ATTRIBUTE( FText, Text )

		/** The text displayed in this text block when there is no source text */
		SLATE_ATTRIBUTE( FText, EmptyText )

		/** Pointer to a style of the inline editable text block, which dictates the font, color, and shadow options. */
		SLATE_STYLE_ARGUMENT( FInlineEditableTextBlockStyle, Style )

		/** Sets the font used to draw the text (overrides style) */
		SLATE_ATTRIBUTE( FSlateFontInfo, Font )

		/** Text color and opacity (overrides style) */
		SLATE_ATTRIBUTE( FSlateColor, ColorAndOpacity )

		/** Drop shadow offset in pixels (overrides style) */
		SLATE_ATTRIBUTE( FVector2D, ShadowOffset )

		/** Shadow color and opacity (overrides style) */
		SLATE_ATTRIBUTE( FLinearColor, ShadowColorAndOpacity )

		/** Highlight this text in the text block */
		SLATE_ATTRIBUTE( FText, HighlightText )

		/** Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs. */
		SLATE_ATTRIBUTE( float, WrapTextAt )

		/** Whether the text should wrap automatically when in non-edit mode */
		SLATE_ATTRIBUTE( bool, AutoWrapNonEditText )

		/** Whether the multiline text should wrap automatically when in edit mode */
		SLATE_ATTRIBUTE( bool, AutoWrapMultilineEditText )

		/** How the text should be aligned with the margin. */
		SLATE_ATTRIBUTE( ETextJustify::Type, Justification )

		/** The iterator to use to detect appropriate soft-wrapping points for lines (or null to use the default) */
		SLATE_ARGUMENT( TSharedPtr<IBreakIterator>, LineBreakPolicy )

		/** True if the editable text block is read-only. It will not be able to enter edit mode if read-only */
		SLATE_ATTRIBUTE( bool, IsReadOnly )

		/** True if the editable text block is multi-line */
		SLATE_ARGUMENT( bool, MultiLine )

		/** The optional modifier key necessary to create a newline when typing into the editor. */
		SLATE_ARGUMENT(EModifierKey::Type, ModiferKeyForNewLine)

		/** Callback when the text starts to be edited */
		SLATE_EVENT( FOnBeginTextEdit, OnBeginTextEdit )

		/** Callback when the text is committed. */
		SLATE_EVENT( FOnTextCommitted, OnTextCommitted )

		/** Callback when the text editing begins. */
		SLATE_EVENT(FSimpleDelegate, OnEnterEditingMode)

		/** Callback when the text editing ends. */
		SLATE_EVENT(FSimpleDelegate, OnExitEditingMode)

		/** Callback to check if the widget is selected, should only be hooked up if parent widget is handling selection or focus. */
		SLATE_EVENT( FIsSelected, IsSelected )

		/** Called whenever the text is changed programmatically or interactively by the user */
		SLATE_EVENT( FOnVerifyTextChanged, OnVerifyTextChanged )

		/** Determines what happens to text that is clipped and doesn't fit within the clip rect for this widget */
		SLATE_ARGUMENT(TOptional<ETextOverflowPolicy>, OverflowPolicy)
	SLATE_END_ARGS()

	/**
	 * Construct this widget.  Called by the SNew() Slate macro.
	 *
	 * @param	InArgs				Declaration used by the SNew() macro to construct this widget
	 * @param	InViewModel			The UI logic not specific to slate
	 */
	void Construct( const FArguments& InArgs );

	/** Attribute for the text to use for the widget */
	TAttribute<FText> EmptyText;
	
	FText Text_NormalDisplayText() const;
};
