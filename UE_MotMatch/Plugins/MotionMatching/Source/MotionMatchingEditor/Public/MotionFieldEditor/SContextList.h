// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "MotionFieldEditor.h"
#include "ContentBrowserDelegates.h"



//////////////////////////////////////////////////////////////////////////
// SSpriteList

class SBorder;
class SScrollBox;
class SBox;
class SButton;
class STextBlock;



class SSingleContextWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSingleContextWidget) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, int32 InFrameIndex, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditor);

	FReply ContextClicked();
	void RemoveContextButton_OnClick();
protected:

	FText GetContextAssetName() const;
	//FText GetKeyframeText() const;
	//FText GetKeyframeTooltip() const;

	//void ShowInContentBrowser();

protected:
	int32 ContextIndex;
	//TSharedPtr<FUICommandList> CommandList;
private:
	TWeakPtr<class FMotionFieldEditor> MotionFieldEditorPtr;;
};



class SContextList : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SContextList) {}
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs, TWeakPtr<class FMotionFieldEditor> InMotionFieldEditor);

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End of SWidget interface
	
	
	FReply AddNewContextClicked();

	void Rebuild();
	/*
	void SelectAsset(UObject* Asset);
	
protected:
	void RebuildWidget(UTexture2D* NewTextureFilter);

	void OnSpriteSelected(const struct FAssetData& AssetData);
	void OnSpriteDoubleClicked(const struct FAssetData& AssetData);
	bool CanShowColumnForAssetRegistryTag(FName AssetType, FName TagName) const;
	*/
protected:

	// Pointer back to owning sprite editor instance (the keeper of state)
	TWeakPtr<class FMotionFieldEditor> MotionFieldEditorPtr;

	TSharedPtr<SVerticalBox> MainBox;

	// Set of tags to prevent creating details view columns for (infrequently used)
	TSet<FName> AssetRegistryTagsToIgnore;

	// Delegate to sync the asset picker to selected assets
	FSyncToAssetsDelegate SyncToAssetsDelegate;
};
