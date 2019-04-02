// Copyright 2019 FeiSu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPositioningStrategy.h"

class FEvenlyPlaceStrategy : public IPositioningStrategy
{
	FSlateRect PlaceNodeInLayer(TArray<FFormatterNode*>& Layer, const FSlateRect& PreBound);
	FFormatterNode* FindFirstNodeInLayeredList(TArray<TArray<FFormatterNode*> >& InLayeredNodes);
public:
	explicit FEvenlyPlaceStrategy(TArray<TArray<FFormatterNode*> >& InLayeredNodes);
};
