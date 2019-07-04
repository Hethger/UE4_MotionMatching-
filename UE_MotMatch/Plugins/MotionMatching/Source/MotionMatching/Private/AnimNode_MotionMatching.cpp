// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNode_MotionMatching.h"

#include "MotionMatching.h"
#include "Engine/Engine.h"
#include "AnimationRuntime.h"
#include "MotCharacter.h"

#include "DualQuat.h"


bool FMotionAnim::ApplyTime(const float DT, const float InBlendTime, const bool Main)
{
	Position += DT;

	if (((!Main) && Maxed))// || Out)
	{
		BlendTime = FMath::Clamp(BlendTime - DT, 0.f, InBlendTime);
		
		if (BlendTime == 0.f)
		{
			return true;
		}
		
	}
	else
	{
		BlendTime = FMath::Clamp(BlendTime + DT, 0.f, InBlendTime);
	}
	
	if ((BlendTime == InBlendTime) && (!Maxed))
	{
		Maxed = true;
	}
	
	Weight = FMath::Clamp((BlendTime / InBlendTime) , 0.f, 1.f);
	
	
	return false;

}


FAnimNode_MotionMatching::FAnimNode_MotionMatching()
	: MotionField(NULL)
	, DesiredTrajectory(FTrajectoryData())
	, Responsiveness(1.f)
	, VelocityStrength(1.f)
	, PoseStrength(1.f)
	, BlendTime(0.3f)
	, PlayRate(1.f)
	, ChangeTimeLimit(0.1f)
	, MaxChanges(2)
	, ChangeCounter(0)

	, ChangeTimer(0.f)
	, bLoopAnimation(true)
{
	TagsToLookFor.Empty();
	TagsToIgnore.Empty();

	m_Anims.Empty();
}

float FAnimNode_MotionMatching::GetCurrentAssetTime()
{
	return InternalTimeAccumulator;
}

float FAnimNode_MotionMatching::GetCurrentAssetTimePlayRateAdjusted()
{
	UAnimSequence* Sequence = GetCurrentAnim();

	float EffectivePlayrate = PlayRate * (Sequence ? Sequence->RateScale : 1.0f);
	return (EffectivePlayrate < 0.0f) ? GetCurrentAssetLength() - InternalTimeAccumulator : InternalTimeAccumulator;
}

float FAnimNode_MotionMatching::GetCurrentAssetLength()
{
	UAnimSequence* Sequence = GetCurrentAnim();

	return Sequence ? Sequence->SequenceLength : 0.0f;

}

void FAnimNode_MotionMatching::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{

	FAnimNode_Base::Initialize_AnyThread(Context);

	GetEvaluateGraphExposedInputs().Execute(Context);
	
	InternalTimeAccumulator = 0.f;
	if (MotionField)
	{
			UAnimSequence* Sequence = GetCurrentAnim();

			if (Sequence != NULL)
			{
				InternalTimeAccumulator = FMath::Clamp(0.f, 0.f, Sequence->SequenceLength);

				if (0.f == 0.f && (PlayRate * Sequence->RateScale) < 0.0f)
				{
					InternalTimeAccumulator = Sequence->SequenceLength;

				}
			}
	}
}

void FAnimNode_MotionMatching::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{

	//BasePose.CacheBones(Context);
}

void FAnimNode_MotionMatching::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);

	if (MotionField)
	{

			MotionUpdate(Context);

			//Compute();// <-----------Whether to do the compute before or after Evaluate_AnyThread, can't notice much difference so leave it to after Evaluate
			

			if ((GetCurrentAnim() != NULL) && (Context.AnimInstanceProxy->IsSkeletonCompatible(GetCurrentAnim()->GetSkeleton())))
			{
				InternalTimeAccumulator = FMath::Clamp(InternalTimeAccumulator, 0.f, GetCurrentAnim()->SequenceLength);

				CreateTickRecordForNode(Context, GetCurrentAnim(), bLoopAnimation, PlayRate);
			}
		
	}
}


