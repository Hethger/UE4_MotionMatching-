// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionStructs.generated.h"
/**
 * 
 */



USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FKeyPoseData
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoseData")
		TArray <FTransform> BoneTMs;
	
	    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoseData")
		FName RetargetSource = NAME_None;

	FKeyPoseData()
	{
		BoneTMs.Empty();
		RetargetSource = NAME_None;
	}

	FORCEINLINE bool operator==(const FKeyPoseData Other) const
	{
		if ((BoneTMs.Num() == Other.BoneTMs.Num()) && (RetargetSource == Other.RetargetSource))
		{
			if (BoneTMs.Num() > 0)
			{

				for (int i = 0; i < BoneTMs.Num(); i++)
				{
					if (!(BoneTMs[i].Equals(Other.BoneTMs[i])))
					{
						return false;
					}
				}
				return true;
			}
			else
			{
				return true;
			}
		}
		return false;
	}
};
USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FTrajectoryPoint
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
		FTransform m_TM = FTransform::Identity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory")
		float m_TimeDelay = 0.f;

	FTrajectoryPoint()
	{
		m_TM = FTransform::Identity;
		m_TimeDelay = 0.f;
	}

	FTrajectoryPoint(const FTransform TMIn, const float TimeDelayIn)
	{
		m_TM = TMIn;
		m_TimeDelay = TimeDelayIn;
	}

};

USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FTrajectoryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrajectoryData")
		TArray <FTrajectoryPoint> TrajectoryPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrajectoryData")
		FVector EndVel = FVector::ZeroVector;

	FTrajectoryData()
	{
		TrajectoryPoints.Empty();
		EndVel = FVector::ZeroVector;
	}

	float CompareTo(const FTrajectoryData Other, const int Per) const;
};


USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FJointData
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoseMotionData")
		FVector BoneCSPos = FVector::ZeroVector;

	    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoseMotionData")
		FVector BoneCSVel = FVector::ZeroVector;

	FJointData()
	{

	}

	float CompareTo(const FJointData Other) const;
};


USTRUCT(BlueprintType)
struct MOTIONMATCHING_API FMotionPose
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoseMotionData")
		FTrajectoryData PresentTrajectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoseMotionData")
		FTrajectoryData FutureTrajectory;

	FMotionPose()
	{

	}

	//float CompareTo(const FJointData Other) const;
};
