// Copyright 2019 FeiSu. All Rights Reserved.

#include "FormatterSettings.h"

UFormatterSettings::UFormatterSettings()
	: PositioningAlgorithm(EGraphFormatterPositioningAlgorithm::EPriorityMethod)
	, CommentBorder(45)
	, HorizontalSpacing(100)
	, VerticalSpacing(80)
	, MaxOrderingIterations(10)
{
}
