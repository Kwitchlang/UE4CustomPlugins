// Copyright 2019 FeiSu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "FormatterDelegates.h"

class UEdGraph;
class SGraphEditor;
class IAssetEditorInstance;

class FFormatterHacker
{
public:
	static void UpdateCommentNodes(SGraphEditor* GraphEditor, UEdGraph* Graph);
	static FFormatterDelegates GetDelegates(UObject* Object, IAssetEditorInstance* Instance);
};
