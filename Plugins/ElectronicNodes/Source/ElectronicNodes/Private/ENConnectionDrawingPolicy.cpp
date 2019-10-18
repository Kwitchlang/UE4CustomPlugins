/* Copyright (C) 2019 Hugo ATTAL - All Rights Reserved
 * This plugin is downloadable from the UE4 Marketplace
 */

#include "ENConnectionDrawingPolicy.h"
#include "Misc/App.h"
#include "BlueprintEditorSettings.h"
#include "Editor.h"
#include "ENPathDrawer.h"

FConnectionDrawingPolicy* FENConnectionDrawingPolicyFactory::CreateConnectionPolicy(const class UEdGraphSchema* Schema, int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const class FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const
{
	const UElectronicNodesSettings& ElectronicNodesSettings = *GetDefault<UElectronicNodesSettings>();
	if (ElectronicNodesSettings.WireStyle == EWireStyle::Default)
	{
		return nullptr;
	}

	if (Schema->IsA(UEdGraphSchema_K2::StaticClass()) || Schema->IsA(UMaterialGraphSchema::StaticClass()))
	{
		return new FENConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, InGraphObj);
	}
	return nullptr;
}

void FENConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FVector2D& Start, const FVector2D& End, const FConnectionParams& Params)
{
	bool RightPriority = ENIsRightPriotity(Params);

	this->LayerId = LayerId;
	this->Params = &Params;
	ClosestDistanceSquared = MAX_FLT;

	FENPathDrawer PathDrawer(LayerId, ZoomFactor, RightPriority, &Params, &DrawElementsList, this);
	FVector2D StartDirection = (Params.StartDirection == EEdGraphPinDirection::EGPD_Output) ? FVector2D(1.0f, 0.0f) : FVector2D(-1.0f, 0.0f);
	FVector2D EndDirection = (Params.EndDirection == EEdGraphPinDirection::EGPD_Input) ? FVector2D(1.0f, 0.0f) : FVector2D(-1.0f, 0.0f);

	FVector2D new_Start = Start;
	FVector2D new_End = End;

	ENCorrectZoomDisplacement(new_Start, new_End);
	ENProcessRibbon(LayerId, new_Start, StartDirection, new_End, EndDirection, Params);

	const float Offset = ElectronicNodesSettings.HorizontalOffset * ZoomFactor;

	if (ElectronicNodesSettings.DisablePinOffset)
	{
		if (!((Params.AssociatedPin1 != nullptr) && (Params.AssociatedPin1->GetName() == "OutputPin")))
		{
			PathDrawer.DrawOffset(new_Start, StartDirection, Offset, false);
		}
		if (!((Params.AssociatedPin2 != nullptr) && (Params.AssociatedPin2->GetName() == "InputPin")))
		{
			PathDrawer.DrawOffset(new_End, EndDirection, Offset, true);
		}
	}
	else
	{
		PathDrawer.DrawOffset(new_Start, StartDirection, Offset, false);
		PathDrawer.DrawOffset(new_End, EndDirection, Offset, true);
	}

	EWireStyle WireStyle = ElectronicNodesSettings.WireStyle;

	if (ElectronicNodesSettings.OverwriteExecWireStyle)
	{
		if (((Params.AssociatedPin1 != nullptr) && Params.AssociatedPin1->PinType.PinCategory.ToString() == "exec") ||
			((Params.AssociatedPin2 != nullptr) && Params.AssociatedPin2->PinType.PinCategory.ToString() == "exec"))
		{
			if (ElectronicNodesSettings.WireStyleForExec != EWireStyle::Default)
			{
				WireStyle = ElectronicNodesSettings.WireStyleForExec;
			}
		}
	}

	switch (WireStyle)
	{
	case EWireStyle::Manhattan:
		PathDrawer.DrawManhattanWire(new_Start, StartDirection, new_End, EndDirection);
		break;
	case EWireStyle::Subway:
		PathDrawer.DrawSubwayWire(new_Start, StartDirection, new_End, EndDirection);
		break;
	}

	if (Settings->bTreatSplinesLikePins)
	{
		const float QueryDistanceTriggerThresholdSquared = FMath::Square(Settings->SplineHoverTolerance + Params.WireThickness * 0.5f);
		const bool bCloseToSpline = ClosestDistanceSquared < QueryDistanceTriggerThresholdSquared;

		if (bCloseToSpline)
		{
			if (ClosestDistanceSquared < SplineOverlapResult.GetDistanceSquared())
			{
				const float SquaredDistToPin1 = (Params.AssociatedPin1 != nullptr) ? (Start - ClosestPoint).SizeSquared() : FLT_MAX;
				const float SquaredDistToPin2 = (Params.AssociatedPin2 != nullptr) ? (End - ClosestPoint).SizeSquared() : FLT_MAX;

				SplineOverlapResult = FGraphSplineOverlapResult(Params.AssociatedPin1, Params.AssociatedPin2, ClosestDistanceSquared, SquaredDistToPin1, SquaredDistToPin2);
			}
		}
	}
}

