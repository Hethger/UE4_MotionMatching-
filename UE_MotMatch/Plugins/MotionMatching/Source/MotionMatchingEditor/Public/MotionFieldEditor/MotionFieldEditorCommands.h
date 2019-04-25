// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "MotionMatchingStyle.h"

class FMotionFieldEditorCommands : public TCommands<FMotionFieldEditorCommands>
{
public:
	FMotionFieldEditorCommands()
		: TCommands<FMotionFieldEditorCommands>(
			TEXT("MotionFieldEditor"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "MotionFieldEditor", "MotionField Editor"), // Localized context name for displaying
			NAME_None, // Parent
			FMotionMatchingStyle::GetStyleSetName() // Icon Style Set
			)
	{
	}

	// TCommand<> interface
	virtual void RegisterCommands() override;
	// End of TCommand<> interface

public:
	TSharedPtr<FUICommandInfo> ProcessAll;

	TSharedPtr<FUICommandInfo> ClearAll;

	TSharedPtr<FUICommandInfo> SetProperties;


	TSharedPtr<FUICommandInfo> SetShowGrid;
	TSharedPtr<FUICommandInfo> SetShowBounds;
	TSharedPtr<FUICommandInfo> SetShowCollision;

	// View Menu Commands
	TSharedPtr<FUICommandInfo> SetShowPivot;
	TSharedPtr<FUICommandInfo> SetShowSockets;

	// Asset commands
	TSharedPtr<FUICommandInfo> PickNewSpriteFrame;
	TSharedPtr<FUICommandInfo> EditSpriteFrame;
	TSharedPtr<FUICommandInfo> ShowInContentBrowser;
};