void FAnimNode_MotionMatching::Evaluate_AnyThread(FPoseContext & Output)
{

	if (m_Anims.Num() > 0)
	{
		if ((m_Anims.Num() > 1) && (BlendTime != 0.f))
		{
			FTransform RootMotion;

			GetBlendPose(Output.AnimInstanceProxy->GetDeltaSeconds(), RootMotion, Output.Pose, Output.Curve);
			
			///// Root Motion Setting using this hack for now as I want to make sure I blend Root Motion properly before
			///// Doing a full on implementation
			if (AMotCharacter* Char = Cast<AMotCharacter>(Output.AnimInstanceProxy->GetSkelMeshComponent()->GetOwner()))
			{
				Char->OverrideRootMotion(RootMotion);
			}
			
		}
		else
		{
			GetCurrentAnim()->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(m_Anims.Last().Position, true));
		}
	}
	else
	{
		Output.Pose.ResetToRefPose();
	}

	Compute();

}

void FAnimNode_MotionMatching::OverrideAsset(UAnimationAsset * NewAsset)
{
	if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(NewAsset))
	{
		
	}
}




void FAnimNode_MotionMatching::GatherDebugData(FNodeDebugData & DebugData)
{

	UAnimSequence* Sequence = GetCurrentAnim();

	FString DebugLine = DebugData.GetNodeName(this);
	DebugLine += FString::Printf(TEXT("('%s' Play Time: %.3f)"), Sequence ? *Sequence->GetName() : TEXT("NULL"), InternalTimeAccumulator);
	DebugData.AddDebugItem(DebugLine, true);
	//	BasePose.GatherDebugData(DebugData);
}

void FAnimNode_MotionMatching::PlayAnimStartingAtTime(const int32 AnimIndex, const float StartingTime)
{
	ChangeCounter++;
	InternalTimeAccumulator = StartingTime;

	FMotionAnim NewAnim = FMotionAnim(AnimIndex, StartingTime);
	m_Anims.Add(NewAnim);
	
}

UAnimSequence * FAnimNode_MotionMatching::GetCurrentAnim()
{
	if (m_Anims.Num() > 0)
	{
		return MotionField->GetSrcAnimAtIndex(m_Anims.Last().AnimIndex);
	}
	return NULL;
}

UAnimSequence * FAnimNode_MotionMatching::AnimAtIndex(const int32 Index)
{
	if (m_Anims.Num() > 0)
	{
		return MotionField->GetSrcAnimAtIndex(m_Anims[Index].AnimIndex);
	}
	return NULL;
}



void FAnimNode_MotionMatching::MotionUpdate(const FAnimationUpdateContext& Context)
{

	const float DT = Context.GetDeltaTime();// *GetCurrentAssetTimePlayRateAdjusted();


	/////-------------------- Small prevention, check to see if we changed more than 2 times in less than 0.01 seconds
	if (ChangeCounter > MaxChanges)
	{
		ChangeTimer += DT;
	}
	if (ChangeTimer > ChangeTimeLimit)
	{
		ChangeTimer = 0.f;
		ChangeCounter = 0;
	}
	/////-------------------------------------------------------------------------------------------------------------


	int32 AnimNum = m_Anims.Num();
	
	if (AnimNum > 0)
	{
		int32 StartIdx = AnimNum - 1;

		for (int32 i = StartIdx; i > -1; i--)
		{

			if (m_Anims[i].ApplyTime(DT, BlendTime, i == StartIdx))
			{
				m_Anims.RemoveAt(i);
			}
		}
	}

}

void FAnimNode_MotionMatching::Compute()
{
	if (MotionField)
	{
		if (m_Anims.Num() > 0)
		{
			if (ChangeCounter <= MaxChanges)
			{
				FVector Vel;
				TArray <FJointData> TMPJointData;
				GetMotionData(TMPJointData, Vel);

				float OutCost;

				int Winner = MotionField->GetLowestCostMotionKey(Responsiveness, VelocityStrength, PoseStrength, DesiredTrajectory, TMPJointData, Vel, TagsToLookFor, TagsToIgnore, OutCost);

				if (Winner >= 0)
				{
					bool TheWinnerIsAtTheSameLocation =
						(MotionField->MotionKeys[Winner].SrcAnimIndex == m_Anims.Last().AnimIndex)
						&&
						(fabs(MotionField->MotionKeys[Winner].StartTime - m_Anims.Last().Position) < .2f);

					if (!TheWinnerIsAtTheSameLocation)
					{
						CurrentMotionKeyIndex = Winner;

						PlayAnimStartingAtTime(MotionField->MotionKeys[Winner].SrcAnimIndex, MotionField->MotionKeys[Winner].StartTime);
					}
				}

			}
		}


		else
		{
			PlayAnimStartingAtTime(0, 0.f);
		}

	}
}


