
#pragma once

#include "GameFramework/Character.h"
#include "MotCharacterMovementComponent.h"
#include "MotCharacter.generated.h"

struct FJointData;



/*Character class that merely implements ExtCharacterMovementComponen to set the Root Motion From the AnimGraph Node Motion Matching*/
UCLASS()
class MOTIONMATCHING_API AMotCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMotCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "OverrideRootMotion")
		virtual void OverrideRootMotion(const FTransform RootMotion);
};
