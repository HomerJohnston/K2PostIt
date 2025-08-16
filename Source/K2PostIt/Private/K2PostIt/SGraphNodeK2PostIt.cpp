// Copyright Epic Games, Inc. All Rights Reserved.


#include "K2PostIt/SGraphNodeK2PostIt.h"

#include "Animation/CurveHandle.h"
#include "Animation/CurveSequence.h"
#include "Containers/EnumAsByte.h"
#include "Delegates/Delegate.h"
#include "EdGraph/EdGraphNode.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericApplication.h"
#include "GraphEditorSettings.h"
#include "HAL/PlatformCrt.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Internationalization/Text.h"
#include "Layout/ChildrenBase.h"
#include "Layout/Geometry.h"
#include "Layout/Margin.h"
#include "Math/Color.h"
#include "Math/UnrealMathSSE.h"
#include "Misc/Attribute.h"
#include "Misc/Guid.h"
#include "SCommentBubble.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"
#include "SlotBase.h"
#include "Styling/AppStyle.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateBrush.h"
#include "Templates/Casts.h"
//#include "TextWrapperHelpers.h"
#include "Selection.h"
#include "TutorialMetaData.h"
#include "K2PostIt/EdGraphNode_K2PostIt.h"
#include "K2PostIt/K2PostItColor.h"
#include "K2PostIt/K2PostItStyle.h"
#include "Types/SlateEnums.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SErrorText.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SWebBrowserView.h"
#include "Interfaces/IPluginManager.h"
#include "K2PostIt/K2PostItDecorator_InlineCode.h"
#include "K2PostIt/K2PostItDecorator_Separator.h"

#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/SRichTextBlock.h"

class FDragDropEvent;

#define LOCTEXT_NAMESPACE "K2PostIt"

namespace SCommentNodeDefs
{

	/** Size of the hit result border for the window borders */
	/* L, T, R, B */
	static const FSlateRect HitResultBorderSize(10,10,10,10);

	/** Minimum resize width for comment */
	static const float MinWidth = 30.0;

	/** Minimum resize height for comment */
	static const float MinHeight = 30.0;

	/** TitleBarColor = CommentColor * TitleBarColorMultiplier */
	static const float TitleBarColorMultiplier = 0.6f;

	/** Titlebar Offset - taken from the widget borders in UpdateGraphNode */
	static const FSlateRect TitleBarOffset(13,8,-3,0);
}


void SGraphNodeK2PostIt::Construct(const FArguments& InArgs, UEdGraphNode_K2PostIt* InNode)
{
	this->GraphNode = InNode;
	this->bIsSelected = false;

	// Set up animation
	{
		ZoomCurve = SpawnAnim.AddCurve(0, 0.1f);
		FadeCurve = SpawnAnim.AddCurve(0.15f, 0.15f);
	}

	// Cache these values so they do not force a re-build of the node next tick.
	CachedCommentTitle = GetNodeComment();
	CachedWidth = InNode->NodeWidth;

	this->UpdateGraphNode();

	// Pull out sizes
	UserSize.X = InNode->NodeWidth;
	UserSize.Y = InNode->NodeHeight;

	// Cache desired size so we cull correctly. We can do this as our ComputeDesiredSize ignores the layout scale.
	CacheDesiredSize(1.0f);

	MouseZone = CRWZ_NotInWindow;
	bUserIsDragging = false;
}

void SGraphNodeK2PostIt::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	const FString CurrentCommentTitle = GetNodeComment();
	if (CurrentCommentTitle != CachedCommentTitle)
	{
		CachedCommentTitle = CurrentCommentTitle;
	}

	const int32 CurrentWidth = static_cast<int32>(UserSize.X);
	if (CurrentWidth != CachedWidth)
	{
		CachedWidth = CurrentWidth;
	}

	UEdGraphNode_K2PostIt* CommentNode = CastChecked<UEdGraphNode_K2PostIt>(GraphNode);
	if (bCachedBubbleVisibility != CommentNode->bCommentBubbleVisible_InDetailsPanel)
	{
		CommentBubble->UpdateBubble();
		bCachedBubbleVisibility = CommentNode->bCommentBubbleVisible_InDetailsPanel;
	}

	if (CachedFontSize != CommentNode->GetFontSize())
	{
		UpdateGraphNode();
	}
}

