// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionKeyUtils.h"
#include "MotionField.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine.h"
#include "AnimationRuntime.h"


const float DeltaTime = 0.1f;

void FMotionKeyUtils::GetAnimVelocityAtTime(const UAnimSequence * InSequence, const float AtTime, FVector& OutVelocity)
{
	//const float DeltaTime = 0.03f;
	/*
	FTransform CurrentTM;
	InSequence->GetBoneTransform(CurrentTM, 0, AtTime, false);
	CurrentTM.InverseTransformVectorNoScale(TmpVelo.GetSafeNormal());
	FTransform PastTM;
	InSequence->GetBoneTransform(PastTM, 0, AtTime - Che, false);
	*/

	FVector TmpVelo = InSequence->ExtractRootMotion(AtTime, DeltaTime, true).GetTranslation();
	
	OutVelocity = TmpVelo.GetSafeNormal() * (TmpVelo.Size() / DeltaTime);

	return;
}

void FMotionKeyUtils::ExtractAnimSmoothedRotation(FQuat & OutRot, const UAnimSequence * InSequence, const float KeyTime)
{
	TArray <float> Weights;
	TArray <FTransform> Rots;
	const float Time = FMath::Clamp(KeyTime - .5f, 0.f, InSequence->SequenceLength);

	for (int32 i = 0; i < 10; i++)
	{
		Rots.Add(InSequence->ExtractRootMotion(Time, 1.f * i, true));
		Weights.Add(0.1f);
	}

	FTransform OutTM;
	FAnimationRuntime::BlendTransformsByWeight(OutTM, Rots, Weights);

	OutRot = OutTM.GetRotation();
}

void FMotionKeyUtils::ExtractAnimTrajectory(FTrajectoryData& OutTrajectoryData, const UAnimSequence * InSequence, const float KeyTime)
{
	if (!InSequence)
	{
		return;
	}

	OutTrajectoryData = FTrajectoryData();

	float EndTime = KeyTime + 1.f;
	float MidTime = KeyTime + 0.5f;

	FTransform KeyTM;
	InSequence->GetBoneTransform(KeyTM, 0, KeyTime, false);

	TArray <FTrajectoryPoint> TrajectoryPoints;

	for (int i = 0; i < 10; i++)
	{

		float TimeDelay = (0.1 * i);
		
		FTransform TrajectoryPointTM = InSequence->ExtractRootMotion(KeyTime, TimeDelay, true);
/*	
		FQuat TMPQuat = FQuat::Identity;
		ExtractAnimSmoothedRotation(TMPQuat, InSequence, KeyTime);
		TrajectoryPointTM.SetRotation(TMPQuat);

		*/
		TrajectoryPoints.Add(FTrajectoryPoint(TrajectoryPointTM, TimeDelay));

	}
	
	OutTrajectoryData.TrajectoryPoints = TrajectoryPoints;
	GetAnimVelocityAtTime(InSequence, KeyTime + 1.f, OutTrajectoryData.EndVel);
	//GetAnimVelocityAtTime(InSequence, EndTime, KeyTM, OutTrajectoryData.EndVel);
	
}

void FMotionKeyUtils::GetAnimBoneLocalTM(const UAnimSequence * InSequence, const float AtTime, const int BoneIndex, FTransform & OutTM)
{

	OutTM = FTransform::Identity;

	bool bUseRawData = false;

	if (InSequence && (BoneIndex != INDEX_NONE))
	{

		InSequence->GetBoneTransform(OutTM, BoneIndex, AtTime, bUseRawData);

		USkeleton* SourceSkeleton = InSequence->GetSkeleton();

		FReferenceSkeleton RefSkel = SourceSkeleton->GetReferenceSkeleton();

		if (RefSkel.IsValidIndex(BoneIndex))
		{
			int CurrentIndex = BoneIndex;
			//(RefSkel.GetParentIndex(CurrentIndex) != 0) && (
			if (CurrentIndex == 0)
			{
				InSequence->GetBoneTransform(OutTM, CurrentIndex, AtTime, bUseRawData);
				return;
			}
			while (RefSkel.GetParentIndex(CurrentIndex) != 0)
			{
				int ParentIndex = RefSkel.GetParentIndex(CurrentIndex);
				FTransform ParentTM;
				InSequence->GetBoneTransform(ParentTM, ParentIndex, AtTime, bUseRawData);

				OutTM = OutTM * ParentTM;
				CurrentIndex = ParentIndex;
				
			}
		}
		else
		{
			OutTM = FTransform::Identity;
		}
	}

}

