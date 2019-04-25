
#pragma once
#include "AnimGraphNode_Base.h"
#include "AnimNode_MotionMatching.h"
#include "AnimGraphNode_MotionMatching.generated.h"
/**
*
*/
/*class that holds Editor version of the AnimGraph Node Motion Matching, along its tittle, tooltip, Node Color, and the category of the node*/
UCLASS()
class MOTIONMATCHINGEDITOR_API UAnimGraphNode_MotionMatching : public UAnimGraphNode_Base
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, Category = Settings)
		FAnimNode_MotionMatching Node;

	//~ Begin UEdGraphNode Interface.
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UAnimGraphNode_Base Interface
	virtual FString GetNodeCategory() const override;
	//~ End UAnimGraphNode_Base Interface

	UAnimGraphNode_MotionMatching(const FObjectInitializer& ObjectInitializer);
};
