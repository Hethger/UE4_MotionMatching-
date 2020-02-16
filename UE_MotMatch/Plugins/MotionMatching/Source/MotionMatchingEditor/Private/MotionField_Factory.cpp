// Fill out your copyright notice in the Description page of Project Settings.

#include "MotionField_Factory.h"

#include "InputCoreTypes.h"
#include "UObject/Interface.h"
#include "Layout/Visibility.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimInstance.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
//#include "Animation/MotionField.h"
#include "Engine/BlueprintGeneratedClass.h"
//#include "Animation/MotionFieldGeneratedClass.h"
#include "AssetData.h"
#include "Editor.h"
#include "Kismet2/KismetEditorUtilities.h"


#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"

#include "IAssetTools.h"
#include "AssetToolsModule.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Colors/SColorBlock.h"
//#include "STreeView.h"


//Dialog to configure creation properties
#define LOCTEXT_NAMESPACE "MotionFieldFactory"

/*SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton")
				   .ForegroundColor(this, &SSingleBoneItem::GetIt)
			.OnClicked(this, &SSingleBoneItem::BoneItemClicked)
				   [
					   SNew(SBox)
					   .MinDesiredHeight(40.f)
				       .MinDesiredWidth(40.f)
				   ]*/

class SSingleBoneItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSingleBoneItem) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const int32 InBoneIndex, const bool DefaultSelected, const FName InBoneName)//, TWeakPtr<SMotionFieldCreateDialog> InMotionFieldEditorPtr)
	{
		BoneName = InBoneName;

		//MotionFieldEditorPtr = InMotionFieldEditorPtr;
		bIsMotion = false;

		const FLinearColor BackgroundColors[2] = { FLinearColor(1.0f, 1.0f, 1.0f, 0.05f), FLinearColor(0.0f, 0.0f, 0.0f, 0.05f) };
		const FLinearColor& BackgroundColorForFrameIndex = BackgroundColors[InBoneIndex & 1];


		ChildSlot
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
			[
				SNew(SColorBlock)
				.Color(BackgroundColorForFrameIndex)
			]
				+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
		    [
				SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromName(BoneName))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
			]
			  + SHorizontalBox::Slot()
				  .FillWidth(1.f)
				  .HAlign(HAlign_Center)
		    [
				
				SNew(SOverlay)
				+ SOverlay::Slot()
				  [
					  SNew(SCheckBox)
					  .OnCheckStateChanged(this, &SSingleBoneItem::OnCheckIsMotion)
				   .IsChecked(ECheckBoxState::Unchecked)
				  .Padding(10.f)
				  .ForegroundColor(this, &SSingleBoneItem::GetIsMotionColor)
				  ]
			 
				  
		    ]
			]
			]
		
			
			];
			
	}

	FReply BoneItemClicked()
	{
		return FReply::Handled();
	}

	void OnCheckIsMotion(ECheckBoxState NewState)
	{
		if (NewState == ECheckBoxState::Checked)
		{
			bIsMotion = true;
		}
		else if (NewState == ECheckBoxState::Unchecked)
		{
			bIsMotion = false;
		}
	}

	FReply OnClickIsMotionKey()
	{
		bIsMotion = !bIsMotion;
		return FReply::Handled();
	}

	FSlateColor GetIsMotionColor() const
	{
		if (bIsMotion)
		{
			return FSlateColor(FLinearColor::Green);
		}
		return FSlateColor(FLinearColor::Red);
	}

	FName BoneName;

	//bool bIsKey;
	bool bIsMotion;
protected:

	//TSharedPtr<FUICommandList> CommandList;
private:
	//TWeakPtr<SMotionFieldCreateDialog> MotionFieldEditorPtr;
};



class SMotionFieldCreateDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMotionFieldCreateDialog) {}

	SLATE_END_ARGS()

		// Constructs this widget with InArgs 
		void Construct(const FArguments& InArgs, const bool Ow)
	{
		bOkClicked = false;

		ChildSlot
			[
				SNew(SBorder)
				.Visibility(EVisibility::Visible)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				SNew(SBox)
				.Visibility(EVisibility::Visible)
			.WidthOverride(500.0f)
			[
				SNew(SVerticalBox)
	      	+ SVerticalBox::Slot()
			.FillHeight(1)
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.Content()
			[
				SAssignNew(SkeletonContainer, SVerticalBox)
			]
			]
		    
		    + SVerticalBox::Slot()
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.Content()
			[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
				.FillHeight(0.1f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Bone_Name", "Bone Name"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				]
			+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Is_Motion_Bone", "Is Motion Bone"))
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				]
				]
			    + SVerticalBox::Slot()
					.FillHeight(0.9f)
				[
				SNew(SScrollBox)
				+SScrollBox::Slot()
				[
				SAssignNew(SkeletonBoneContainer, SVerticalBox)
				]
				]
			]
			]
		// Ok/Cancel buttons
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(8)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
			.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
			.OnClicked(this, &SMotionFieldCreateDialog::OkClicked)
			.Text(LOCTEXT("CreateMotionFieldOk", "OK"))
			]
		+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
			.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
			.OnClicked(this, &SMotionFieldCreateDialog::CancelClicked)
			.Text(LOCTEXT("CreateMotionFieldCancel", "Cancel"))
			]
			]
			]
			]
			];

		//MakeParentClassPicker();
		MakeSkeletonPicker();
		RebuildBonePicker();
	}

	// Sets properties for the supplied MotionFieldFactory 
	bool ConfigureProperties(TWeakObjectPtr<UMotionFieldFactory> InMotionFieldFactory)
	{
		MotionFieldFactory = InMotionFieldFactory;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(LOCTEXT("CreateMotionFieldOptions", "Create Animation Blueprint"))
			.ClientSize(FVector2D(400, 700))
			.SupportsMinimize(false).SupportsMaximize(false)
			[
				AsShared()
			];

		PickerWindow = Window;

		GEditor->EditorAddModalWindow(Window);
		MotionFieldFactory.Reset();

		return bOkClicked;
	}