void FMotionKeyUtils::GetAnimBoneWorldTM(const UAnimSequence * InSequence, const float AtTime, const int BoneIndex, FTransform & OutTM)
{

	OutTM = FTransform::Identity;

	bool bUseRawData = false;

	if (InSequence && (BoneIndex != INDEX_NONE))
	{

		InSequence->GetBoneTransform(OutTM, BoneIndex, AtTime, bUseRawData);

		USkeleton* SourceSkeleton = InSequence->GetSkeleton();

		FReferenceSkeleton RefSkel = SourceSkeleton->GetReferenceSkeleton();

		if (RefSkel.IsValidIndex(BoneIndex))
		{
			int CurrentIndex = BoneIndex;
			//(RefSkel.GetParentIndex(CurrentIndex) != 0) && (
			if (CurrentIndex == 0)
			{
				InSequence->GetBoneTransform(OutTM, CurrentIndex, AtTime, bUseRawData);
				return;
			}
			while (RefSkel.GetParentIndex(CurrentIndex) != INDEX_NONE)
			{
				int ParentIndex = RefSkel.GetParentIndex(CurrentIndex);
				FTransform ParentTM;
				InSequence->GetBoneTransform(ParentTM, ParentIndex, AtTime, bUseRawData);

				OutTM = OutTM * ParentTM;
				CurrentIndex = ParentIndex;

			}
		}
		else
		{
			OutTM = FTransform::Identity;
		}
	}
}

void FMotionKeyUtils::GetAnimBoneLocalVel(const UAnimSequence * InSequence, const float AtTime, const int BoneIndex, FVector & OutVel)
{
	if (InSequence)
	{
		//const float DeltaTime = 0.03f;

		FTransform BoneTM = FTransform::Identity;

		GetAnimBoneLocalTM(InSequence, AtTime, BoneIndex, BoneTM);
		FTransform PastBoneTM = FTransform::Identity;

		GetAnimBoneLocalTM(InSequence, AtTime - DeltaTime, BoneIndex, PastBoneTM);
		////Velocity
		FVector TmpVelo = BoneTM.GetLocation() - PastBoneTM.GetLocation();
		float UUS = TmpVelo.Size() / DeltaTime;
		FVector Vel = TmpVelo.GetSafeNormal() * UUS;

		OutVel = Vel;
	}
}

void FMotionKeyUtils::GetAnimJointData(const UAnimSequence * InSequence, const float AtTime, const FName BoneName, FJointData & OutJointData)
{
	OutJointData = FJointData();
	if (InSequence && (BoneName != NAME_None))
	{

		USkeleton* SourceSkeleton = InSequence->GetSkeleton();

		FReferenceSkeleton RefSkel = SourceSkeleton->GetReferenceSkeleton();

		const int BoneIndex = RefSkel.FindBoneIndex(BoneName);

		//const float DeltaTime = 0.03f;

		bool LocalVel = false;

		if (LocalVel)
		{
			FTransform BoneTM = FTransform::Identity;
			GetAnimBoneLocalTM(InSequence, AtTime, BoneIndex, BoneTM);
			FTransform PastBoneTM = FTransform::Identity;
			GetAnimBoneLocalTM(InSequence, AtTime - DeltaTime, BoneIndex, PastBoneTM);
			
			////Velocity
			FVector TmpVelo = BoneTM.GetLocation() - PastBoneTM.GetLocation();
			float UUS = TmpVelo.Size() / DeltaTime;
			FVector Vel = TmpVelo.GetSafeNormal() * UUS;

			OutJointData.BoneCSPos = BoneTM.GetLocation();

			OutJointData.BoneCSVel = Vel;
		}
		else
		{
			FTransform RootTM;
			InSequence->GetBoneTransform(RootTM, 0, AtTime, false);

			FTransform BoneTM = FTransform::Identity;
			GetAnimBoneWorldTM(InSequence, AtTime, BoneIndex, BoneTM);

			FTransform PastBoneTM = FTransform::Identity;
			GetAnimBoneWorldTM(InSequence, AtTime - DeltaTime, BoneIndex, PastBoneTM);

			////Velocity
			FVector TmpVelo = BoneTM.GetLocation() - PastBoneTM.GetLocation();
			float UUS = TmpVelo.Size() / DeltaTime;

			OutJointData.BoneCSPos = RootTM.InverseTransformPositionNoScale(BoneTM.GetLocation());

			OutJointData.BoneCSVel = RootTM.InverseTransformVectorNoScale(TmpVelo.GetSafeNormal()) * UUS;

		}
	}
}

