// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Engine/Texture2D.h"
#include "IDetailsView.h"


#include "Layout/Visibility.h"
#include "Widgets/SWidget.h"

#include "IDetailCustomization.h"



struct FAssetData;
class IDetailLayoutBuilder;
class IPropertyHandle;
class SComboButton;
class UAnimSequence;
class FAssetThumbnailPool;

/*Dialog used to choose A new Sequence to extracct Motion Keys From*/
class SAddNewContextDialog : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SAddNewContextDialog) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, TSharedPtr<class FMotionFieldEditor> InMotionFieldEditorPtr);

	~SAddNewContextDialog();

	static bool ShowWindow(TSharedPtr<FMotionFieldEditor> InMotionFieldEditorPtr);

	//void OnAnimSelected(TArray< FAssetData > Data);
	bool FilterAnim(const FAssetData& AssetData);


private:

	//TArray <UAnimSequence*> StoredSequences;
	//TArray <int> SelectedIndexes;

	/** Used to get the currently selected assets */
	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate;

	FString SkeletonName;

	FReply AddClicked();

	FReply CancelClicked();

	void CloseContainingWindow();

	TSharedPtr<class IDetailsView> MainPropertyView;

	TSharedPtr<class FMotionFieldEditor> MotionFieldEditorPtr;

	TSharedPtr <FAssetThumbnailPool> ThumbnailPool;
};
