// Copyright Epic Games, Inc. All Rights Reserved.

#include "IconEditorStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateStyleRegistry.h"

TUniquePtr<FIconEditorStyle> FIconEditorStyle::Instance(nullptr);
FColor FIconEditorStyle::TypeColor(104,49,178);

FIconEditorStyle::FIconEditorStyle() : FSlateStyleSet("IconEditorStyle")
{
	Set("ClassIcon.BlackboardKeyType_SOClaimHandle", new FSlateRoundedBoxBrush(FLinearColor(TypeColor), 2.5f, FVector2D(16.f, 5.f)));
	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FIconEditorStyle::~FIconEditorStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

FIconEditorStyle& FIconEditorStyle::Get()
{
	if (!Instance.IsValid())
	{
		Instance = TUniquePtr<FIconEditorStyle>(new FIconEditorStyle);
	}
	return *(Instance.Get());
}

void FIconEditorStyle::Shutdown()
{
	
}
