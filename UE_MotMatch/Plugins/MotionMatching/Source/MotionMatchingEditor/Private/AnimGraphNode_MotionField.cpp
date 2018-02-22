
#include "AnimGraphNode_MotionField.h"
#include "MotionMatchingEditor.h"


#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_MotionField::UAnimGraphNode_MotionField(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

FLinearColor UAnimGraphNode_MotionField::GetNodeTitleColor() const
{
	return FLinearColor::Red;
}

FText UAnimGraphNode_MotionField::GetTooltipText() const
{
	return LOCTEXT("Generates_A_Pose_By_Matching_Against_The_Motion_Keys_Of_A_Motion_Field", "Generates a pose by Matching against the Motion Keys of a Motion Field");
}

FText UAnimGraphNode_MotionField::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Motion_Field", "Motion Field");
}

FString UAnimGraphNode_MotionField::GetNodeCategory() const
{
	return TEXT("Tools");
}

#undef LOCTEXT_NAMESPACE