void FAnimNode_MotionMatching::GetMotionData(TArray<FJointData>& OutJointData, FVector & Velocity)
{
	FMotionKeyUtils::GetAnimVelocityAtTime(GetCurrentAnim(), m_Anims.Last().Position, Velocity);
	
	OutJointData.SetNum(MotionField->MotionBones.Num());

	for (int32 i = 0; i < OutJointData.Num(); i++)
	{
		FMotionKeyUtils::GetAnimJointData(GetCurrentAnim(), m_Anims.Last().Position, MotionField->MotionBones[i], OutJointData[i]);
	}
}



void FAnimNode_MotionMatching::GetBlendPose(const float DT, FTransform & OutRootMotion, FCompactPose & OutPose, FBlendedCurve & OutCurve)
{
	const int32 NumPoses = m_Anims.Num();
	OutRootMotion = FTransform::Identity;

	if (NumPoses > 0)
	{
		TArray<FCompactPose, TInlineAllocator<8>> ChildrenPoses;
		ChildrenPoses.AddZeroed(NumPoses);

		TArray<FBlendedCurve, TInlineAllocator<8>> ChildrenCurves;
		ChildrenCurves.AddZeroed(NumPoses);

		TArray<float, TInlineAllocator<8>> ChildrenWeights;
		ChildrenWeights.AddZeroed(NumPoses);

		TArray<FTransform> ChildrenRootMotions;
		ChildrenRootMotions.AddZeroed(NumPoses);

		for (int32 ChildrenIdx = 0; ChildrenIdx<ChildrenPoses.Num(); ++ChildrenIdx)
		{
			ChildrenPoses[ChildrenIdx].SetBoneContainer(&OutPose.GetBoneContainer());
			ChildrenCurves[ChildrenIdx].InitFrom(OutCurve);
		}

		// get all child atoms we interested in
		
		float Sum = 0.f;
		
		for (int32 I = 0; I < NumPoses; I++)
		{
				FCompactPose& Pose = ChildrenPoses[I];

				ChildrenWeights[I] = m_Anims[I].Weight * ((((float)(I + 1)) / ((float)NumPoses)));
				Sum += ChildrenWeights[I];

				const float Time = FMath::Clamp<float>(m_Anims[I].Position, 0.f, AnimAtIndex(I)->SequenceLength);

				AnimAtIndex(I)->GetAnimationPose(Pose, ChildrenCurves[I], FAnimExtractContext(Time, true));

				ChildrenRootMotions[I] = AnimAtIndex(I)->ExtractRootMotion(Time - DT, DT, true);

		}
		

		if (Sum > 0.f)
		{

			TArrayView<FCompactPose> ChildrenPosesView(ChildrenPoses);

			for(int32 i = 0; i < ChildrenWeights.Num(); i++)
			{
				ChildrenWeights[i] = ChildrenWeights[i] / Sum;
			}

			FAnimationRuntime::BlendPosesTogether(ChildrenPosesView, ChildrenCurves, ChildrenWeights, OutPose, OutCurve);
			OutPose.NormalizeRotations();

			////-----------------------------------------------------------------------------Root Motion Blending 
			////This is pretty much a hack to check if Root Motion is being blended correctly before wasting time on a full on Implementation

			FRootMotionMovementParams MotMotion;
			MotMotion.Clear();


			for (int j = 0; j < NumPoses; j++)
			{
				MotMotion.AccumulateWithBlend(ChildrenRootMotions[j], ChildrenWeights[j]);
			}

			OutRootMotion = MotMotion.GetRootMotionTransform();
			OutRootMotion.NormalizeRotation();

			////-----------------------------------------------------------------------------
		}
		else
		{
			OutRootMotion = AnimAtIndex(NumPoses - 1)->ExtractRootMotion(m_Anims.Last().Position - DT, DT, true);

			GetCurrentAnim()->GetAnimationPose(OutPose, OutCurve, FAnimExtractContext(m_Anims.Last().Position, true));
		}
		
	}
	else
	{
		GetCurrentAnim()->GetAnimationPose(OutPose, OutCurve, FAnimExtractContext(m_Anims.Last().Position, true));
	}
}

