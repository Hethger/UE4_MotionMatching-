// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionFieldEditor.h"

#include "EditorViewportClient.h"
#include "UObject/Package.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"
#include "SSingleObjectDetailsPanel.h"


#include "SEditorViewport.h"
#include "ScopedTransaction.h"
#include "MotionFieldEditor/MotionFieldEditorViewportClient.h"
#include "MotionFieldEditor/MotionFieldEditorCommands.h"

#include "MotionMatchingEditor.h"

#include "SCommonEditorViewportToolbarBase.h"
#include "MotionFieldEditor/SMotionFieldEditorViewportToolbar.h"
#include "SScrubControlPanel.h"
#include "MotionFieldEditor/SMotionFieldTimeline.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/GenericCommands.h"

#include "Animation/DebugSkelMeshComponent.h"

#include "Classes/AnimPreviewInstance.h"

#include "SContextList.h"

#define LOCTEXT_NAMESPACE "MotionFieldEditor"

//////////////////////////////////////////////////////////////////////////

const FName MotionFieldEditorAppName = FName(TEXT("MotionFieldEditorApp"));

//////////////////////////////////////////////////////////////////////////

struct FMotionFieldEditorTabs
{
	// Tab identifiers
	
	static const FName DetailsID;
	static const FName ViewportID;
};

//////////////////////////////////////////////////////////////////////////

const FName FMotionFieldEditorTabs::DetailsID(TEXT("Details"));
const FName FMotionFieldEditorTabs::ViewportID(TEXT("Viewport"));

//////////////////////////////////////////////////////////////////////////
// SMotionFieldEditorViewport

class SMotionFieldEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SMotionFieldEditorViewport)
		: _MotionFieldBeingEdited((UMotionField*)nullptr)
	{}

	SLATE_ATTRIBUTE(UMotionField*, MotionFieldBeingEdited)

		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, TWeakPtr<FMotionFieldEditor> InMotionFieldEditorPtr);

	// SEditorViewport interface
	virtual void BindCommands() override;
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual EVisibility GetTransformToolbarVisibility() const override;
	virtual void OnFocusViewportToSelection() override;
	// End of SEditorViewport interface

	// ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	// End of ICommonEditorViewportToolbarInfoProvider interface

	UDebugSkelMeshComponent* GetPreviewComponent() const
	{
		return EditorViewportClient->GetPreviewComponent();
	}

private:
	TAttribute<UMotionField*> MotionFieldBeingEdited;
	TWeakPtr<FMotionFieldEditor> MotionFieldEditorPtr;
	// Viewport client
	TSharedPtr<FMotionFieldEditorViewportClient> EditorViewportClient;
};

void SMotionFieldEditorViewport::Construct(const FArguments& InArgs, TWeakPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
{
	MotionFieldBeingEdited = InArgs._MotionFieldBeingEdited;
	MotionFieldEditorPtr = InMotionFieldEditorPtr;
	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SMotionFieldEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	const FMotionFieldEditorCommands& Commands = FMotionFieldEditorCommands::Get();

	TSharedRef<FMotionFieldEditorViewportClient> EditorViewportClientRef = EditorViewportClient.ToSharedRef();

	CommandList->MapAction(
		Commands.SetShowGrid,
		FExecuteAction::CreateSP(EditorViewportClientRef, &FEditorViewportClient::SetShowGrid),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(EditorViewportClientRef, &FEditorViewportClient::IsSetShowGridChecked));

	CommandList->MapAction(
		Commands.SetShowBounds,
		FExecuteAction::CreateSP(EditorViewportClientRef, &FEditorViewportClient::ToggleShowBounds),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(EditorViewportClientRef, &FEditorViewportClient::IsSetShowBoundsChecked));

	CommandList->MapAction(
		Commands.SetShowCollision,
		FExecuteAction::CreateSP(EditorViewportClientRef, &FEditorViewportClient::SetShowCollision),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(EditorViewportClientRef, &FEditorViewportClient::IsSetShowCollisionChecked));

	CommandList->MapAction(
		Commands.SetShowPivot,
		FExecuteAction::CreateSP(EditorViewportClientRef, &FMotionFieldEditorViewportClient::ToggleShowPivot),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(EditorViewportClientRef, &FMotionFieldEditorViewportClient::IsShowPivotChecked));

	CommandList->MapAction(
		Commands.SetShowSockets,
		FExecuteAction::CreateSP(EditorViewportClientRef, &FMotionFieldEditorViewportClient::ToggleShowSockets),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(EditorViewportClientRef, &FMotionFieldEditorViewportClient::IsShowSocketsChecked));
}

TSharedRef<FEditorViewportClient> SMotionFieldEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FMotionFieldEditorViewportClient(MotionFieldBeingEdited, MotionFieldEditorPtr));

	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SMotionFieldEditorViewport::MakeViewportToolbar()
{
	return SNew(SMotionFieldEditorViewportToolbar, SharedThis(this));
}