void FMotionKeyUtils::GetKeyPoseDataFromAnim(const UAnimSequence * InSequence, const float AtTime, const TArray<FName> KeyBones, FKeyPoseData& OutPoseData)
{
	

	if (InSequence)
	{
		FReferenceSkeleton RefSkel = InSequence->GetSkeleton()->GetReferenceSkeleton();

		OutPoseData = FKeyPoseData();

		OutPoseData.RetargetSource = InSequence->RetargetSource;

		for (int i = 0; i < KeyBones.Num(); i++)
		{
			
			FTransform BoneTM = FTransform::Identity;

			bool bUseRawData = false;

			InSequence->GetBoneTransform(BoneTM, RefSkel.FindBoneIndex(KeyBones[i]), AtTime, bUseRawData);

			OutPoseData.BoneTMs.Add(BoneTM);
		}
	}

}

void FMotionKeyUtils::GetDataFromAnimation(const UAnimSequence * InSequence, const float AtTime, const TArray<FName> InMotionBoneNames, FTrajectoryData & OutPresentTrajectory, FTrajectoryData & OutFutureTrajectory, TArray<FJointData>& OutJointData)
{
	if (InSequence)
	{
		    
			for (int i = 0; i < InMotionBoneNames.Num(); i++)
			{
				FJointData NewJointData = FJointData();
				FMotionKeyUtils::GetAnimJointData(InSequence, AtTime, InMotionBoneNames[i], NewJointData);
				OutJointData.Add(NewJointData);
			}

			FMotionKeyUtils::ExtractAnimTrajectory(OutPresentTrajectory, InSequence, AtTime - 1.f);
			FMotionKeyUtils::ExtractAnimTrajectory(OutFutureTrajectory, InSequence, AtTime);

	}
}


float FMotionKeyUtils::CompareTrajectories(const FTrajectoryData TrajectoryData_A, const FTrajectoryData TrajectoryData_B)
{
	FTrajectoryData TD = TrajectoryData_A;
	return TD.CompareTo(TrajectoryData_B, 4);
}

float FMotionKeyUtils::CompareJointDatas(const TArray<FJointData> JointData_A, const TArray<FJointData> JointData_B)
{
	if ((JointData_A.Num() > 0) && (JointData_A.Num() == JointData_B.Num()))
	{
		float PoseCost = 0.f;

		for (int i = 0; i < JointData_A.Num(); i++)
		{
			PoseCost += JointData_A[i].CompareTo(JointData_B[i]);
		}

		return PoseCost;
	}
	return -666.f;
}


bool FMotionKeyUtils::MakeTrajectoryData(FTrajectoryData& OutTrajectory, const TArray<FTransform> PastTMs, const TArray<FVector> Velocities)
{
	if (PastTMs.Num() == 11)
	{

	FTransform PastTM = PastTMs[0];

	OutTrajectory = FTrajectoryData();
	
	for (int i = 0; i < PastTMs.Num(); i++)
	{
		OutTrajectory.TrajectoryPoints.Add(FTrajectoryPoint(PastTMs[i].GetRelativeTransform(PastTM), 0.1f * i));
	}
		
	FTransform Ow = PastTMs[4];
	Ow.Blend(PastTMs[4], PastTMs[5], 0.5f);

			
	return true;
		

	}

	return false;
}

