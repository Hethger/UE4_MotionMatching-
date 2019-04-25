
#include "MotionMatchingStyle.h"
#include "MotionMatchingEditor.h"


#include "SlateGameResources.h"

#include "IPluginManager.h"

TSharedPtr< FSlateStyleSet > FMotionMatchingStyle::StyleInstance = NULL;

void FMotionMatchingStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FMotionMatchingStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

TSharedPtr< class ISlateStyle > FMotionMatchingStyle::Get() { return StyleInstance; }

FName FMotionMatchingStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("MotionMatchingStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )



TSharedRef< FSlateStyleSet > FMotionMatchingStyle::Create()
{
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);

	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("MotionMatchingStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("MotionMatching")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ClassIcon.MotionField", new IMAGE_BRUSH(TEXT("MotionField_ClassIcon"), Icon20x20));
	Style->Set("ClassThumbnail.MotionField", new IMAGE_BRUSH(TEXT("MotionField_ClassIcon"), Icon128x128));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