EVisibility SMotionFieldEditorViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}

void SMotionFieldEditorViewport::OnFocusViewportToSelection()
{
	EditorViewportClient->RequestFocusOnSelection(/*bInstant=*/ false);
}

TSharedRef<class SEditorViewport> SMotionFieldEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SMotionFieldEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SMotionFieldEditorViewport::OnFloatingButtonClicked()
{
}

//////////////////////////////////////////////////////////////////////////
// FMotionFieldEditor

FMotionFieldEditor::FMotionFieldEditor()
	: MotionFieldBeingEdited(nullptr)
{
}

TSharedRef<SDockTab> FMotionFieldEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	ViewInputMin = 0.0f;
	ViewInputMax = GetTotalSequenceLength();
	LastObservedSequenceLength = ViewInputMax;

	//@TODO: PAPER2D: Implement OnBeginSliderMovement/OnEndSliderMovement so that refreshing works, animation stops, etc...
	TSharedPtr<FMotionFieldEditor> MotionFieldEditorPtr = SharedThis(this);


	TSharedRef<SWidget> ScrubControl = SNew(SScrubControlPanel)
		.IsEnabled(true)
		.Value(this, &FMotionFieldEditor::GetPlaybackPosition)
		.NumOfKeys(this, &FMotionFieldEditor::GetTotalFrameCountPlusOne)
		.SequenceLength(this, &FMotionFieldEditor::GetTotalSequenceLength)
		.OnValueChanged(this, &FMotionFieldEditor::SetPlaybackPosition)
		//		.OnBeginSliderMovement(this, &SAnimationScrubPanel::OnBeginSliderMovement)
		//		.OnEndSliderMovement(this, &SAnimationScrubPanel::OnEndSliderMovement)
		.OnClickedForwardPlay(this, &FMotionFieldEditor::OnClick_Forward)
		.OnClickedForwardStep(this, &FMotionFieldEditor::OnClick_Forward_Step)
		.OnClickedForwardEnd(this, &FMotionFieldEditor::OnClick_Forward_End)
		.OnClickedBackwardPlay(this, &FMotionFieldEditor::OnClick_Backward)
		.OnClickedBackwardStep(this, &FMotionFieldEditor::OnClick_Backward_Step)
		.OnClickedBackwardEnd(this, &FMotionFieldEditor::OnClick_Backward_End)
		.OnClickedToggleLoop(this, &FMotionFieldEditor::OnClick_ToggleLoop)
		.OnGetLooping(this, &FMotionFieldEditor::IsLooping)
		.OnGetPlaybackMode(this, &FMotionFieldEditor::GetPlaybackMode)
		.ViewInputMin(this, &FMotionFieldEditor::GetViewRangeMin)
		.ViewInputMax(this, &FMotionFieldEditor::GetViewRangeMax)
		.OnSetInputViewRange(this, &FMotionFieldEditor::SetViewRange)
		.bAllowZoom(true)
		.IsRealtimeStreamingMode(false)
		;// .Visibility(EVisibility::Collapsed);

	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
		[
			ViewportPtr.ToSharedRef()
		]
	
	+ SVerticalBox::Slot()
		.Padding(0, 8, 0, 0)
		.AutoHeight()
		.HAlign(HAlign_Fill)
		[
			SAssignNew(TagTimelineBoxPtr, SVerticalBox)//Timeline//.ToSharedRef()
		]
		
	+ SVerticalBox::Slot()
		.Padding(0, 8, 0, 0)
		.AutoHeight()
		[
			ScrubControl
		]
		];
	/*
	for (int32 i = 0; i < 5; i++)
	{
		TagTimelineBoxPtr->AddSlot()
			.FillWidth(0.5f)
			[
				Timeline
			];
	}
	*/
}