void FMotionKeyUtils::DrawTrajectoryData(const FTrajectoryData Trajectory, const FTransform WorldTM, FPrimitiveDrawInterface * PDI, const FColor InColor, const uint8 Depth)
{
	float TrajectoryThickness = 2.f;
	
	for (int i = 1; i < Trajectory.TrajectoryPoints.Num(); i++)
	{
		FVector Curr = WorldTM.TransformPosition(Trajectory.TrajectoryPoints[i].m_TM.GetTranslation());
		FVector Past = WorldTM.TransformPosition(Trajectory.TrajectoryPoints[i - 1].m_TM.GetTranslation());

		PDI->DrawLine(Curr, Past, InColor, Depth, TrajectoryThickness);
	}


	
	
	/*
	if (MidWorldTM.GetLocation() == WorldTM.GetLocation())
	{
		FTransform MidArrowTM = FTransform(FRotationMatrix::MakeFromX((FVector(0.f) - FVector(0.f, 0.f, 100.f)).GetSafeNormal()).ToQuat());
		MidArrowTM.SetTranslation(MidWorldTM.GetLocation());
		DrawDirectionalArrow(PDI, MidArrowTM.ToMatrixNoScale(), InColor, 0.0f, 20.f, Depth, 2.f);
	}
	else
	{
		FTransform MidArrowTM = FTransform(FRotationMatrix::MakeFromX((MidWorldTM.GetLocation() - WorldTM.GetLocation()).GetSafeNormal()).ToQuat());
		MidArrowTM.SetTranslation(MidWorldTM.GetLocation());
		DrawDirectionalArrow(PDI, MidArrowTM.ToMatrixNoScale(), InColor, 0.0f, 20.f, Depth, 2.f);
	}

	if (EndWorldTM.GetLocation() == MidWorldTM.GetLocation())
	{
		FTransform EndArrowTM = FTransform(FRotationMatrix::MakeFromX((FVector(0.f) - FVector(0.f, 0.f, 100.f)).GetSafeNormal()).ToQuat());
		EndArrowTM.SetTranslation(EndWorldTM.GetLocation());
		DrawDirectionalArrow(PDI, EndArrowTM.ToMatrixNoScale(), InColor, 0.0f, 20.f, Depth, 2.f);

	}
	else
	{
		FTransform EndArrowTM = FTransform(FRotationMatrix::MakeFromX((EndWorldTM.GetLocation() - MidWorldTM.GetLocation()).GetSafeNormal()).ToQuat());
		EndArrowTM.SetTranslation(EndWorldTM.GetLocation());
		DrawDirectionalArrow(PDI, EndArrowTM.ToMatrixNoScale(), InColor, 0.0f, 20.f, Depth, 2.f);
	}
	*/
}

void FMotionKeyUtils::BlendKeyPoses(const FKeyPoseData PoseA, const FKeyPoseData PoseB, const float Alpha, FKeyPoseData & OutBlendedKeyPose)
{
	if ((PoseA.BoneTMs.Num() > 0) && (PoseB.BoneTMs.Num() > 0))
	{
		if (PoseA.BoneTMs.Num() == PoseB.BoneTMs.Num())
		{
			
			OutBlendedKeyPose = PoseA;

			for (int i = 0; i < PoseA.BoneTMs.Num(); i++)
			{
				OutBlendedKeyPose.BoneTMs[i].Blend(PoseA.BoneTMs[i], PoseB.BoneTMs[i], Alpha);
			}
		}
	}
}

void FMotionKeyUtils::MakeGoal(FTrajectoryData & OutGoal, const FTransform DesiredTransform, const float TargetUUS, const FTransform RootWorldTM)
{
	OutGoal = FTrajectoryData();

	FTransform CurrentTM; 
	
	for (int i = 0; i < 10; i++)
	{
		float Alpho = ((float)(i + 1)) / ((float)10);

		CurrentTM.Blend(RootWorldTM, DesiredTransform, Alpho);

		FTransform PointTM = CurrentTM;
		//PointTM.SetTranslation(RootWorldTM.InverseTransformPosition(RootWorldTM.GetLocation() + (Dir * (TargetUUS * Alpho))));

		
		OutGoal.TrajectoryPoints.Add(FTrajectoryPoint(PointTM.GetRelativeTransform(RootWorldTM), 0.1f * i));

	}

	const FVector TrueDesDir = RootWorldTM.InverseTransformVectorNoScale(CurrentTM.GetRotation().GetForwardVector());

	OutGoal.EndVel = TrueDesDir * TargetUUS;


}