FReply SGraphNodeK2PostIt::OnDrop( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent )
{
	return FReply::Unhandled();
}

void SGraphNodeK2PostIt::OnDragEnter( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent )
{

}

float SGraphNodeK2PostIt::GetWrapAt() const
{
	return (float)(CachedWidth - 32.0f);
}

void SGraphNodeK2PostIt::RebuildRichText()
{
	UEdGraphNode_K2PostIt* CommentNode = CastChecked<UEdGraphNode_K2PostIt>(GraphNode);
	
	if (!CommentNode)
	{
		return;
	}

	FormattedTextPanel->ClearChildren();
	
	TArray< TSharedRef< ITextDecorator > > CustomDecorators;

	const FTextBlockStyle& MyStyle = FK2PostItStyle::Get().GetWidgetStyle<FTextBlockStyle>(K2PostItStyles.TextStyle_Normal);

	FWidgetDecorator::FCreateWidget Delegate;

	Delegate.BindStatic(&GetWidgetThing);

	// 	DECLARE_DELEGATE_RetVal_TwoParams( FSlateWidgetRun::FWidgetRunInfo, FCreateWidget, const FTextRunInfo& /*RunInfo*/, const ISlateStyle* /*Style*/ )

	TDelegate<FSlateWidgetRun::FWidgetRunInfo(const FTextRunInfo& RunInfo, const ISlateStyle* Style)> Delegate2;
	
	// FSlateWidgetRun::FWidgetRunInfo, FCreateWidget, const FTextRunInfo& /*RunInfo#1#, const ISlateStyle* /*Style#1# 
	//Delegate = [] (const FTextRunInfo& RunInfo, const ISlateStyle* Style) -> FSlateWidgetRun::FWidgetRunInfo
	//{
		
	//};

	TSharedRef<FWidgetDecorator> Del = FWidgetDecorator::Create("Test", Delegate2);
	
	CustomDecorators.Add(Del);
	/*
	for (auto& CurrentEvent : CollectedEvents)
	{
		CustomDecorators.Add(SRichTextBlock::HyperlinkDecorator(CurrentEvent, FSlateHyperlinkRun::FOnClick::CreateLambda(
			[this, VisualLoggerView](const FSlateHyperlinkRun::FMetadata& Metadata){ VisualLoggerView->SetSearchString(FText::FromString(Metadata[TEXT("id")])); }
		)));
	}
	*/
	
	auto& Blocks = CommentNode->Blocks;

	for (TInstancedStruct<FK2PostIt_BaseBlock>& Block : Blocks)
	{
		FormattedTextPanel->AddSlot()
		.AutoHeight()
		[
			Block.GetPtr<FK2PostIt_BaseBlock>()->Draw().ToSharedRef()
		];
	}
	/*
	FormattedTextPanel->AddSlot()
	.AutoHeight()
	[
		SNew(SRichTextBlock)
		.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_Normal)
		.DecoratorStyleSet( &FK2PostItStyle::Get() )
		.Text(Text)
		.LineHeightPercentage(1.1f)
		.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
		.AutoWrapText(true)
		+ SRichTextBlock::Decorator(FK2PostItDecorator_InlineCode::Create("Hello", FLinearColor::Blue))
		+ SRichTextBlock::Decorator(FK2PostItDecorator_Separator::Create("Separator", FLinearColor::Blue))
	];
	*/
}

bool SGraphNodeK2PostIt::IsNameReadOnly() const
{
	return !IsEditable.Get() || SGraphNode::IsNameReadOnly();
}