TSharedRef<SDockTab> FMotionFieldEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	TSharedPtr<FMotionFieldEditor> MotionFieldEditorPtr = SharedThis(this);
	// Spawn the tab

	ContextList = SNew(SContextList, MotionFieldEditorPtr);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
	      	SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(0.5f)
		[
			SNew(SBorder)
			[
				ContextList.ToSharedRef()
			]
		]
		];
		
}

void FMotionFieldEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_MotionFieldEditor", "MotionField Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FMotionFieldEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FMotionFieldEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	
	InTabManager->RegisterTabSpawner(FMotionFieldEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FMotionFieldEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.ContentBrowser"));
}

void FMotionFieldEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FMotionFieldEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FMotionFieldEditorTabs::DetailsID);
}

void FMotionFieldEditor::InitMotionFieldEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UMotionField* InitMotionField)
{
	FAssetEditorManager::Get().CloseOtherEditors(InitMotionField, this);
	MotionFieldBeingEdited = InitMotionField;

	CurrentExtractionContextIndex = INDEX_NONE;
	
	FMotionFieldEditorCommands::Register();

	BindCommands();
	TSharedPtr<FMotionFieldEditor> MotionFieldEditorPtr = SharedThis(this);

	ViewportPtr = SNew(SMotionFieldEditorViewport, MotionFieldEditorPtr)
		.MotionFieldBeingEdited(this, &FMotionFieldEditor::GetMotionFieldBeingEdited);


	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_MotionFieldEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(1.f)
					->SetHideTabWell(true)
					->AddTab(FMotionFieldEditorTabs::ViewportID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.75f)
					->SetHideTabWell(true)
					->AddTab(FMotionFieldEditorTabs::DetailsID, ETabState::OpenedTab)
				)
			)
		);

	// Initialize the asset editor and spawn nothing (dummy layout)
	InitAssetEditor(Mode, InitToolkitHost, MotionFieldEditorAppName, StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, InitMotionField);

	// Extend things
	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();


	RebuildTagTimelines();
}

UDebugSkelMeshComponent* FMotionFieldEditor::GetPreviewComponentExtraction() const
{
	UDebugSkelMeshComponent* PreviewComponent = ViewportPtr->GetPreviewComponent();
	check(PreviewComponent);
	return PreviewComponent;
}

UAnimSequence * FMotionFieldEditor::GetSourceAnimation() const
{
	if (GetMotionFieldBeingEdited()->IsValidSrcAnimIndex(CurrentExtractionContextIndex))
	{
		UAnimSequence* SourceAnimation = GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(CurrentExtractionContextIndex);
		if (SourceAnimation)
		{
			check(SourceAnimation);
			return SourceAnimation;
		}
	}
	return NULL;
}

bool FMotionFieldEditor::SetPreviewComponentSkeletalMesh(USkeletalMesh * SkeletalMesh) const
{
	if (GetPreviewComponentExtraction())
	{
		if (SkeletalMesh)
		{
			if (GetPreviewComponentExtraction()->SkeletalMesh)
			{

				if (GetPreviewComponentExtraction()->SkeletalMesh->Skeleton != SkeletalMesh->Skeleton)
				{
					SetExtractionAnimSequence(NULL);

					GetPreviewComponentExtraction()->SetSkeletalMesh(SkeletalMesh, true);
					//GetPreviewComponent()->RefreshBoneTransforms();
					ViewportPtr->OnFocusViewportToSelection();
					return false;
				}
				else
				{
					GetPreviewComponentExtraction()->SetSkeletalMesh(SkeletalMesh, false);
					ViewportPtr->OnFocusViewportToSelection();
					return true;
				}
				//GetPreviewComponent()->RefreshBoneTransforms();
				/*
				TArray <FName> BoneNames = GetPreviewComponent()->GetAllSocketNames();

				for (int i = 0; i < BoneNames.Num(); i++)
				{
					GetPreviewComponent()->ResetBoneTransformByName(BoneNames[i]);
				}
				*/
			}
			
			SetExtractionAnimSequence(NULL);

			GetPreviewComponentExtraction()->SetSkeletalMesh(SkeletalMesh, true);
				//GetPreviewComponent()->RefreshBoneTransforms();
			ViewportPtr->OnFocusViewportToSelection();
			return false;
			
		}
		else
		{
			SetExtractionAnimSequence(NULL);

			GetPreviewComponentExtraction()->SetSkeletalMesh(NULL, true);

			return false;
		}
	}
	return false;
}

