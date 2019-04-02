// Copyright 2019 FeiSu. All Rights Reserved.

#include "FormatterCommands.h"

#define LOCTEXT_NAMESPACE "GraphFormatter"

void FFormatterCommands::RegisterCommands()
{
	UI_COMMAND(FormatGraph, "Format Graph", "Auto format graph using Layered Graph Drawing algorithm", EUserInterfaceActionType::Button, FInputChord(EKeys::F, false, false, true, false));
}

#undef LOCTEXT_NAMESPACE
