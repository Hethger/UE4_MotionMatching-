// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionFieldEditor/SMotionFieldEditorViewportToolbar.h"

#include "SEditorViewport.h"
#include "MotionFieldEditor/MotionFieldEditorCommands.h"

#define LOCTEXT_NAMESPACE "SMotionFieldEditorViewportToolbar"

///////////////////////////////////////////////////////////
// SMotionFieldEditorViewportToolbar

void SMotionFieldEditorViewportToolbar::Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider)
{
	SCommonEditorViewportToolbarBase::Construct(SCommonEditorViewportToolbarBase::FArguments(), InInfoProvider);
}

TSharedRef<SWidget> SMotionFieldEditorViewportToolbar::GenerateShowMenu() const
{
	GetInfoProvider().OnFloatingButtonClicked();

	TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());
	{
		ShowMenuBuilder.AddMenuEntry(FMotionFieldEditorCommands::Get().SetShowSockets);
		ShowMenuBuilder.AddMenuEntry(FMotionFieldEditorCommands::Get().SetShowPivot);

		ShowMenuBuilder.AddMenuSeparator();

		ShowMenuBuilder.AddMenuEntry(FMotionFieldEditorCommands::Get().SetShowGrid);
		ShowMenuBuilder.AddMenuEntry(FMotionFieldEditorCommands::Get().SetShowBounds);

		ShowMenuBuilder.AddMenuSeparator();

		ShowMenuBuilder.AddMenuEntry(FMotionFieldEditorCommands::Get().SetShowCollision);
	}

	return ShowMenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