bool FMotionFieldEditor::SetExtractionAnimSequence(UAnimSequence * AnimSequence) const
{
	if (!GetPreviewComponentExtraction()->SkeletalMesh)
	{
		return false;
	}
	if (AnimSequence)
	{
		if (AnimSequence->GetSkeleton() == GetPreviewComponentExtraction()->SkeletalMesh->Skeleton)
		{
			GetPreviewComponentExtraction()->EnablePreview(true, AnimSequence);
			return true;
		}
		else
		{
			GetPreviewComponentExtraction()->EnablePreview(true, NULL);
			
			return false;
		}
	}
	else
	{
		GetPreviewComponentExtraction()->EnablePreview(true, NULL);
		
		return true;
	}
	return false;
}

void FMotionFieldEditor::AddNewExtractionContext(TArray <UAnimSequence*> FromSequences)
{
	for (int i = 0; i < FromSequences.Num(); i++)
	{
		if (FromSequences[i])
		{
			GetMotionFieldBeingEdited()->AddSrcAnim(FromSequences[i]);
		}
	}
	ContextList.Get()->Rebuild();
	RebuildTagTimelines();
}

void FMotionFieldEditor::DeleteExtractionContext(const int AtIndex)
{
	if (AtIndex == CurrentExtractionContextIndex)
	{
		CurrentExtractionContextIndex = INDEX_NONE;
		SetExtractionAnimSequence(NULL);
	}

	GetMotionFieldBeingEdited()->DeleteSrcAnim(AtIndex);
	ContextList.Get()->Rebuild();

	RebuildTagTimelines();
}

void FMotionFieldEditor::SetCurrentExtractionContext(const int AtIndex)
{

	if (IsValidContext(AtIndex) && (AtIndex != CurrentExtractionContextIndex))
	{
		CurrentExtractionContextIndex = AtIndex;
		SetExtractionAnimSequence(GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(AtIndex));
	}
	else
	{
		
		CurrentExtractionContextIndex = INDEX_NONE;
		SetExtractionAnimSequence(NULL);
	}

	RebuildTagTimelines();
	//MotionKeyList.Get()->Rebuild();

}


FText FMotionFieldEditor::GetContextAnimationName(const int AtContextIndex)
{
	if (GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(AtContextIndex))
	{
		return FText::AsCultureInvariant(GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(AtContextIndex)->GetName());
	}
	return LOCTEXT("NullAnimation", "Null Animation");
}

bool FMotionFieldEditor::IsValidContext(const int AtContextIndex)
{
	if (GetMotionFieldBeingEdited()->IsValidSrcAnimIndex(AtContextIndex))
	{
		if (GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(AtContextIndex))
		{
			return true;
		}
	}
	
		return false;
	
}

bool FMotionFieldEditor::IsExtractedFrame(const int FrameIndex)
{
	/*if (GetMotionFieldBeingEdited()->IsValidSrcAnimIndex(CurrentExtractionContextIndex))
	{
		if (MotionKeysInCurrentContext.Num() > 0)
		{
			for (int i = 0; i < MotionKeysInCurrentContext.Num(); i++)
			{
				if (MotionFieldBeingEdited->MotionKeys[MotionKeysInCurrentContext[i]].MotionKeyHandle.SrcFrameIndex == FrameIndex)
				{
					//FString JO = TEXT("TRUE MY MAN WHAT");
					//GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Red, JO);
					return true;

				}
			}
		}
	}*/
	return false;
}


FString FMotionFieldEditor::GetMotionFieldSkeletonName()
{
	return GetMotionFieldBeingEdited()->GetMotionFieldSkeleton()->GetName();
}

bool FMotionFieldEditor::AnimationAlreadyAddedToContextArray(const FName SequenceName)
{
	const int Num = GetMotionFieldBeingEdited()->GetSrcAnimNum();
	for (int i = 0; i < Num; i++)
	{
		if (GetMotionFieldBeingEdited()->GetSrcAnimAtIndex(i)->GetFName() == SequenceName)
		{
			return true;
		}
	}
	
	return false;
}

