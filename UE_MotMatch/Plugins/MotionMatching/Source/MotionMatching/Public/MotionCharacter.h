// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "MotionField.h"

#include "MotionCharacter.generated.h"

//class UMotionField;
//struct FTrajectoryData;


/*This character does an implementation of motion matching where It plays the animations as Dynamic Anim Montages,
Unfortunatelly it does not produce good results as Montages don't blend well when every frame there's a new Montage blending in on the same slot*/
UCLASS()
class MOTIONMATCHING_API AMotionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMotionCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatching")
		UMotionField* MotionField;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatchingAnimSettings")
		float PlayRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatchingAnimSettings")
		float BlendTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatchingCompute")
		float Responsiveness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatchingCompute")
		float VelocityStrength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatchingCompute")
		float PoseStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatchingMontage")
		FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionMatchingDebug")
		bool DrawDebug;

	/** Event fired when the character changes to a New Motion Key */
	UFUNCTION(BlueprintNativeEvent, Category = "Pawn|Character")
		void OnNewMotionKey(const int32 WinnerIndex, const float Cost);
	virtual void OnNewMotionKey_Implementation(const int32 WinnerIndex, const float Cost);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MotionMatching")
	int32 GetMotionKeyIndex()
	{
		return MotionKeyIndex;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MotionMatching")
		FTrajectoryData GetDesiredTrajectory()
	{
		return DesiredTrajectory;
	}

	UFUNCTION(BlueprintCallable, Category = "MotionMatching")
		void SetDesiredTrajectory(const FTrajectoryData NewDesiredTrajectory)
	{
	    DesiredTrajectory = NewDesiredTrajectory;
	}


	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;

private:

	UPROPERTY()
		int CurrentAnimIndex;
	UPROPERTY()
		float CurrentAnimTime;

	UPROPERTY()
	int32 MotionKeyIndex;

	UPROPERTY()
	FTrajectoryData DesiredTrajectory;
	
	////////////////////////////////////////// Motion DATA
	UPROPERTY()
		TArray <FJointData> JointsData;
	UPROPERTY()
		FVector Vel;
	//////////////////////////////////////////


	void PostMotionFieldChanged();

	void MotionUpdate( const float DT);

	void DrawDebugMotionData(const float LifeTime);

	bool AreSkeletonsCompatible();

	void PlayAnimStartingAtTime(const int AnimIndex, const float AnimTime, const float InBlendTime);

	UAnimSequence* GetAnimAtIndex(const int AnimIndex);

	//void ResetMontages();
};
