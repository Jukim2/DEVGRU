// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"
#include "Project/CharacterComponent/Inventory/InventoryComponent.h"

#include "ItemWidget.h"
#include "Blueprint/DragDropOperation.h"


void UInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}							

void UInventory::InitInventory()
{
	WBP_InventoryGrid->InitInventoryGrid(InventoryComponent, TileSize);
}

FReply UInventory::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (BackgroundBorder && BackgroundBorder->IsHovered())
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

bool UInventory::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	if (!InOperation || !WBP_InventoryGrid) return false;
	
	FVector2D ScreenSpacePosition = InDragDropEvent.GetScreenSpacePosition();
	FGeometry GridGeometry = WBP_InventoryGrid->GetCachedGeometry();
	bool bIsMouseOverGrid = GridGeometry.IsUnderLocation(ScreenSpacePosition);
	
	if (bIsMouseOverGrid)
	{
		return false;
	}

	// Drop
	AItemObject* ItemObject = Cast<AItemObject>(InOperation->Payload);
	AActor* Actor = InventoryComponent->GetOwner();
	FVector3d SpawnLocation =  Actor->GetActorLocation() + Actor->GetActorForwardVector() * 150.f;

	InventoryComponent->ServerSpawnItem(SpawnLocation, ItemObject->GetItemClass());
	return true;
}

