// Fill out your copyright notice in the Description page of Project Settings.

#include "SContextList.h"

#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "SScrubControlPanel.h"
#include "EditorStyleSet.h"
#include "SBorder.h"
#include "SScrollBox.h"
#include "SBox.h"
#include "SButton.h"
#include "STextBlock.h"
//#include "PaperSprite.h"
#include "AddNewContextDialog.h"


#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "ContextList"

//////////////////////////////////////////////////////////////////////////
// SSpriteList


void SSingleContextWidget::Construct(const FArguments& InArgs, int32 InFrameIndex, TWeakPtr<FMotionFieldEditor> InMotionFieldEditor)
{
	ContextIndex = InFrameIndex;
	MotionFieldEditorPtr = InMotionFieldEditor;

	// Color each region based on whether a sprite has been set or not for it
	const auto BorderColorDelegate = [](TAttribute<UMotionField*> ThisMotionFieldPtr, int32 TestIndex) -> FSlateColor
	{
		UMotionField* MotionFieldPtr = ThisMotionFieldPtr.Get();
		const bool bFrameValid = (MotionFieldPtr != nullptr) && (MotionFieldPtr->MotionKeys.Num() > 0);
		return bFrameValid ? FLinearColor::White : FLinearColor::Black;
	};

	TSharedRef<SWidget> ClearButton = PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &SSingleContextWidget::RemoveContextButton_OnClick),
		 LOCTEXT("RemoveContextToolTip", "Remove Context."), true);
	
	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("DialogueWaveDetails.HeaderBorder"))
		[
			//SNew(SDialogueContextHeaderWidget, ContextPropertyHandle.ToSharedRef(), DetailLayoutBuilder->GetThumbnailPool().ToSharedRef())
			SNew(SButton)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.ButtonStyle(FEditorStyle::Get(), "FlatButton")
		.ForegroundColor(FLinearColor::White)
		.OnClicked(this, &SSingleContextWidget::ContextClicked)
		[
			// Voice Description
			SNew(STextBlock)
			.Text(this, &SSingleContextWidget::GetContextAssetName)
		]
		]
		]
	    + SHorizontalBox::Slot()
		.Padding(2.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			ClearButton
		]
	];
}

FReply SSingleContextWidget::ContextClicked()
{
	MotionFieldEditorPtr.Pin().Get()->SetCurrentExtractionContext(ContextIndex);

	return FReply::Handled();
}

void SSingleContextWidget::RemoveContextButton_OnClick()
{
	MotionFieldEditorPtr.Pin().Get()->DeleteExtractionContext(ContextIndex);
}

FText SSingleContextWidget::GetContextAssetName() const
{
	return MotionFieldEditorPtr.Pin().Get()->GetContextAnimationName(ContextIndex);
}





void SContextList::Construct(const FArguments& InArgs, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditor)
{
	MotionFieldEditorPtr = InMotionFieldEditor;

	MainBox = SNew(SVerticalBox);

	this->ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.Padding(2.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
			[
			SNew(SButton)
			.Text(LOCTEXT("AddMotionExtractionContext", "Add Motion Extraction Context"))
		    .HAlign(HAlign_Center)
		.ToolTipText(LOCTEXT("AddContextToolTip", "Adds a new Extraction Context to the Motion Field"))
		.OnClicked(FOnClicked::CreateSP(this, &SContextList::AddNewContextClicked))
			]
		]
	+ SVerticalBox::Slot()
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
		.ScrollBarAlwaysVisible(true)
		//.OnUserScrolled(this, &SMotionFieldTimeline::AnimationScrollBar_OnUserScrolled)
		+ SScrollBox::Slot()
		[
			SNew(SOverlay)
			// MotionField header and track
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 2)
		[
			SNew(SBox)
			[
				MainBox.ToSharedRef()
			]
		]
		]
		]
		]
		]
		];

	Rebuild();
}

void SContextList::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

}


FReply SContextList::AddNewContextClicked()
{
	SAddNewContextDialog::ShowWindow(MotionFieldEditorPtr.Pin());
	return FReply::Handled();
}

void SContextList::Rebuild()
{
	MainBox->ClearChildren();
	
	const int Num = MotionFieldEditorPtr.Pin().Get()->GetMotionFieldBeingEdited()->GetSrcAnimNum();
	
	if (Num > 0)
	{
		for (int32 KeyFrameIdx = 0; KeyFrameIdx < Num; ++KeyFrameIdx)
		{

			MainBox->AddSlot()
				.FillHeight(0.5f)
				[
					SNew(SSingleContextWidget, KeyFrameIdx, MotionFieldEditorPtr)
				];
		}
	}

}


#undef LOCTEXT_NAMESPACE