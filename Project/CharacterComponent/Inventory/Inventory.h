// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryGrid.h"
#include "Inventory.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API UInventory : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void InitInventory();


private:
	UPROPERTY()
	class UInventoryComponent* InventoryComponent;
	UPROPERTY(meta = (BindWidget))
	UInventoryGrid* WBP_InventoryGrid;
	UPROPERTY(meta = (BindWidget))
	UBorder* BackgroundBorder;

	float TileSize = 50.f;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
public:
	void SetInventoryComponent(UInventoryComponent* _InventoryComponent) { InventoryComponent = _InventoryComponent; }
	void SetTileSize(float _TileSize) { TileSize = _TileSize; }
};
