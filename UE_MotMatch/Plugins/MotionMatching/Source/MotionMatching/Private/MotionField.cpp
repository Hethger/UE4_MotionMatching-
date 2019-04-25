
#include "MotionField.h"
#include "MotionMatching.h"
#include "AnimationRuntime.h"



UMotionField::UMotionField(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	TimeStep = 0.1f;
	Skeleton = NULL;
	Tags.Empty();
	
	MotionKeys.Empty();

	MotionBones.Empty();

	
	

}

///////////////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UMotionField::ResetTagHelperRanges()
{
	for (int32 i = 0; i < TagHelpers.Num(); i++)
	{
		TagHelpers[i].Ranges.Empty();
		TagHelpers[i].Ranges.SetNum(Tags.Num());
	}
}
void UMotionField::ProcessTagHelpers()
{
	if (TagHelpers.Num() > 0)
	{
		for (int32 k = 0; k < TagHelpers.Num(); k++)
		{

			ResetTagsInAnim(k);

			for (int32 i = 0; i < TagHelpers.Num(); i++)
			{
				for (int32 l = 0; l < TagHelpers[i].Ranges.Num(); l++)
				{
					TArray <float> CurrentRanges = TagHelpers[i].Ranges[l].RangeKeys;
					if (CurrentRanges.Num() > 1)
					{
						while (CurrentRanges.Num() > 1)
						{
							float Start = FMath::Min(CurrentRanges);
							CurrentRanges.Remove(Start);

							float End = FMath::Min(CurrentRanges);
							CurrentRanges.Remove(End);

							ApplyTagsToKeysInAnimRange(k, Start, End, l);
						}

					}
				}
			}
		}
	}
}
#endif //WITH_EDITOR

int UMotionField::GetLowestCostMotionKey
(
	const float Responsiveness, 
	const float VelocityStrength, 
	const float PoseStrength, 
	const FTrajectoryData DesiredTrajectory, 
	const TArray <FJointData> PresentJointData, 
	const FVector PresentVel, 
	const TArray <uint8> TagsToLookFor,
	const TArray <uint8> TagsToIgnore, 
	float & OutLowestCost
)
{
	int WinnerIndex = INDEX_NONE;
	float LowestCost = 1000000000.f;

	if (MotionKeys.Num() > 0)
	{

		for (int i = 0; i < MotionKeys.Num(); i++)
		{
			if ((!CheckMotionKeyAgainstTags(i, TagsToLookFor)) && (TagsToLookFor.Num() > 0))
			{
				continue;
			}

			if (CheckMotionKeyAgainstTags(i, TagsToIgnore))
			{
				continue;
			}

			float CurrentCost = MotionKeys[i].ComputeMotionKeyCost(Responsiveness, VelocityStrength, PoseStrength, DesiredTrajectory, PresentJointData, PresentVel);


			if (CurrentCost < LowestCost)
			{
				WinnerIndex = i;
				LowestCost = CurrentCost;
			}

		}

	}
	OutLowestCost = LowestCost;

	return WinnerIndex;
}
bool UMotionField::CheckMotionKeyAgainstTags(const int32 MotionKeyIdx, const TArray<uint8> TagsIdx)
{
	if ((TagsIdx.Num() > 0) && (MotionKeys[MotionKeyIdx].TagsIdx.Num() > 0))
	{
		for (int32 j = 0; j < TagsIdx.Num(); j++)
		{
			for (int32 k = 0; k < MotionKeys[MotionKeyIdx].TagsIdx.Num(); k++)
			{
				if (TagsIdx[j] == MotionKeys[MotionKeyIdx].TagsIdx[k])
				{
					return true;
				}
			}
		}

	};

	return false;
}
#if WITH_EDITOR
void UMotionField::AddRangeToTagHelper(const float RangeTime, const uint8 AtTagIdx, const int32 AtAnimIdx)
{
	
	check(TagHelpers.IsValidIndex(AtAnimIdx))
	check(TagHelpers[AtAnimIdx].Ranges.IsValidIndex(AtTagIdx))

	TagHelpers[AtAnimIdx].Ranges[AtTagIdx].RangeKeys.Add(RangeTime);
	
}

void UMotionField::RemoveRangeFromTagHelper(const float RangeTime, const uint8 AtTagIdx, const int32 AtAnimIdx)
{

	check(TagHelpers.IsValidIndex(AtAnimIdx));
	check(TagHelpers[AtAnimIdx].Ranges.IsValidIndex(AtTagIdx))

	TagHelpers[AtAnimIdx].Ranges[AtTagIdx].RangeKeys.Remove(RangeTime);
		
}
bool UMotionField::IsTimeTagged(const float RangeTime, const uint8 AtTagIdx, const int32 AtAnimIdx)
{
	return TagHelpers[AtAnimIdx].Ranges[AtTagIdx].RangeKeys.Contains(RangeTime);
}
#endif //WITH_EDITOR

void UMotionField::ResetTagsInAnim(const int32 AnimIdx)
{
	if (MotionKeys.Num() > 0)
	{
		for (int32 i = 0; i < MotionKeys.Num(); i++)
		{
			if (MotionKeys[i].SrcAnimIndex == AnimIdx)
			{
				
				MotionKeys[i].TagsIdx.Empty();
				
			}
		}
	}
}

