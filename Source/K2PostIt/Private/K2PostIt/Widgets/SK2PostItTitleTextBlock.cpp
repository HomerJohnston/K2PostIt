// Unlicensed. This file is public domain.

#include "K2PostIt/Widgets/SK2PostItTitleTextBlock.h"

#include "Widgets/SBoxPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"


void SK2PostItTitleTextBlock::Construct( const FArguments& InArgs )
{
	check(InArgs._Style);

	OnBeginTextEditDelegate = InArgs._OnBeginTextEdit;
	OnTextCommittedDelegate = InArgs._OnTextCommitted;
	IsSelected = InArgs._IsSelected;
	OnVerifyTextChanged= InArgs._OnVerifyTextChanged;
	Text = InArgs._Text;
	EmptyText = InArgs._EmptyText;
	bIsReadOnly = InArgs._IsReadOnly;
	bIsMultiLine = InArgs._MultiLine;
	DoubleSelectDelay = 0.0f;

	OnEnterEditingMode = InArgs._OnEnterEditingMode;
	OnExitEditingMode = InArgs._OnExitEditingMode;

	ChildSlot
	[
		SAssignNew(HorizontalBox, SHorizontalBox)
			
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		[
			SAssignNew(TextBlock, STextBlock)
			.Text(this, &SK2PostItTitleTextBlock::Text_NormalDisplayText)
			.TextStyle( &InArgs._Style->TextStyle )
			.Font(InArgs._Font)
			.ColorAndOpacity( InArgs._ColorAndOpacity )
			.ShadowColorAndOpacity( InArgs._ShadowColorAndOpacity )
			.ShadowOffset( InArgs._ShadowOffset )
			.HighlightText( InArgs._HighlightText )
			.ToolTipText( InArgs._ToolTipText )
			.WrapTextAt( InArgs._WrapTextAt )
			.AutoWrapText( InArgs._AutoWrapNonEditText )
			.Justification( InArgs._Justification )
			.LineBreakPolicy( InArgs._LineBreakPolicy )
			.OverflowPolicy(InArgs._OverflowPolicy)
		]
	];

#if WITH_FANCY_TEXT
	if( bIsMultiLine )
	{
		SAssignNew(MultiLineTextBox, SMultiLineEditableTextBox)
			.Text(InArgs._Text)
			.Style(&InArgs._Style->EditableTextBoxStyle)
			.Font(InArgs._Font)
			.ToolTipText(InArgs._ToolTipText)
			.OnTextChanged(this, &SK2PostItTitleTextBlock::OnTextChanged)
			.OnTextCommitted(this, &SK2PostItTitleTextBlock::OnTextBoxCommitted)
			.WrapTextAt(InArgs._WrapTextAt)
			.AutoWrapText(InArgs._AutoWrapMultilineEditText)
			.Justification(InArgs._Justification)
			.SelectAllTextWhenFocused(true)
			.ClearKeyboardFocusOnCommit(true)
			.RevertTextOnEscape(true)
			.ModiferKeyForNewLine(InArgs._ModiferKeyForNewLine)
			.OverflowPolicy(InArgs._OverflowPolicy);
	}
	else
#endif //WITH_FANCY_TEXT
	{
		SAssignNew(TextBox, SEditableTextBox)
			.Text(InArgs._Text)
			.Style(&InArgs._Style->EditableTextBoxStyle)
			.Font(InArgs._Font)
			.ToolTipText(InArgs._ToolTipText)
			.OnTextChanged(this, &SK2PostItTitleTextBlock::OnTextChanged)
			.OnTextCommitted(this, &SK2PostItTitleTextBlock::OnTextBoxCommitted)
			.SelectAllTextWhenFocused(true)
			.ClearKeyboardFocusOnCommit(false)
			.OverflowPolicy(InArgs._OverflowPolicy);
	}
}

FText SK2PostItTitleTextBlock::Text_NormalDisplayText() const
{
	FText Val = Text.Get();

	return Val.IsEmpty() ? EmptyText.Get() : Val;
}
