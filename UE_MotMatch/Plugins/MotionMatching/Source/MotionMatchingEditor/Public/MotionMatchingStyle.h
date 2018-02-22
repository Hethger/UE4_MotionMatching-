
#pragma once
#include "SlateBasics.h"
/**
 * 
 */
/**/
class FMotionMatchingStyle
{
public:
	static void Initialize();

	static void Shutdown();
	
	static FName GetStyleSetName();

	static TSharedPtr< class ISlateStyle > Get();

private:
	static TSharedRef< class FSlateStyleSet > Create();

private:

	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};
