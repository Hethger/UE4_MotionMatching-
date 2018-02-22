
#pragma once
#include "GameFramework/CharacterMovementComponent.h"
#include "MotCharacterMovementComponent.generated.h"
/**
 * 
 */

struct FRootMotionMovementParams;
/*CharacterMovementComponent Class just Here to be able to override root motion with our own Root Motion Blended from Motion Matching*/
UCLASS()
class MOTIONMATCHING_API UMotCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
protected:
	virtual void PerformMovement(float DeltaSeconds) override;

public:

	UMotCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
	UPROPERTY(Category = "Character Movement: Root Motion", EditAnywhere, BlueprintReadWrite)
		bool AllowRootMotionOverride;
	UPROPERTY(BlueprintReadOnly)
		bool OverrideRootMotion;
	UPROPERTY(BlueprintReadOnly)
		FTransform OverridenRootMotion;
};
