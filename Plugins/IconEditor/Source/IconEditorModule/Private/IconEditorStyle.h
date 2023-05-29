// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"

class FIconEditorStyle final : public FSlateStyleSet
{
public:
	virtual ~FIconEditorStyle();

	static FIconEditorStyle& Get();
	static void Shutdown();

	static FColor TypeColor;
private:
	FIconEditorStyle();

	static TUniquePtr<FIconEditorStyle> Instance;
};