bool FMotionFieldEditor::GetCurrentTrajectoryData(FTrajectoryData & PresentTrajectory, FTrajectoryData & FutureTrajectory, FTransform& PastTM)
{
	if (UAnimSequence* CurrentSequence = GetSourceAnimation())
	{
		if (CurrentSequence)
		{
			float KeyTime = GetPlaybackPosition();
			float MaxTime = CurrentSequence->GetPlayLength() - 1.0f;
			if ((KeyTime >= 1.0f) && (KeyTime <= MaxTime))
			{
				GetSourceAnimation()->GetBoneTransform(PastTM, 0, KeyTime - 1.0f, false);
				FMotionKeyUtils::ExtractAnimTrajectory(PresentTrajectory, CurrentSequence, KeyTime - 1.0f);
				FMotionKeyUtils::ExtractAnimTrajectory(FutureTrajectory, CurrentSequence, KeyTime);
				return true;
			}
		}
	}
	return false;
}

void FMotionFieldEditor::AddRangeToTagHelper(const float RangeTime, const uint8 AtTagIdx)
{

	MotionFieldBeingEdited->Modify();
	MotionFieldBeingEdited->AddRangeToTagHelper(RangeTime, AtTagIdx, CurrentExtractionContextIndex);
	RebuildTagTimelines();
}

void FMotionFieldEditor::RemoveRangeFromTagHelper(const float RangeTime, const uint8 AtTagIdx)
{
	MotionFieldBeingEdited->Modify();
	MotionFieldBeingEdited->RemoveRangeFromTagHelper(RangeTime, AtTagIdx, CurrentExtractionContextIndex);
	RebuildTagTimelines();
}

bool FMotionFieldEditor::IsTimeTagged(const float Time, const uint8 AtTagIdx)
{
	return MotionFieldBeingEdited->IsTimeTagged(Time, AtTagIdx, CurrentExtractionContextIndex);
}

void FMotionFieldEditor::BindCommands()
{
	const FMotionFieldEditorCommands& Commands = FMotionFieldEditorCommands::Get();

	const TSharedRef<FUICommandList>& UICommandList = GetToolkitCommands();
	
	UICommandList->MapAction(Commands.ProcessAll,
		FExecuteAction::CreateSP(this, &FMotionFieldEditor::ProcessAllClicked));
	UICommandList->MapAction(Commands.ClearAll,
		FExecuteAction::CreateSP(this, &FMotionFieldEditor::ClearAllClicked));
	UICommandList->MapAction(Commands.SetProperties,
		FExecuteAction::CreateSP(this, &FMotionFieldEditor::SetPropertiesClicked));
}

FName FMotionFieldEditor::GetToolkitFName() const
{
	return FName("MotionFieldEditor");
}

FText FMotionFieldEditor::GetBaseToolkitName() const
{
	return LOCTEXT("MotionFieldEditorAppLabel", "MotionField Editor");
}

FText FMotionFieldEditor::GetToolkitName() const
{
	const bool bDirtyState = MotionFieldBeingEdited->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("MotionFieldName"), FText::FromString(MotionFieldBeingEdited->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("MotionFieldEditorToolkitName", "{MotionFieldName}{DirtyState}"), Args);
}

FText FMotionFieldEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(MotionFieldBeingEdited);
}

FString FMotionFieldEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("MotionFieldEditor");
}

FString FMotionFieldEditor::GetDocumentationLink() const
{
	return TEXT("Engine/Paper2D/MotionFieldEditor");
}

FLinearColor FMotionFieldEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

void FMotionFieldEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(MotionFieldBeingEdited);
}

void FMotionFieldEditor::ExtendMenu()
{
}

void FMotionFieldEditor::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("Command");
			{
				ToolbarBuilder.AddToolBarButton(FMotionFieldEditorCommands::Get().ProcessAll);
				ToolbarBuilder.AddToolBarButton(FMotionFieldEditorCommands::Get().ClearAll);
				ToolbarBuilder.AddToolBarButton(FMotionFieldEditorCommands::Get().SetProperties);
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		/*ViewportPtr->GetCommandList()*/ GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);

	FMotionMatchingEditorModule* MotionMatchingEditorModule = &FModuleManager::LoadModuleChecked<FMotionMatchingEditorModule>("MotionMatchingEditor");
	AddToolbarExtender(MotionMatchingEditorModule->GetMotionFieldEditorToolBarExtensibilityManager()->GetAllExtenders());
}




