// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimGraphNode_MotionMatching.h"
#include "MotionMatchingEditor.h"


#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_MotionMatching::UAnimGraphNode_MotionMatching(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

FLinearColor UAnimGraphNode_MotionMatching::GetNodeTitleColor() const
{
	return FLinearColor::Blue;
}

FText UAnimGraphNode_MotionMatching::GetTooltipText() const
{
	return LOCTEXT("Generates_A_Pose_By_Matching_Against_The_Motion_Keys_Of_A_Motion_Field", "Generates a pose by Matching against the Motion Keys of a Motion Field");
}

FText UAnimGraphNode_MotionMatching::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Motion_Matching", "Motion Matching");
}

FString UAnimGraphNode_MotionMatching::GetNodeCategory() const
{
	return TEXT("Tools");
}

#undef LOCTEXT_NAMESPACE
