// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Components/CanvasPanelSlot.h"

void UItemWidget::SetVariables(float _TileSize, AItemObject* _ItemObject)
{
	TileSize = _TileSize;
	ItemObject = _ItemObject;
}

void UItemWidget::Refresh()
{
	if (!ItemObject || !ItemImage) return;
	
	Size.X = ItemObject->GetDimensions().X * TileSize;
	Size.Y = ItemObject->GetDimensions().Y * TileSize;

	BackgroundSizeBox->SetWidthOverride(Size.X);
	BackgroundSizeBox->SetHeightOverride(Size.Y);

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ItemImage->Slot);
	CanvasSlot->SetSize(Size);

	FSlateBrush NewBrush = MakeBrushFromMaterial(ItemObject->GetIcon(), Size);
	ItemImage->SetBrush(NewBrush);
}

FSlateBrush UItemWidget::MakeBrushFromMaterial(UMaterialInterface* Material, FVector2D ImageSize)
{
	FSlateBrush Brush;

	if (Material)
	{
		Brush.SetResourceObject(Material);
		Brush.ImageSize = ImageSize;
		Brush.DrawAs = ESlateBrushDrawType::Image;
	}

	return Brush;
}

void UItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	
	FLinearColor Color(0.5f, 0.5f, 0.5f, 0.2f);
	Backgroundborder->SetBrushColor(Color);
	
}

void UItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	
	FLinearColor Color(0, 0, 0, 0.5f);
	Backgroundborder->SetBrushColor(Color);
}

void UItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	if (ItemObject)
	{
		UDragDropOperation* DragOperation = NewObject<UDragDropOperation>();
		DragOperation->DefaultDragVisual = this;
		DragOperation->Payload = ItemObject; 

		OutOperation = DragOperation;
		OnWidgetDestroyed.Broadcast(ItemObject);
		RemoveFromParent();
	}
} 

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}