void FMotionFieldEditor::ProcessAllClicked()
{
	if (MotionFieldBeingEdited)
	{
		MotionFieldBeingEdited->Modify();

		MotionFieldBeingEdited->RebakeAllAnim();

		MotionFieldBeingEdited->MarkPackageDirty();
		/*
		if (MotionFieldBeingEdited->IsValidSrcAnimIndex(CurrentExtractionContextIndex))
		{
			

			UAnimSequence* Anim = GetSourceAnimation();
			
			if (Anim)
			{
				PendingFrames.Empty();

				float MaxTime = Anim->SequenceLength;
				float CurrentTime = 0.f;
				while (CurrentTime < MaxTime)
				{
					CurrentTime += 0.1f;

					PendingFrames.Add(Anim->GetFrameAtTime(CurrentTime));

				}
			}

			//ResetMotionKeysInCurrentContext();
		}
		*/
	}
}

void FMotionFieldEditor::ClearAllClicked()
{
	MotionFieldBeingEdited->ClearAllMotionKeys();
}

void FMotionFieldEditor::SetPropertiesClicked()
{
	SSetPropertiesDialog::ShowWindow(GetMotionFieldBeingEdited(), SharedThis(this));
	//SAddNewContextDialog::ShowWindow(MotionFieldEditorPtr.Pin());
}


///////////////////////////////////////////////////////////////OW BABYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
///////////////////////////////////////////////////////////////////


FReply FMotionFieldEditor::OnClick_Forward()
{
	
	UDebugSkelMeshComponent* PreviewComponent = GetPreviewComponentExtraction();

	const bool bIsReverse = PreviewComponent->PreviewInstance->IsReverse();
	const bool bIsPlaying = PreviewComponent->PreviewInstance->IsPlaying();

	if (bIsReverse && bIsPlaying)
	{
		// Play forwards instead of backwards
		PreviewComponent->PreviewInstance->SetReverse(false);
	}
	else if (bIsPlaying)
	{
		// Was already playing forwards, so pause
		PreviewComponent->PreviewInstance->StopAnim();
	}
	else
	{
		// Was paused, start playing
		PreviewComponent->PreviewInstance->SetPlaying(true);
	}
	
	return FReply::Handled();
}

FReply FMotionFieldEditor::OnClick_Forward_Step()
{
	
	GetPreviewComponentExtraction()->PreviewInstance->StopAnim();
	
	SetCurrentFrame(GetCurrentFrame() + 1);
	
	return FReply::Handled();
}

FReply FMotionFieldEditor::OnClick_Forward_End()
{
	
	UDebugSkelMeshComponent* PreviewComponent = GetPreviewComponentExtraction();
	PreviewComponent->PreviewInstance->StopAnim();
	PreviewComponent->PreviewInstance->SetPosition(GetTotalSequenceLength(), false);
	
	return FReply::Handled();
}

FReply FMotionFieldEditor::OnClick_Backward()
{
	
	UDebugSkelMeshComponent* PreviewComponent = GetPreviewComponentExtraction();

	const bool bIsReverse = PreviewComponent->PreviewInstance->IsReverse();
	const bool bIsPlaying = PreviewComponent->PreviewInstance->IsPlaying();

	if (bIsReverse && bIsPlaying)
	{
		// Was already playing backwards, so pause
		PreviewComponent->PreviewInstance->StopAnim();
	}
	else if (bIsPlaying)
	{
		// Play backwards instead of forwards
		PreviewComponent->PreviewInstance->SetReverse(true);
	}
	else
	{
		// Was paused, start reversing
		PreviewComponent->PreviewInstance->SetReverse(true);
		PreviewComponent->PreviewInstance->SetPlaying(true);
	}
	
	return FReply::Handled();
}

FReply FMotionFieldEditor::OnClick_Backward_Step()
{
	
	GetPreviewComponentExtraction()->PreviewInstance->StopAnim();
	SetCurrentFrame(GetCurrentFrame() - 1);
	
	return FReply::Handled();
}

FReply FMotionFieldEditor::OnClick_Backward_End()
{

	UDebugSkelMeshComponent* PreviewComponent = GetPreviewComponentExtraction();
	PreviewComponent->PreviewInstance->StopAnim();
	PreviewComponent->PreviewInstance->SetPosition(0.0f, false);
	
	return FReply::Handled();
}