void FENConnectionDrawingPolicy::ENCorrectZoomDisplacement(FVector2D& Start, FVector2D& End)
{
	if (ElectronicNodesSettings.FixZoomDisplacement)
	{
		const float ZoomDisplacement = ZoomFactor * -19.0f + 8.0f;
		if (ZoomDisplacement > 0)
		{
			Start.X += ZoomDisplacement / 2.0f;
			End.X -= ZoomDisplacement / 2.0f;
		}
	}
}

void FENConnectionDrawingPolicy::ENProcessRibbon(int32 LayerId, FVector2D& Start, FVector2D& StartDirection, FVector2D& End, FVector2D& EndDirection, const FConnectionParams& Params)
{
	int32 DepthOffsetX = 0;
	int32 DepthOffsetY = 0;
	const float SplineSize = (End - Start).Size();

	if (ElectronicNodesSettings.ActivateRibbon)
	{
		for (ENRibbonConnection RibbonConnection : RibbonConnections)
		{
			if (RibbonConnection.Horizontal)
			{
				if (FMath::Abs(Start.Y - RibbonConnection.Main) < ElectronicNodesSettings.RibbonOffset)
				{
					const float CurrentMax = FMath::Max(Start.X, End.X);
					const float CurrentMin = FMath::Min(Start.X, End.X);
					const float RibbonMax = FMath::Max(RibbonConnection.Start, RibbonConnection.End);
					const float RibbonMin = FMath::Min(RibbonConnection.Start, RibbonConnection.End);

					if (RibbonMin == CurrentMin)
					{
						continue;
					}

					if (FMath::Min(CurrentMax, RibbonMax) > FMath::Max(CurrentMin, RibbonMin) - 1.0f)
					{
						if (End.Y - RibbonConnection.Sub > 0)
						{
							DepthOffsetY = FMath::Max(DepthOffsetY, FMath::Max(1, RibbonConnection.Depth + 1));
						}
						else
						{
							DepthOffsetY = FMath::Min(DepthOffsetY, FMath::Min(-1, RibbonConnection.Depth - 1));
						}
					}
				}
			}
		}
		for (ENRibbonConnection RibbonConnection : RibbonConnections)
		{
			if (!RibbonConnection.Horizontal)
			{
				if (FMath::Abs(End.X - RibbonConnection.Main) < ElectronicNodesSettings.RibbonOffset)
				{
					const float CurrentMax = FMath::Max(Start.Y, End.Y);
					const float CurrentMin = FMath::Min(Start.Y, End.Y);
					const float RibbonMax = FMath::Max(RibbonConnection.Start, RibbonConnection.End);
					const float RibbonMin = FMath::Min(RibbonConnection.Start, RibbonConnection.End);

					if (RibbonConnection.Sub == Start.X)
					{
						continue;
					}

					if (FMath::Min(CurrentMax, RibbonMax) > FMath::Max(CurrentMin, RibbonMin) - 1.0f)
					{
						if ((Start.Y - RibbonConnection.Start) * FMath::Sign(End.Y - Start.Y) > 0)
						{
							DepthOffsetX = FMath::Max(DepthOffsetX, FMath::Max(1, RibbonConnection.Depth + 1));
						}
						else
						{
							DepthOffsetX = FMath::Min(DepthOffsetX, FMath::Min(-1, RibbonConnection.Depth - 1));
						}

						if (DepthOffsetY != 0)
						{
							DepthOffsetX = FMath::Sign(End.Y - Start.Y) * DepthOffsetY;
						}
					}
				}
			}
		}

		RibbonConnections.Add(ENRibbonConnection(Start.Y, End.Y, true, Start.X, End.X, DepthOffsetY));
		RibbonConnections.Add(ENRibbonConnection(End.X, Start.X, false, Start.Y, End.Y, DepthOffsetX));

		FVector2D StartKey(FMath::FloorToInt(Start.X), FMath::FloorToInt(Start.Y));
		FVector2D EndKey(FMath::FloorToInt(End.X), FMath::FloorToInt(End.Y));

		FENPathDrawer PathDrawer(LayerId, ZoomFactor, true, &Params, &DrawElementsList, this);

		if (DepthOffsetY != 0)
		{
			FVector2D newStart = Start;
			newStart.X += ElectronicNodesSettings.RibbonMergeOffset * ZoomFactor * StartDirection.X;
			newStart.Y += (int32)ElectronicNodesSettings.RibbonOffset * DepthOffsetY * ZoomFactor;

			PathDrawer.DrawManhattanWire(Start, StartDirection, newStart, StartDirection);

			Start = newStart;
		}

		if (DepthOffsetX != 0)
		{
			FVector2D newEnd = End;
			newEnd.X -= (int32)ElectronicNodesSettings.RibbonOffset * DepthOffsetX * ZoomFactor * EndDirection.X;

			if (DepthOffsetX * EndDirection.X > 0)
			{
				PathDrawer.DrawManhattanWire(newEnd, EndDirection, End, EndDirection);
			}

			End = newEnd;
		}
	}
}