void UMotionField::ApplyTagsToKeysInAnimRange(const int32 AnimIdx, const float Start, const float End, const uint8 TagIdx)
{
	if (MotionKeys.Num() > 0)
	{
		for (int32 i = 0; i < MotionKeys.Num(); i++)
		{
			if (MotionKeys[i].SrcAnimIndex == AnimIdx)
			{
				bool InRange = (MotionKeys[i].StartTime <= End) && (MotionKeys[i].StartTime >= Start);
				
				if (InRange)
				{
					MotionKeys[i].TagsIdx.Add(TagIdx);
				}
			}
		}
	}
}

FString UMotionField::GetTagAtIndex(const int32 TagIdx) const
{
	if (Tags.IsValidIndex(TagIdx))
	{
		return Tags[TagIdx];
	}

	return FString();
}

void UMotionField::GetMotionFieldProperties(float & OutTimeStep, TArray<FString> & OutTags ) const
{
	OutTags = Tags;
	OutTimeStep = TimeStep;
}

void UMotionField::SetProperties(const float InTimeStep, const TArray<FString> InTags)
{
	Modify();
	Tags = InTags;
	TimeStep = InTimeStep;

#if WITH_EDITOR
	ResetTagHelperRanges();
#endif //WITH_EDITOR

	RebakeAllAnim();

	MarkPackageDirty();
}

void UMotionField::ClearAnimMotionKeysUtil(const int AtSourceAnimIndex)
{
	if (MotionKeys.Num() > 0)
	{
		for (int i = MotionKeys.Num() - 1; i > -1; i--)
		{
			if (MotionKeys[i].SrcAnimIndex == AtSourceAnimIndex)
			{
				MotionKeys.RemoveAt(i);
			}
		}
	}
}
void UMotionField::ClearAllMotionKeys()
{
	Modify();
	
	MotionKeys.Empty();
	/*
	const int Num = GetSrcAnimNum();
	for (int i = 0; i < Num; i++)
	{
		ClearAnimMotionKeysUtil(i);
	}
	*/
	MarkPackageDirty();
}
void UMotionField::ClearAnimMotionKeys(const int AtSourceAnimIndex)
{
	Modify();

	ClearAnimMotionKeysUtil(AtSourceAnimIndex);

	MarkPackageDirty();
}


void UMotionField::RebakeMotionKeysInAnim(const int  FromSourceAnimation)
{

	if (SourceAnimations[FromSourceAnimation])
	{
		Modify();

		ClearAnimMotionKeysUtil(FromSourceAnimation);
		
		const float AnimLength = SourceAnimations[FromSourceAnimation]->GetPlayLength();
		float AccumulatedTime = 0.f;

		while (AccumulatedTime <= AnimLength)
		{
			///////////////////////////EXTRACT MOTION KEY PART

			bool CanKey = (AccumulatedTime >= 1.f) && (AccumulatedTime <= (AnimLength - 1.f));

			if (CanKey)
			{
				////////////   ///////////////////////////

					FMotionKey NewMotionKey = FMotionKey();

					NewMotionKey.ExtractDataFromAnimation(SourceAnimations[FromSourceAnimation], FromSourceAnimation, AccumulatedTime, MotionBones);

					MotionKeys.Add(NewMotionKey);
			}

			AccumulatedTime += TimeStep;

		}

		MarkPackageDirty();
	}
}

void UMotionField::RebakeAllAnim()
{
	ClearAllMotionKeys();
	if (SourceAnimations.Num() > 0)
	{
		for (int32 i = 0; i < SourceAnimations.Num(); i++)
		{
			RebakeMotionKeysInAnim(i);
		}

	}
}

void UMotionField::AddSrcAnim(UAnimSequence * NewAnim)
{
	Modify();
	SourceAnimations.Add(NewAnim);

#if WITH_EDITOR
	FTagHelper NewHelper;
	NewHelper.AnimName = NewAnim->GetFName();
	NewHelper.Ranges.SetNum(Tags.Num());
	TagHelpers.Add(NewHelper);
#endif //WITH_EDITOR

	RebakeAllAnim();
	MarkPackageDirty();
}

void UMotionField::DeleteSrcAnim(const int AtIndex)
{
	Modify();

#if WITH_EDITOR
	for (int32 i = TagHelpers.Num() - 1; i > -1; i--)
	{
		if(TagHelpers[i].AnimName == SourceAnimations[AtIndex]->GetFName())
		{
			TagHelpers.RemoveAt(i);
		}
	}
#endif //WITH_EDITOR

	SourceAnimations.RemoveAt(AtIndex);
	
	RebakeAllAnim();

	MarkPackageDirty();
}

bool UMotionField::IsExtractedFrame(const FName AnimName, const float Time)
{
	if (MotionKeys.Num() > 0)
	{
		for (int i = 0; i < MotionKeys.Num(); i++)
		{
			if (
				(MotionKeys[i].SrcAnimationName == AnimName)
				&&
				(MotionKeys[i].StartTime == Time)
				)
			{
				return true;
			}
		}
	}
	return false;
}


//#endif //WITH_EDITOR