FReply FMotionFieldEditor::OnClick_ToggleLoop()
{

	UDebugSkelMeshComponent* PreviewComponent = GetPreviewComponentExtraction();
	PreviewComponent->PreviewInstance->SetLooping(!PreviewComponent->PreviewInstance->IsLooping());
	
	
	return FReply::Handled();
}

EPlaybackMode::Type FMotionFieldEditor::GetPlaybackMode() const
{
	
	UDebugSkelMeshComponent* PreviewComponent = GetPreviewComponentExtraction();
	if (PreviewComponent->PreviewInstance->IsPlaying())
	{
		return PreviewComponent->PreviewInstance->IsReverse() ? EPlaybackMode::PlayingReverse : EPlaybackMode::PlayingForward;
	}
	else
	{
		return EPlaybackMode::Stopped;
	}
	
	return EPlaybackMode::Stopped;
}

uint32 FMotionFieldEditor::GetTotalFrameCount() const
{
	if (GetSourceAnimation())
	{
		return GetSourceAnimation()->GetNumberOfFrames();
	}
	return 0;
}

uint32 FMotionFieldEditor::GetTotalFrameCountPlusOne() const
{
	//return MotionFieldBeingEdited->MotionKeys.Num() + 1;
	if (GetSourceAnimation())
	{
		return GetSourceAnimation()->GetNumberOfFrames() + 1;
	}
	return 1;
}

float FMotionFieldEditor::GetTotalSequenceLength() const
{
	if(GetSourceAnimation())
	{
		return GetSourceAnimation()->GetPlayLength();
	}
	return 0.f;
}

float FMotionFieldEditor::GetPlaybackPosition() const
{
	return GetPreviewComponentExtraction()->PreviewInstance->GetCurrentTime();//GetPreviewComponent()->GetPlaybackPosition();
}

void FMotionFieldEditor::SetPlaybackPosition(float NewTime)
{
	NewTime = FMath::Clamp<float>(NewTime, 0.0f, GetTotalSequenceLength());
	
	GetPreviewComponentExtraction()->PreviewInstance->SetPosition(NewTime, /*bFireEvents=*/ false);
}

bool FMotionFieldEditor::IsLooping() const
{
	return GetPreviewComponentExtraction()->PreviewInstance->IsLooping();//GetPreviewComponent()->IsLooping();
}

float FMotionFieldEditor::GetViewRangeMin() const
{
	return ViewInputMin;
}

float FMotionFieldEditor::GetViewRangeMax() const
{
	// See if the MotionField changed length, and if so reframe the scrub bar to include the full length
	//@TODO: This is a pretty odd place to put it, but there's no callback for a modified timeline at the moment, so...
	const float SequenceLength = GetTotalSequenceLength();
	if (SequenceLength != LastObservedSequenceLength)
	{
		LastObservedSequenceLength = SequenceLength;
		ViewInputMin = 0.0f;
		ViewInputMax = SequenceLength;
	}

	return ViewInputMax;
}

void FMotionFieldEditor::SetViewRange(float NewMin, float NewMax)
{
	ViewInputMin = FMath::Max<float>(NewMin, 0.0f);
	ViewInputMax = FMath::Min<float>(NewMax, GetTotalSequenceLength());
}


void FMotionFieldEditor::RebuildTagTimelines()
{
	TagTimelineBoxPtr->ClearChildren();

	if (CurrentExtractionContextIndex != INDEX_NONE)
	{
		int32 NumOfTags = MotionFieldBeingEdited->GetNumOfTags();
		if (NumOfTags > 0)
		{

			TSharedPtr<FMotionFieldEditor> MotionFieldEditorPtr = SharedThis(this);

			for (int32 i = 0; i < NumOfTags; i++)
			{

				TSharedRef<SMotionFieldTimeline> Timeline = SNew(SMotionFieldTimeline, GetToolkitCommands(), MotionFieldEditorPtr, i)
					.MotionFieldBeingEdited(this, &FMotionFieldEditor::GetMotionFieldBeingEdited)
					.PlayTime(this, &FMotionFieldEditor::GetPlaybackPosition)
					.SourceAnimationAttr(this, &FMotionFieldEditor::GetSourceAnimation);
				// .Visibility(EVisibility::Collapsed);
				//.OnSelectionChanged(this, &FMotionFieldEditor::SetSelection)


				TagTimelineBoxPtr->AddSlot()
					.FillHeight(0.5f)
					.HAlign(HAlign_Fill)
					[
						Timeline
					];
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