bool FENConnectionDrawingPolicy::ENIsRightPriotity(const FConnectionParams& Params)
{
	bool RightPriority = (ElectronicNodesSettings.WireAlignment == EWireAlignment::Right);
	EWirePriority WirePriority = ElectronicNodesSettings.WirePriority;

	if (ElectronicNodesSettings.OverwriteExecWireStyle)
	{
		if (((Params.AssociatedPin1 != nullptr) && Params.AssociatedPin1->PinType.PinCategory.ToString() == "exec") ||
			((Params.AssociatedPin2 != nullptr) && Params.AssociatedPin2->PinType.PinCategory.ToString() == "exec"))
		{
			RightPriority = (ElectronicNodesSettings.WireAlignmentForExec == EWireAlignment::Right);
			WirePriority = ElectronicNodesSettings.WirePriorityForExec;
		}
	}

	if (WirePriority != EWirePriority::None)
	{
		if ((Params.AssociatedPin1 != nullptr) && (Params.AssociatedPin2 != nullptr))
		{
			bool IsOutputPin = (Params.AssociatedPin1->GetName() == "OutputPin");
			bool IsInputPin = (Params.AssociatedPin2->GetName() == "InputPin");
			if (IsOutputPin ^ IsInputPin)
			{
				switch (WirePriority)
				{
				case EWirePriority::Node:
					RightPriority = IsOutputPin;
					break;
				case EWirePriority::Pin:
					RightPriority = IsInputPin;
					break;
				}
			}
		}
	}

	return RightPriority;
}

int32 FENConnectionDrawingPolicy::ENGetZoomLevel()
{
	const float ZoomLevels[] = { 2.0f, 1.875f, 1.75f, 1.675f, 1.5f, 1.375f, 1.25f, 1.0f, 0.875f, 0.75f, 0.675f, 0.5f, 0.375f, 0.25f, 0.225f, 0.2f, 0.175f, 0.15f, 0.125f, 0.1f };

	for (int32 i = 0; i < 20; i++)
	{
		if (ZoomFactor > ZoomLevels[i] - KINDA_SMALL_NUMBER)
		{
			return 7 - i;
		}
	}

	return -12;
}

