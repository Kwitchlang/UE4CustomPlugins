// Copyright 2019 FeiSu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FormatterSettings.generated.h"

UENUM()
enum class EGraphFormatterPositioningAlgorithm
{
	EEvenlyInLayer UMETA(DisplayName = "Place node evenly in layer"),
	EPriorityMethod UMETA(DisplayName = "Use priority method"),
};

UCLASS(config = Editor)
class GRAPHFORMATTER_API UFormatterSettings : public UObject
{
public:
	GENERATED_BODY()

	UFormatterSettings();

	/** Positioning algorithm*/
	UPROPERTY(config, EditAnywhere)
	EGraphFormatterPositioningAlgorithm PositioningAlgorithm;

	/** Border thickness */
	UPROPERTY(config, EditAnywhere, Category = "Spacing", meta = (ClampMin = 45))
	int32 CommentBorder;

	/** Spacing between two layers */
	UPROPERTY(config, EditAnywhere, Category = "Spacing", meta = (ClampMin = 0))
	int32 HorizontalSpacing;

	/** Spacing between two nodes */
	UPROPERTY(config, EditAnywhere, Category = "Spacing", meta = (ClampMin = 0))
	int32 VerticalSpacing;

	/** Vertex ordering max iterations */
	UPROPERTY(config, EditAnywhere, Category = "Performance", meta = (ClampMin = 0, ClampMax = 100))
	int32 MaxOrderingIterations;
};
