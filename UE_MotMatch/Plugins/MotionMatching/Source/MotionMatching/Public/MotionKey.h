// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "MotionStructs.h"
#include "Animation/AnimSequence.h"

#include "MotionKeyUtils.h"

#include "MotionKey.generated.h"

/**
 * 
 */

class UAnimSequence;

USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FMotionKey 
{
	GENERATED_BODY()
		;
public:

	FMotionKey();

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PoseData")
			int32 SrcAnimIndex;
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PoseData")
			float StartTime;

		/////////////////////////// FIRST EVALUATION DATA
		

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
			TArray <uint8> TagsIdx;
		
		////------------Here Store store the data of the specific bones whe want to look for in evaluation, tipically the feet.
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KeyBoneMotionData")
			TArray <FJointData> MotionJointData;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KeyBoneMotionData")
			FVector PresentVel;
		///////////////////////////

		//////////////////////////FUTURE KEY Data
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FutureMotionData")
			FTrajectoryData FutureTrajectory;

		////////////////////////SECOND EVALUATION DATA
		
		//// Here evaluate the current Key against candidate keys/////--------Gotta Take this and put it elswhere and make it static at some point
		float ComputeMotionKeyCost(const float Responsiveness, const float VelocityStrength, const float PoseStrength, const FTrajectoryData DesiredTrajectory, const TArray <FJointData> PresentJointData, const FVector PresentVel);


#if WITH_EDITOR
		//USE THIS WHEN REMOVING KEYS FROM THE EDITOR TO REBAKE THE NEXT KEYS INDEX
		UPROPERTY(BlueprintReadOnly, Category = "MotionKeyHandle")
			FName SrcAnimationName;
#endif //WITH_EDITOR

		
		//////////////////////////

		////////////-------------This is the function used to store the precomputed data from the designated sequence.
		void ExtractDataFromAnimation(const UAnimSequence * InSequence, const int AtSrcAnimIndex, const float AtSrcStartTime, TArray <FName> InMotionBoneNames);


		FORCEINLINE bool operator==(const FMotionKey Other) const
		{
			return (SrcAnimIndex == Other.SrcAnimIndex) && (StartTime == Other.StartTime);
		}
};