void SGraphNodeK2PostIt::UpdateGraphNode()
{
	// No pins in a comment box
	InputPins.Empty();
	OutputPins.Empty();

	// Avoid standard box model too
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	// Remember if we should be showing the bubble
	UEdGraphNode_K2PostIt* CommentNode = CastChecked<UEdGraphNode_K2PostIt>(GraphNode);
	bCachedBubbleVisibility = CommentNode->bCommentBubbleVisible_InDetailsPanel;

	// Setup a tag for this node
	FString TagName;

	// We want the name of the blueprint as our name - we can find the node from the GUID
	UObject* Package = GraphNode->GetOutermost();
	UObject* LastOuter = GraphNode->GetOuter();
	while (LastOuter->GetOuter() != Package)
	{
		LastOuter = LastOuter->GetOuter();
	}
	TagName = FString::Printf(TEXT("GraphNode,%s,%s"), *LastOuter->GetFullName(), *GraphNode->NodeGuid.ToString());

	SetupErrorReporting();

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	PopulateMetaTag(&TagMeta);

	CachedFontSize = CommentNode->GetFontSize();

	CommentStyle = FAppStyle::Get().GetWidgetStyle<FInlineEditableTextBlockStyle>("Graph.CommentBlock.TitleInlineEditableText");
	CommentStyle.EditableTextBoxStyle.TextStyle.Font.Size = CachedFontSize;
	CommentStyle.TextStyle.Font.Size = CachedFontSize;

	this->ContentScale.Bind( this, &SGraphNode::GetContentScale );
	this->GetOrAddSlot( ENodeZone::Center )
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(MainPanel, SBorder)
			.BorderImage( FK2PostItStyle::GetImageBrush(K2PostItBrushes.Border_K2PostItNode))
			.ColorAndOpacity( FLinearColor::White )
			.BorderBackgroundColor( this, &SGraphNodeK2PostIt::GetCommentBodyColor )
			.ForegroundColor(K2PostItColor::DarkGray)
			.Padding(  FMargin(3.0f) )
			.AddMetaData<FGraphNodeMetaData>(TagMeta)
			[
				SNew(SVerticalBox)
				.ToolTipText( this, &SGraphNode::GetNodeTooltip )
				+SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					SAssignNew(TitleBar, SBorder)
					//.BorderImage( FAppStyle::GetBrush("Graph.Node.TitleBackground") )
					.BorderImage( FAppStyle::GetBrush("NoBorder") )
					.BorderBackgroundColor(K2PostItColor::White)
					.Padding( FMargin(10,5,5,3) )
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					[
						SAssignNew(InlineEditableText, SInlineEditableTextBlock)
						.Style( &CommentStyle )
						.ColorAndOpacity(K2PostItColor::DarkGray)
						.ShadowOffset(0)
						.ShadowColorAndOpacity(K2PostItColor::Transparent)
						.Text( this, &SGraphNodeK2PostIt::GetEditableNodeTitleAsText )
						.OnVerifyTextChanged(this, &SGraphNodeK2PostIt::OnVerifyNameTextChanged)
						.OnTextCommitted(this, &SGraphNodeK2PostIt::OnNameTextCommited)
						.IsReadOnly( this, &SGraphNodeK2PostIt::IsNameReadOnly )
						.IsSelected( this, &SGraphNodeK2PostIt::IsSelectedExclusively )
						.WrapTextAt( this, &SGraphNodeK2PostIt::GetWrapAt )
						.MultiLine(true)
						.ModiferKeyForNewLine(EModifierKey::Shift)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2.0f)
				[
					SNew(SSeparator)
					.Thickness(2)
					.ColorAndOpacity(K2PostItColor::DarkGray_Trans)
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(1.0f)
				[
					ErrorReporting->AsWidget()
				]
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex_Lambda( [this] ()
					{
						return HasAnyUserFocusOrFocusedDescendants() || IsSelectedExclusively() || CommentTextSource->HasKeyboardFocus() ? 0 : 1;
					} )
					+ SWidgetSwitcher::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SBorder)
						.BorderImage( FAppStyle::GetBrush("NoBorder") )
						.ForegroundColor(K2PostItColor::DarkGray)
						.VAlign(VAlign_Fill)
						.Padding(4, 8, 4, 8)
						[
							SAssignNew(CommentTextSource, SMultiLineEditableText)
							.TextStyle(FK2PostItStyle::Get(), K2PostItStyles.TextStyle_Editor)
							.Text(this, &SGraphNodeK2PostIt::GetCommentText)
							.OnTextCommitted(this, &SGraphNodeK2PostIt::OnPostItCommentTextCommitted)
						]
					]
					+ SWidgetSwitcher::Slot()
					.Padding(8)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(FormattedTextPanel, SVerticalBox)
					]
				]
			]
		];

	// Create comment bubble
	CommentBubble = SNew(SCommentBubble)
	.GraphNode(GraphNode)
	.Text(this, &SGraphNodeK2PostIt::GetNodeComment)
	.OnTextCommitted(this, &SGraphNodeK2PostIt::OnNameTextCommited)
	.ColorAndOpacity(this, &SGraphNodeK2PostIt::GetCommentBubbleColor )
	.AllowPinning(true)
	.EnableTitleBarBubble(false)
	.EnableBubbleCtrls(false)
	.GraphLOD(this, &SGraphNode::GetCurrentLOD)
	.InvertLODCulling(true)
	.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
	.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
	.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
	.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
	.VAlign(VAlign_Top)
	[
		CommentBubble.ToSharedRef()
	];

	RebuildRichText();
}