void FENConnectionDrawingPolicy::ENDrawBubbles(const FVector2D& Start, const FVector2D& StartTangent, const FVector2D& End, const FVector2D& EndTangent)
{
	if (Params->bDrawBubbles || (ElectronicNodesSettings.ForceDrawBubbles && (ElectronicNodesSettings.BubbleZoomThreshold <= ENGetZoomLevel())))
	{
		FInterpCurve<float> SplineReparamTable;
		const float SplineLength = (Start - End).Size();
		const int32 NumBubbles = FMath::CeilToInt(SplineLength / (ElectronicNodesSettings.BubbleSpace * ZoomFactor));

		const float BubbleSpeed = ElectronicNodesSettings.BubbleSpeed;
		FVector2D BubbleSize = BubbleImage->ImageSize * ZoomFactor * 0.1f * ElectronicNodesSettings.BubbleSize * FMath::Sqrt(Params->WireThickness);
		if (Params->bDrawBubbles)
		{
			BubbleSize *= 2.0f;
		}
		const float Time = (FPlatformTime::Seconds() - GStartTime);

		float AlphaOffset = FMath::Frac(Time * BubbleSpeed);

		for (int32 i = 0; i < NumBubbles; ++i)
		{
			const float Alpha = (AlphaOffset + i) / NumBubbles;
			FVector2D BubblePos;
			if (StartTangent != FVector2D::ZeroVector && EndTangent != FVector2D::ZeroVector)
			{
				if ((StartTangent != EndTangent) && ((StartTangent * EndTangent) == FVector2D::ZeroVector))
				{
					BubblePos = Start + StartTangent * FMath::Sin(Alpha * PI / 2.0f) + EndTangent * (1.0f - FMath::Cos(Alpha * PI / 2.0f));
				}
				else
				{
					BubblePos = FMath::CubicInterp(Start, StartTangent, End, EndTangent, Alpha);
				}
			}
			else
			{
				BubblePos = FMath::Lerp(Start, End, Alpha);
			}
			BubblePos -= (BubbleSize * 0.5f);

			FSlateDrawElement::MakeBox(
				DrawElementsList,
				LayerId,
				FPaintGeometry(BubblePos, BubbleSize, ZoomFactor),
				BubbleImage,
				ESlateDrawEffect::None,
				Params->WireColor
			);
		}
	}
}

void FENConnectionDrawingPolicy::ENDrawArrow(const FVector2D& Start, const FVector2D& End)
{
	if (MidpointImage != nullptr && (Start - End).Size() > 4 * MinXOffset)
	{
		const FVector2D MidpointDrawPos = (Start + End) / 2.0f - MidpointRadius * 0.75f;
		const FVector2D SlopeUnnormalized = (End - Start);
		const float AngleInRadians = FMath::Atan2(SlopeUnnormalized.Y, SlopeUnnormalized.X);

		FSlateDrawElement::MakeRotatedBox(DrawElementsList, LayerId, FPaintGeometry(MidpointDrawPos, MidpointImage->ImageSize * ZoomFactor * 0.75f, ZoomFactor * 0.75f),
			MidpointImage, ESlateDrawEffect::None, AngleInRadians, TOptional<FVector2D>(), FSlateDrawElement::RelativeToElement, Params->WireColor);
	}
}

void FENConnectionDrawingPolicy::DrawDebugPoint(const FVector2D& Position, FLinearColor Color)
{
	const FVector2D BubbleSize = BubbleImage->ImageSize * ZoomFactor * 0.1f * ElectronicNodesSettings.BubbleSize * FMath::Sqrt(Params->WireThickness);
	const FVector2D BubblePos = Position - (BubbleSize * 0.5f);

	FSlateDrawElement::MakeBox(
		DrawElementsList,
		LayerId,
		FPaintGeometry(BubblePos, BubbleSize, ZoomFactor),
		BubbleImage,
		ESlateDrawEffect::None,
		Color
	);
}

void FENConnectionDrawingPolicy::ENComputeClosestPoint(const FVector2D& Start, const FVector2D& End)
{
	const FVector2D TemporaryPoint = FMath::ClosestPointOnSegment2D(LocalMousePosition, Start, End);
	const float TemporaryDistance = (LocalMousePosition - TemporaryPoint).SizeSquared();

	if (TemporaryDistance < ClosestDistanceSquared)
	{
		ClosestDistanceSquared = TemporaryDistance;
		ClosestPoint = TemporaryPoint;
	}
}
