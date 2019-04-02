// Copyright 2019 FeiSu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "FormatterStyle.h"

class FFormatterCommands :public TCommands<FFormatterCommands>
{
public:
	FFormatterCommands() :
		TCommands<FFormatterCommands>("GraphFormatterEditor", NSLOCTEXT("Contexts", "GraphFormatterEditor", "Grap Formatter Editor"), NAME_None, FFormatterStyle::Get()->GetStyleSetName())
	{
	}
	TSharedPtr<FUICommandInfo> FormatGraph;
	TSharedPtr<FUICommandInfo> SelectConnectedNodes;
	TSharedPtr<FUICommandInfo> SelectSuccessorNodes;
	TSharedPtr<FUICommandInfo> SelectPredecessorNodes;
	void RegisterCommands() override;
};
