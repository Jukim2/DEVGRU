// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryComponent.h"
#include "ItemWidget.h"
#include "LineStruct.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "InventoryGrid.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API UInventoryGrid : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	// Initialize
	UFUNCTION(BlueprintCallable)
	void InitInventoryGrid(UInventoryComponent* _InventoryComponent, float _TileSize);
	void CreateLineSegments();
	void AddVerticalLine();
	void AddHorizontalLine();

	// Refresh
	UFUNCTION()
	void Refresh();

	// Destroyed
	UFUNCTION()
	void OnItemWidgetDestroyed(AItemObject* RemovedItem);

	UPROPERTY(EditAnywhere)
	USlateBrushAsset* SlateBrushAsset;
private:
	// To Draw Inventory
	UPROPERTY(EditAnywhere, Category="Inventory") UInventoryComponent* InventoryComponent;
	UPROPERTY(EditAnywhere, Category="Inventory") float TileSize;
	UPROPERTY(EditAnywhere, Category="Inventory") TSubclassOf<class UUserWidget> ItemWidgetClass;

	// Widget elements
	UPROPERTY(meta = (BindWidget)) UBorder* GridBorder;
	UPROPERTY(meta = (BindWidget)) UCanvasPanel* GridCanvasPanel;

	// Inventory Line
	TArray<FLineStruct> Lines;

	bool IsRoomAvailableForPayload (AItemObject* Payload) const;

	// Mouse
	FIntPoint DraggedItemTopLeftTile;
	bool DrawDropLocation;
	UPROPERTY()
	AItemObject* CurrentDraggingObject;
	UPROPERTY()
	UItemWidget* CurrentDraggingItemWidget;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
public:
	AItemObject* GetPayload(UDragDropOperation* Operation);
};