FVector2D SGraphNodeK2PostIt::ComputeDesiredSize( float ) const
{
	//float Height = TitleBar->GetDesiredSize().Y + ErrorReporting->AsWidget()->GetDesiredSize().Y + FormattedTextPanel->GetDesiredSize().Y;
	float Height = MainPanel->GetDesiredSize().Y;
	
	return FVector2D(UserSize.X, Height);
}

FString SGraphNodeK2PostIt::GetNodeComment() const
{
	const FString Title = GetEditableNodeTitle();;
	return Title;
}

FReply SGraphNodeK2PostIt::OnMouseButtonDoubleClick( const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent )
{
	// If user double-clicked in the title bar area
	if(FindMouseZone(InMyGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition())) == CRWZ_TitleBar && IsEditable.Get())
	{
		// Request a rename
		RequestRename();

		// Set the keyboard focus
		if(!HasKeyboardFocus())
		{
			FSlateApplication::Get().SetKeyboardFocus(SharedThis(this), EFocusCause::SetDirectly);
		}

		return FReply::Handled();
	}
	else
	{
		// Otherwise let the graph handle it, to allow spline interactions to work when they overlap with a comment node
		return FReply::Unhandled();
	}
}

FReply SGraphNodeK2PostIt::OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent )
{
	if ( (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) && bUserIsDragging )
	{
		bUserIsDragging = false;

		// End resize transaction
		ResizeTransactionPtr.Reset();

		// Update contained child Nodes
		HandleSelection( bIsSelected, true );

		return FReply::Handled().ReleaseMouseCapture();
	}
	else
	{
		return FReply::Handled().ReleaseMouseCapture();
	}
	//return FReply::Unhandled();
}

int32 SGraphNodeK2PostIt::GetSortDepth() const
{
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>( GraphNode );
	return CommentNode ? CommentNode->CommentDepth : -1;
}

void SGraphNodeK2PostIt::HandleSelection(bool bSelected, bool bUpdateNodesUnderComment) const
{
	const FVector2D NodeSize = GetDesiredSize();
	// we only want to do this after the comment has a valid desired size
	if( !NodeSize.IsZero() )
	{
		bIsSelected = bSelected;
	}
}

const FSlateBrush* SGraphNodeK2PostIt::GetShadowBrush(bool bSelected) const
{
	HandleSelection(bSelected);
	
	return bSelected
		? FK2PostItStyle::GetImageBrush(K2PostItBrushes.SelectionShadow_K2PostItNode)
		: FK2PostItStyle::GetImageBrush(K2PostItBrushes.Shadow_K2PostItNode);

	//return SGraphNode::GetShadowBrush(bSelected);
}