private:
	class FMotionFieldParentFilter : public IClassViewerFilter
	{
	public:
		// All children of these classes will be included unless filtered out by another setting. 
		TSet< const UClass* > AllowedChildrenOfClasses;
		const FAssetData& ShouldBeCompatibleWithSkeleton;

		FMotionFieldParentFilter(const FAssetData& Skeleton) : ShouldBeCompatibleWithSkeleton(Skeleton) {}

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			// If it appears on the allowed child-of classes list (or there is nothing on that list)
			if (InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed)
			{
				return true;// return CanCreateMotionField(ShouldBeCompatibleWithSkeleton, InClass);
			}
			return false;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			// If it appears on the allowed child-of classes list (or there is nothing on that list)
			return InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
		}
	};

	// Handler for when a parent class is selected 
	void OnClassPicked(UClass* ChosenClass)
	{
		MakeSkeletonPicker();
	}

	// Creates the combo menu for the target skeleton 
	void MakeSkeletonPicker()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

		FAssetPickerConfig AssetPickerConfig;
		AssetPickerConfig.Filter.ClassNames.Add(USkeleton::StaticClass()->GetFName());
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SMotionFieldCreateDialog::OnSkeletonSelected);
		AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SMotionFieldCreateDialog::FilterSkeletonBasedOnParentClass);
		AssetPickerConfig.bAllowNullSelection = true;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
		AssetPickerConfig.InitialAssetSelection = TargetSkeleton;

		SkeletonContainer->ClearChildren();
		SkeletonContainer->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TargetSkeleton", "Target Skeleton:"))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			];

		SkeletonContainer->AddSlot()
			[
				ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
			];
	}

	void RebuildBonePicker()
	{
		BoneItems.Empty();

		SkeletonBoneContainer->ClearChildren();
		
		if (TargetSkeleton.IsValid())
		{
			if (!TargetSkeleton.IsAssetLoaded())
			{
				TargetSkeleton.GetPackage()->FullyLoad();
			}
			USkeleton* Skel = Cast<USkeleton>(TargetSkeleton.GetAsset());
			if (Skel)
			{

			

				for (int i = 1; i < Skel->GetReferenceSkeleton().GetNum(); i++)
				{

					BoneItems.Add(SNew(SSingleBoneItem, i, false, Skel->GetReferenceSkeleton().GetBoneName(i)));
					
					SkeletonBoneContainer->AddSlot()
						[
							BoneItems.Last().ToSharedRef()
							/*
							SNew(SButton)
							[
								SNew(STextBlock)
								.Text(FText::FromName(Skel->GetReferenceSkeleton().GetBoneName(i)))
						        .ShadowOffset(FVector2D(1.0f, 1.0f))
							]
							*/
						];

				}
			}
		}
	}

	bool FilterSkeletonBasedOnParentClass(const FAssetData& AssetData)
	{
		return false;// !CanCreateMotionField(AssetData, ParentClass.Get());
	}

	// Handler for when a skeleton is selected 
	void OnSkeletonSelected(const FAssetData& AssetData)
	{
		TargetSkeleton = AssetData;
		RebuildBonePicker();
	}

	// Handler for when ok is clicked 
	FReply OkClicked()
	{
		MotionFieldFactory->TargetMotionBones.Empty();
		if (BoneItems.Num() > 0)
		{
			for (int i = 0; i < BoneItems.Num(); i++)
			{
				if (BoneItems[i].IsValid())
				{
					if (BoneItems[i].Get()->bIsMotion)
					{
						MotionFieldFactory->TargetMotionBones.Add(BoneItems[i].Get()->BoneName);
					}
				}
			}
		}
		if (MotionFieldFactory.IsValid())
		{
			MotionFieldFactory->TargetSkeleton = Cast<USkeleton>(TargetSkeleton.GetAsset());
		}

		if (!TargetSkeleton.IsValid())
		{
			// if TargetSkeleton is not valid
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidSkeleton", "Must specify a valid skeleton for the Anim Blueprint to target."));
			return FReply::Handled();
		}
		
		CloseDialog(true);

		return FReply::Handled();
	}

	void CloseDialog(bool bWasPicked = false)
	{
		bOkClicked = bWasPicked;
		if (PickerWindow.IsValid())
		{
			PickerWindow.Pin()->RequestDestroyWindow();
		}
	}

	// Handler for when cancel is clicked 
	FReply CancelClicked()
	{
		CloseDialog();
		return FReply::Handled();
	}

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			CloseDialog();
			return FReply::Handled();
		}
		return SWidget::OnKeyDown(MyGeometry, InKeyEvent);
	}

private:
	// The factory for which we are setting up properties 
	TWeakObjectPtr<UMotionFieldFactory> MotionFieldFactory;

	// A pointer to the window that is asking the user to select a parent class 
	TWeakPtr<SWindow> PickerWindow;

	// The container for the target skeleton picker
	TSharedPtr<SVerticalBox> SkeletonContainer;
	TSharedPtr<SVerticalBox> SkeletonBoneContainer;
	TArray <TSharedPtr <SSingleBoneItem>> BoneItems;
	//TSharedPtr<STreeView<TSharedPtr<int>>> Ow;
	
	// The selected skeleton 
	FAssetData TargetSkeleton;
	TArray <FName> TargetMotionBones;
	// True if Ok was clicked 
	bool bOkClicked;
};

//////////////////////////////////////////////////////////



//UMotionFieldFactory implementation.

UMotionFieldFactory::UMotionFieldFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMotionField::StaticClass();
}

bool UMotionFieldFactory::ConfigureProperties()
{
	
	TSharedRef<SMotionFieldCreateDialog> Dialog = SNew(SMotionFieldCreateDialog, true);
	return Dialog->ConfigureProperties(this);
};

UObject* UMotionFieldFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	
	UMotionField* NewMotionField = NewObject<UMotionField>(InParent, Class, Name, Flags | RF_Transactional);
	////////////////////////////
	if(NewMotionField && TargetSkeleton)
	{

		NewMotionField->PopulateFromSkeleton(TargetSkeleton, TargetMotionBones);
	
		return NewMotionField;
	}
		

	return NULL;
}

UObject* UMotionFieldFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE
