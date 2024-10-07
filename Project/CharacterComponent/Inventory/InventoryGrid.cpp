// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryGrid.h"
#include "ItemWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Slate/SlateBrushAsset.h"

void UInventoryGrid::InitInventoryGrid(UInventoryComponent* _InventoryComponent, float _TileSize)
{
	this->InventoryComponent = _InventoryComponent;
	this->TileSize = _TileSize;
	
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(GridBorder->Slot);
	if (CanvasSlot && InventoryComponent)
	{
		FVector2D NewSize;
		NewSize.X = _TileSize * _InventoryComponent->GetColumns();
		NewSize.Y = _TileSize * _InventoryComponent->GetRows();
		
		CanvasSlot->SetSize(NewSize);
		CreateLineSegments();
		Refresh();
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryGrid::Refresh);
	}
}

void UInventoryGrid::CreateLineSegments()
{
	AddVerticalLine();
	AddHorizontalLine();
}

void UInventoryGrid::AddVerticalLine()
{
	for (size_t idx = 0; idx <= InventoryComponent->GetColumns(); idx++)
	{
		float X = idx * TileSize;
		FVector2D Start (X, 0);
		FVector2D End (X, InventoryComponent->GetRows() * TileSize);

		FLineStruct NewLine(Start, End);
		NewLine.Start = Start;
		NewLine.End = End;
		
		Lines.Add(NewLine);
	}
}

void UInventoryGrid::AddHorizontalLine()
{
	for (size_t idx = 0; idx <= InventoryComponent->GetRows(); idx++)
	{
		float Y = idx * TileSize;
		FVector2D Start (InventoryComponent->GetColumns() * TileSize, Y);
		FVector2D End (0, Y);

		FLineStruct NewLine;
		NewLine.Start = Start;
		NewLine.End = End;
		
		Lines.Add(NewLine);
	}
}

int32 UInventoryGrid::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                  const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                  const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 CurrentLayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	
	for (const FLineStruct& Line : Lines)
	{
		FVector2D BorderTopLeft = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0, 0));
		
		FVector2D Start = BorderTopLeft + Line.Start;
		FVector2D End = BorderTopLeft + Line.End;
		
		TArray<FVector2D> Points;
		Points.Add(Start);
		Points.Add(End);
		
		FLinearColor LineColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.25f);
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			CurrentLayerId,
			AllottedGeometry.ToPaintGeometry(),
			Points,
			ESlateDrawEffect::None,
			LineColor,
			true,
			1.0f
		);
	}
	if (DrawDropLocation && CurrentDraggingObject)
	{
		FVector2d Position(DraggedItemTopLeftTile * TileSize);
		FVector2d Size(CurrentDraggingObject->GetDimensions().X * TileSize, CurrentDraggingObject->GetDimensions().Y * TileSize);
		FLinearColor BoxColor;
		if (IsRoomAvailableForPayload(CurrentDraggingObject))
		{
			BoxColor = FLinearColor(0.5f, 1.0f, 0.5f, 0.5f);
		}
		else
		{
			BoxColor = FLinearColor(1.0f, 0.5f, 0.5f, 0.5f);
		}
		FSlateBrush SlateBrush = SlateBrushAsset->Brush;
		FSlateDrawElement::MakeBox(
				   OutDrawElements,
				   CurrentLayerId,
				   AllottedGeometry.ToPaintGeometry(Position, Size),
				   &SlateBrush,
				   ESlateDrawEffect::None,
				   BoxColor
			   );
		
	}

	return CurrentLayerId + 1;
}

void UInventoryGrid::Refresh()
{
	TMap<AItemObject*, FTileStruct> ItemMap = InventoryComponent->GetAllItems();
	GridCanvasPanel->ClearChildren();

	for (auto& Elem : ItemMap)
	{
		AItemObject* ItemObject = Elem.Key;
		FTileStruct TopLeftTile = Elem.Value;
		
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController && ItemWidgetClass)
		{
			UItemWidget* ItemWidget = CreateWidget<UItemWidget>(PlayerController, ItemWidgetClass);
			ItemWidget->SetVariables(TileSize, ItemObject);
			ItemWidget->Refresh();

			ItemWidget->OnWidgetDestroyed.AddDynamic(this, &UInventoryGrid::OnItemWidgetDestroyed);

			UPanelSlot* PanelSlot = GridCanvasPanel->AddChild(ItemWidget);
			Cast<UCanvasPanelSlot>(PanelSlot)->SetAutoSize(true);
			Cast<UCanvasPanelSlot>(PanelSlot)->SetPosition(FVector2D(TopLeftTile.X * TileSize, TopLeftTile.Y * TileSize));
		}
	}
}

void UInventoryGrid::OnItemWidgetDestroyed(AItemObject* RemovedItem)
{
	InventoryComponent->RemoveItem(RemovedItem);
}

bool UInventoryGrid::IsRoomAvailableForPayload(AItemObject* Payload) const
{
	if (!Payload) return false;

	FTileStruct Tile;
	Tile.X = DraggedItemTopLeftTile.X;
	Tile.Y = DraggedItemTopLeftTile.Y;
	
	int32 index = InventoryComponent->TileToIndex(Tile);

	return InventoryComponent->IsRoomAvailable(Payload, index);
}

FReply UInventoryGrid::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (GridBorder && GridBorder->IsHovered())
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

bool UInventoryGrid::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	AItemObject* Payload = GetPayload(InOperation);
	if (IsRoomAvailableForPayload(Payload))
	{
		FTileStruct Tile;
		Tile.X = DraggedItemTopLeftTile.X;
		Tile.Y = DraggedItemTopLeftTile.Y;
	
		int32 index = InventoryComponent->TileToIndex(Tile);
		InventoryComponent->AddItemAt(Payload, index);
	}
	else
	{
		InventoryComponent->TryAddItem(Payload);
	}
	CurrentDraggingObject = nullptr;
	CurrentDraggingItemWidget = nullptr;
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

bool UInventoryGrid::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation) return false;
	
	FVector2D MousePosition = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	bool Right = FMath::Fmod(MousePosition.X, TileSize) > TileSize / 2.f;
	bool Down = FMath::Fmod(MousePosition.Y, TileSize) > TileSize / 2.f;

	FIntPoint Dimension = Cast<AItemObject>(InOperation->Payload)->GetDimensions();
	Dimension.X = FMath::Clamp(Dimension.X, 0, Dimension.X - Right);
	Dimension.Y = FMath::Clamp(Dimension.Y, 0, Dimension.Y - Down);
	
	DraggedItemTopLeftTile.X = MousePosition.X / TileSize - Dimension.X / 2;
	DraggedItemTopLeftTile.Y = MousePosition.Y / TileSize - Dimension.Y / 2;
	
	return true;
}

void UInventoryGrid::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	CurrentDraggingObject = GetPayload(InOperation);
	CurrentDraggingItemWidget = Cast<UItemWidget>(InOperation->DefaultDragVisual);
	DrawDropLocation = true;
}

void UInventoryGrid::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	DrawDropLocation = false;
}

FReply UInventoryGrid::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::R)
	{
		if (CurrentDraggingObject && CurrentDraggingItemWidget)
		{
			CurrentDraggingObject->Rotate();
			CurrentDraggingItemWidget->Refresh();
		}
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

AItemObject* UInventoryGrid::GetPayload(UDragDropOperation* Operation)
{
	if (!Operation) return nullptr;

	return Cast<AItemObject>(Operation->Payload);
}