void SGraphNodeK2PostIt::GetOverlayBrushes(bool bSelected, const FVector2D WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const
{
	/*
	const float Fudge = 3.0f;

	HandleSelection(bSelected);

	FOverlayBrushInfo HandleBrush = FAppStyle::GetBrush( TEXT("Graph.Node.Comment.Handle") );

	HandleBrush.OverlayOffset.X = WidgetSize.X - HandleBrush.Brush->ImageSize.X - Fudge;
	HandleBrush.OverlayOffset.Y = WidgetSize.Y - HandleBrush.Brush->ImageSize.Y - Fudge;

	Brushes.Add(HandleBrush);
	*/
	return SGraphNode::GetOverlayBrushes(bSelected, WidgetSize, Brushes);
}

void SGraphNodeK2PostIt::MoveTo( const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty)
{
	SGraphNode::MoveTo(NewPosition, NodeFilter, bMarkDirty);
}

void SGraphNodeK2PostIt::EndUserInteraction() const
{
	// Find any parent comments and their list of child nodes
	const FVector2D NodeSize = GetDesiredSize();
	if( !NodeSize.IsZero() )
	{
		const FVector2D NodePosition = GetPosition();
		const FSlateRect CommentRect( NodePosition.X, NodePosition.Y, NodePosition.X + NodeSize.X, NodePosition.Y + NodeSize.Y );

		TSharedPtr<SGraphPanel> Panel = GetOwnerPanel();
		FChildren* PanelChildren = Panel->GetAllChildren();
		int32 NumChildren = PanelChildren->Num();
		static FString SGraphNodeK2PostItType = "SGraphNodeK2PostIt";

		for ( int32 NodeIndex=0; NodeIndex < NumChildren; ++NodeIndex )
		{
			const TSharedPtr<SGraphNode> SomeNodeWidget = StaticCastSharedRef<SGraphNode>(PanelChildren->GetChildAt(NodeIndex));

			UObject* GraphObject = SomeNodeWidget->GetObjectBeingDisplayed();
			if ( !GraphObject->IsA<UEdGraphNode_K2PostIt>() )
			{
				continue;
			}

			const FVector2D SomeNodePosition = SomeNodeWidget->GetPosition();
			const FVector2D SomeNodeSize = SomeNodeWidget->GetDesiredSize();

			const FSlateRect NodeGeometryGraphSpace(SomeNodePosition.X, SomeNodePosition.Y, SomeNodePosition.X + SomeNodeSize.X, SomeNodePosition.Y + SomeNodeSize.Y);
			if (FSlateRect::DoRectanglesIntersect(CommentRect, NodeGeometryGraphSpace))
			{
				// This downcast *should* be valid at this point, since we verified the GraphObject is a comment node
				TSharedPtr<SGraphNodeK2PostIt> CommentWidget = StaticCastSharedPtr<SGraphNodeK2PostIt>(SomeNodeWidget);
				CommentWidget->HandleSelection(CommentWidget->bIsSelected, true);
			}
		}
	}	
}

float SGraphNodeK2PostIt::GetTitleBarHeight() const
{
	return TitleBar.IsValid() ? TitleBar->GetDesiredSize().Y : 0.0f;
}

FSlateRect SGraphNodeK2PostIt::GetHitTestingBorder() const
{
	return SCommentNodeDefs::HitResultBorderSize;
}

FVector2D SGraphNodeK2PostIt::GetNodeMaximumSize() const
{
	return FVector2D( UserSize.X + 100, UserSize.Y + 100 );
}

FSlateColor SGraphNodeK2PostIt::GetCommentBodyColor() const
{
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode);

	if (CommentNode)
	{
		return CommentNode->CommentColor;
	}
	else
	{
		return FLinearColor::White;
	}
}

