
#include "AddNewContextDialog.h"
#include "MotionMatchingEditor.h"


#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/SViewport.h"
#include "Misc/FeedbackContext.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Docking/TabManager.h"
#include "EditorStyleSet.h"
#include "CanvasItem.h"

#include "PropertyEditorModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "CanvasTypes.h"

#include "Factories/DataAssetFactory.h"

#include "MotionKey.h"

#include "SScrollBox.h"

#include "Animation/AnimSequence.h"
//////////////////////////////////////////

#include "AssetRegistryModule.h"

#include "AssetThumbnail.h"

#include "STextBlock.h"
#include "SBox.h"

#define LOCTEXT_NAMESPACE "MotionMatchingEditor"

namespace MyAssetShortcutConstants
{
	const int32 ThumbnailSize = 128;
	const int32 ThumbnailSizeSmall = 16;
}



void SAddNewContextDialog::Construct(const FArguments & InArgs, TSharedPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
{
	MotionFieldEditorPtr = InMotionFieldEditorPtr;
	SkeletonName = MotionFieldEditorPtr.Get()->GetMotionFieldSkeletonName();

	/*

	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	TArray<FAssetData> AssetData;
	const UClass* Class = UAnimSequence::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass("AnimSequence", AssetData);
	*/

	TSharedPtr <SVerticalBox> MainBox = SNew(SVerticalBox);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(UAnimSequence::StaticClass()->GetFName());
	AssetPickerConfig.SelectionMode = ESelectionMode::Multi;
	//AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SAddNewContextDialog::OnAnimSelected);
	AssetPickerConfig.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);
	//AssetPickerConfig.OnAssetsActivated = FOnAssetsActivated::CreateSP(this, &SAddNewContextDialog::OnAnimSelected);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SAddNewContextDialog::FilterAnim);
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
	AssetPickerConfig.ThumbnailScale = 128.f;
	//AssetPickerConfig.InitialAssetSelection = SelectionArray[0];

	MainBox->AddSlot()
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];

	ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("DetailsView.CategoryTop"))
		.Padding(FMargin(1.0f, 1.0f, 1.0f, 0.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		[
		
			SNew(SScrollBox)
			+ SScrollBox::Slot()
		     .Padding(1.0f)
		   .HAlign(HAlign_Fill)
		   .VAlign(VAlign_Fill)
		[
		 MainBox.ToSharedRef()//MainPropertyView.ToSharedRef()
		]
		
		]
	    + SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SUniformGridPanel)
			.SlotPadding(1)
		+ SUniformGridPanel::Slot(0, 0)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
		.ForegroundColor(FLinearColor::White)
		.Text(LOCTEXT("AddButton", "Add"))
		.OnClicked(this, &SAddNewContextDialog::AddClicked)
		]
	+ SUniformGridPanel::Slot(1, 0)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton")
		.ForegroundColor(FLinearColor::White)
		.Text(LOCTEXT("CancelButton", "Cancel"))
		.OnClicked(this, &SAddNewContextDialog::CancelClicked)
		]
		]
		]
		]
		];
		
}

SAddNewContextDialog::~SAddNewContextDialog()
{
}

bool SAddNewContextDialog::ShowWindow(TSharedPtr<FMotionFieldEditor> InMotionFieldEditorPtr)
{

	const FText TitleText = NSLOCTEXT("MotionMatchingEditor", "MotionMatchingEditor_AddNewContext", "Add New Context");
	// Create the window to pick the class
	TSharedRef<SWindow> AddNewContextWindow = SNew(SWindow)
		.Title(TitleText)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(550.f, 1000.f))
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false);

	TSharedRef<SAddNewContextDialog> AddNewContextDialog = SNew(SAddNewContextDialog, InMotionFieldEditorPtr);

	AddNewContextWindow->SetContent(AddNewContextDialog);
	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (RootWindow.IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(AddNewContextWindow, RootWindow.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(AddNewContextWindow);
	}

	return false;
}

bool SAddNewContextDialog::FilterAnim(const FAssetData & AssetData)
{
	if (!AssetData.IsAssetLoaded())
	{
		AssetData.GetPackage()->FullyLoad();
	}

	bool AlreadyAdded = MotionFieldEditorPtr.Get()->AnimationAlreadyAddedToContextArray(AssetData.AssetName);

	return AlreadyAdded || (SkeletonName != Cast<UAnimSequence>(AssetData.GetAsset())->GetSkeleton()->GetName());
}

FReply SAddNewContextDialog::AddClicked()
{
	
	TArray<FAssetData> SelectionArray = GetCurrentSelectionDelegate.Execute();
	
	if (SelectionArray.Num() > 0)
	{
		TArray <UAnimSequence*> StoredSequences;

		for (int i = 0; i < SelectionArray.Num(); i++)
		{
			if (SelectionArray[i].IsAssetLoaded())
			{
				UAnimSequence* NewSequence = Cast <UAnimSequence>(SelectionArray[i].GetAsset());
				if (NewSequence)
				{
					StoredSequences.Add(NewSequence);
				}
			}
		}

		if (StoredSequences.Num() > 0)
		{
			MotionFieldEditorPtr.Get()->AddNewExtractionContext(StoredSequences);
		}

		CloseContainingWindow();
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoAnimationsSelected", "No Animations Selected."));
	}

	return FReply::Handled();
}

FReply SAddNewContextDialog::CancelClicked()
{
	CloseContainingWindow();
	return FReply::Handled();
}

void SAddNewContextDialog::CloseContainingWindow()
{
	FWidgetPath WidgetPath;
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared(), WidgetPath);
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

#undef LOCTEXT_NAMESPACE
