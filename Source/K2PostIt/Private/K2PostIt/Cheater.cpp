#include "K2PostIt/Cheater.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2PostIt/EdGraphNode_K2PostIt.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

#include "K2PostIt/BMPrivateAccess.h"

DEFINE_PRIVATE_MEMBER_ACCESSOR(FBlueprintActionDatabaseRegistrar, GeneratingClass, TSubclassOf<UEdGraphNode>);

void UK2PostItCheater::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(UEdGraphNode_K2PostIt::StaticClass());

	check(NodeSpawner);

	auto CustomizeMessageNodeLambda = [] (UEdGraphNode* NewNode, bool bIsTemplateNode)
	{
		UEdGraphNode_K2PostIt* PostItNode = CastChecked<UEdGraphNode_K2PostIt>(NewNode);

		UEdGraph* OuterGraph = NewNode->GetGraph();
		check(OuterGraph != nullptr);
		UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(OuterGraph);
		check(Blueprint != nullptr);

		const float OldNodePosX = static_cast<float>(NewNode->NodePosX);
		const float OldNodePosY = static_cast<float>(NewNode->NodePosY);
		const float OldHalfHeight = NewNode->NodeHeight / 2.f;
		const float OldHalfWidth  = NewNode->NodeWidth  / 2.f;
		
		static const float DocNodePadding = 50.0f;
		FSlateRect Bounds(OldNodePosX - OldHalfWidth, OldNodePosY - OldHalfHeight, OldNodePosX + OldHalfWidth, OldNodePosY + OldHalfHeight);
		FKismetEditorUtilities::GetBoundsForSelectedNodes(Blueprint, Bounds, DocNodePadding);
	};

	NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeMessageNodeLambda);

	NodeSpawner->DefaultMenuSignature.MenuName = INVTEXT("Add Comment (K2PostIt)...");

	auto OverrideMenuNameLambda = [] (FBlueprintActionContext const& Context, IBlueprintNodeBinder::FBindingSet const& /*Bindings*/, FBlueprintActionUiSpec* UiSpecOut)
	{
		for (UBlueprint* Blueprint : Context.Blueprints)
		{
			if (FKismetEditorUtilities::GetNumberOfSelectedNodes(Blueprint) > 0)
			{
				UiSpecOut->MenuName = INVTEXT("TODO2");
				break;
			}
		}
	};

	// BlueprintActionDatabase.cpp, BlueprintActionDatabaseImpl::GetNodeSpecificActions is hard coded to require a UK2Node class or UEdGraphNode_Comment::StaticClass() or UEdGraphNode_Documentation::StaticClass()
	// This "cheater" class hooks into the action registration process by being a K2Node, but then hijacks its own registration process to register for UEdGraphNode_K2PostIt instead.
	NodeSpawner->DynamicUiSignatureGetter = UBlueprintNodeSpawner::FUiSpecOverrideDelegate::CreateStatic(OverrideMenuNameLambda);

	TSubclassOf<UEdGraphNode>& ValueRef = FBlueprintActionDatabaseRegistrar_Private::Get_GeneratingClass(ActionRegistrar);
	ValueRef = UEdGraphNode_K2PostIt::StaticClass();
	
	ActionRegistrar.AddBlueprintAction(NodeSpawner);
}