FSlateColor SGraphNodeK2PostIt::GetCommentTitleBarColor() const
{
	return K2PostItColor::Noir_Glass;

	/*
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode);
	if (CommentNode)
	{
		const FLinearColor Color = CommentNode->CommentColor * SCommentNodeDefs::TitleBarColorMultiplier;
		return FLinearColor(Color.R, Color.G, Color.B);
	}
	else
	{
		const FLinearColor Color = FLinearColor::White * SCommentNodeDefs::TitleBarColorMultiplier;
		return FLinearColor(Color.R, Color.G, Color.B);
	}
	*/
}

FSlateColor SGraphNodeK2PostIt::GetCommentBubbleColor() const
{
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode);
	if (CommentNode)
	{
		const FLinearColor Color = CommentNode->bColorCommentBubble ?	(CommentNode->CommentColor * SCommentNodeDefs::TitleBarColorMultiplier) :
																		GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;
		return FLinearColor(Color.R, Color.G, Color.B);
	}
	else
	{
		const FLinearColor Color = FLinearColor::White * SCommentNodeDefs::TitleBarColorMultiplier;
		return FLinearColor(Color.R, Color.G, Color.B);
	}
}

FText SGraphNodeK2PostIt::GetCommentText() const
{
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode);
	if (CommentNode)
	{
		return CommentNode->CommentText;
	}

	return FText::GetEmpty();
}

void SGraphNodeK2PostIt::OnPostItCommentTextCommitted(const FText& Text, ETextCommit::Type Arg)
{
	UEdGraphNode_K2PostIt* CommentNode = Cast<UEdGraphNode_K2PostIt>(GraphNode);
	if (CommentNode)
	{
		FScopedTransaction Transaction(TEXT("K2PostIt"), LOCTEXT("Transaction_ChangeCommentText", "Change Comment Text"), CommentNode);
		CommentNode->SetCommentText(Text);
		CommentNode->Modify();
	}

	RebuildRichText();
}

FSlateWidgetRun::FWidgetRunInfo SGraphNodeK2PostIt::GetWidgetThing(const FTextRunInfo& RunInfo, const ISlateStyle* Style)
{
	return FSlateWidgetRun::FWidgetRunInfo(SNew(STextBlock).Text(INVTEXT("HELLO WORLD")), 5);
}

bool SGraphNodeK2PostIt::CanBeSelected(const FVector2D& MousePositionInNode) const
{
	const EResizableWindowZone InMouseZone = FindMouseZone(MousePositionInNode);
	return CRWZ_TitleBar == InMouseZone;
}

FVector2D SGraphNodeK2PostIt::GetDesiredSizeForMarquee() const
{
	const float TitleBarHeight = TitleBar.IsValid() ? TitleBar->GetDesiredSize().Y : 0.0f;
	return FVector2D(UserSize.X, TitleBarHeight);
}

FSlateRect SGraphNodeK2PostIt::GetTitleRect() const
{
	const FVector2D NodePosition = GetPosition();
	FVector2D NodeSize  = TitleBar.IsValid() ? TitleBar->GetDesiredSize() : GetDesiredSize();
	return FSlateRect( NodePosition.X, NodePosition.Y, NodePosition.X + NodeSize.X, NodePosition.Y + NodeSize.Y ) + SCommentNodeDefs::TitleBarOffset;
}

void SGraphNodeK2PostIt::PopulateMetaTag(FGraphNodeMetaData* TagMeta) const
{
	if (GraphNode != nullptr)
	{
		// We want the name of the blueprint as our name - we can find the node from the GUID
		UObject* Package = GraphNode->GetOutermost();
		UObject* LastOuter = GraphNode->GetOuter();
		while (LastOuter->GetOuter() != Package)
		{
			LastOuter = LastOuter->GetOuter();
		}
		TagMeta->Tag = FName(*FString::Printf(TEXT("GraphNode_%s_%s"), *LastOuter->GetFullName(), *GraphNode->NodeGuid.ToString()));
		TagMeta->OuterName = LastOuter->GetFullName();
		TagMeta->GUID = GraphNode->NodeGuid;
		TagMeta->FriendlyName = FString::Printf(TEXT("%s in %s"), *GraphNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString(), *TagMeta->OuterName);
	}
}

#undef LOCTEXT_NAMESPACE