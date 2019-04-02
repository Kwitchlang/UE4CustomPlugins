// Copyright 2019 FeiSu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPositioningStrategy.h"

class FPriorityPositioningStrategy : public IPositioningStrategy
{
public:
	explicit FPriorityPositioningStrategy(TArray<TArray<FFormatterNode*> >& InLayeredNodes);
};
