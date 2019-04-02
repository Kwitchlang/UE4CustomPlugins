// Copyright 2019 FeiSu. All Rights Reserved.

#include "FormatterStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

void FFormatterStyle::Initialize()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = MakeShareable(new FSlateStyleSet("GraphFormatterStyle"));
		StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate/Icons"));
		StyleSet->Set("GraphFormatter.ApplyIcon.Small", new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Profiler/profiler_Calls_32x"), TEXT(".png")), FVector2D(16.0f, 16.0f)));
		StyleSet->Set("GraphFormatter.ApplyIcon", new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Profiler/profiler_Calls_32x"), TEXT(".png")), FVector2D(32.0f, 32.0f)));
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
	}
}

void FFormatterStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

TSharedPtr<ISlateStyle> FFormatterStyle::Get()
{
	return StyleSet;
}

TSharedPtr<class FSlateStyleSet > FFormatterStyle::StyleSet = nullptr;

