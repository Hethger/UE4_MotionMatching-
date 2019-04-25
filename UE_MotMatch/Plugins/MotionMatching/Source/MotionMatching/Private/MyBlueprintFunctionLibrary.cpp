// Fill out your copyright notice in the Description page of Project Settings.

#include "MyBlueprintFunctionLibrary.h"
#include "MotionField.h"

void UMyBlueprintFunctionLibrary::PoseGetTest(const UAnimSequence * InSequence, const float AtTime, const bool bExtractRootMotion, TArray<FTransform>& OutBoneTransforms)
{
	OutBoneTransforms.Empty();
	/*
	FCompactPose CmpPose = FCompactPose();
	FBlendedCurve BldCurve = FBlendedCurve();
	FAnimExtractContext ExtracContex = FAnimExtractContext(AtTime, bExtractRootMotion);
	*/
	int Num = InSequence->GetAnimationTrackNames().Num();
	FTransform OutTM = FTransform::Identity;

	InSequence->GetBoneTransform(OutTM, 0, AtTime, false);

	OutBoneTransforms.Add(OutTM);
}

FTransform UMyBlueprintFunctionLibrary::GetBoneCSTM(const UAnimSequence * InSequence, const float AtTime, const bool bUseRawData, const int BoneIndex)
{

	FTransform OutTM = FTransform::Identity;

	if (InSequence && (BoneIndex != INDEX_NONE))
	{

		InSequence->GetBoneTransform(OutTM, BoneIndex, AtTime, bUseRawData);

		USkeleton* SourceSkeleton = InSequence->GetSkeleton();

		FReferenceSkeleton RefSkel = SourceSkeleton->GetReferenceSkeleton();

		if (RefSkel.IsValidIndex(BoneIndex))
		{
			int CurrentIndex = BoneIndex;

			while ((RefSkel.GetParentIndex(CurrentIndex) != 0) && (RefSkel.GetParentIndex(CurrentIndex) != INDEX_NONE))
			{
				FTransform ParentTM;
				InSequence->GetBoneTransform(ParentTM, RefSkel.GetParentIndex(CurrentIndex), AtTime, bUseRawData);

				OutTM = OutTM * ParentTM;
				CurrentIndex = RefSkel.GetParentIndex(CurrentIndex);
			}
		}
		else
		{
			OutTM = FTransform::Identity;
		}
	}

	return OutTM;
}


FQuat UMyBlueprintFunctionLibrary::GetQuat(const FTransform InTM)
{
	return InTM.GetRotation();
}

bool UMyBlueprintFunctionLibrary::BuildTrajectoryData(FTrajectoryData& OutTrajectory, const TArray<FTransform> PastTMs, const TArray<FVector> Velocities)
{
	return FMotionKeyUtils::MakeTrajectoryData(OutTrajectory, PastTMs, Velocities);
}

int UMyBlueprintFunctionLibrary::EvaluateMotionFieldToData(UMotionField * MotionField, const float Responsiveness, const float PoseStrength, const FTrajectoryData DesiredTrajectory, const FTrajectoryData PresentTrajectory, const TArray<FJointData> PresentJointData, const FVector PresentVel, float & OutLowestCost)
{
	int LowestCostIndex = INDEX_NONE;
	if (MotionField)
	{

		LowestCostIndex = 0;//MotionField->GetLowestCostMotionKey(Responsiveness, PoseStrength, DesiredTrajectory, PresentTrajectory, PresentJointData, PresentVel, OutLowestCost);

	}
	return LowestCostIndex;
}

void UMyBlueprintFunctionLibrary::BuildGoal(FTrajectoryData & OutGoal, const FTransform DesiredTransform, const float TargetUUS, const FTransform RootWorldTM)
{
	FMotionKeyUtils::MakeGoal(OutGoal, DesiredTransform, TargetUUS, RootWorldTM);
}

void UMyBlueprintFunctionLibrary::GetMotionDataFromAnimation(const UAnimSequence * InSequence, const float AtTime, TArray<FName> InMotionBoneNames, FTrajectoryData & OutPresentTrajectory, FTrajectoryData & OutFutureTrajectory, TArray<FJointData>& OutJointData, FTransform& OutPastTMRel)
{
	float TrueTime = FMath::Clamp(AtTime, 1.f, InSequence->SequenceLength);
	FTransform PresentTM;
	InSequence->GetBoneTransform(PresentTM, 0, TrueTime, false);

	FTransform PastTM;
	InSequence->GetBoneTransform(PastTM, 0, TrueTime - 1.f, false);

	OutPastTMRel = PastTM.GetRelativeTransform(PresentTM);

	FMotionKeyUtils::GetDataFromAnimation(InSequence, TrueTime,  InMotionBoneNames, OutPresentTrajectory,  OutFutureTrajectory, OutJointData);
}

float UMyBlueprintFunctionLibrary::CompareAnimFramesPoseCost(const UAnimSequence * InSequence, const TArray <FName> MotionBones, const int32 Frame1, const int32 Frame2)
{
	if (InSequence && (MotionBones.Num() > 0))
	{
		const float Frame1Time = InSequence->GetTimeAtFrame(Frame1);

		FVector Frame1Vel;

		FMotionKeyUtils::GetAnimVelocityAtTime(InSequence, Frame1Time, Frame1Vel);

		TArray <FJointData> Frame1JointData;
		 
		for (int i = 0; i < MotionBones.Num(); i++)
		{
			FJointData NewJointData = FJointData();
			FMotionKeyUtils::GetAnimJointData(InSequence, Frame1Time, MotionBones[i], NewJointData);
			Frame1JointData.Add(NewJointData);
		}

		/////////////////////////////

		const float Frame2Time = InSequence->GetTimeAtFrame(Frame2);

		FVector Frame2Vel;

		FMotionKeyUtils::GetAnimVelocityAtTime(InSequence, Frame2Time, Frame2Vel);

		TArray <FJointData> Frame2JointData;

		for (int i = 0; i < MotionBones.Num(); i++)
		{
			FJointData NewJointData = FJointData();
			FMotionKeyUtils::GetAnimJointData(InSequence, Frame2Time, MotionBones[i], NewJointData);
			Frame2JointData.Add(NewJointData);
		}

		/////////////////////////////////

		float PoseCost = 0.f;

		for (int i = 0; i < MotionBones.Num(); i++)
		{
			PoseCost += Frame1JointData[i].CompareTo(Frame2JointData[i]);
		}

		return PoseCost;

	}
	return -666.f;
}



