// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Inventory.h"
#include "ItemObject.h"
#include "Project/Character/ShooterCharacter.h"

#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"


UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Inventory
	(TEXT("/Game/Character/Input/Actions/IA_Inventory"));
	if (IA_Inventory.Succeeded())
	{
		IAInventory = IA_Inventory.Object;
	}
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	PrimaryComponentTick.bCanEverTick = true;
	Items.SetNum(Columns * Rows);
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (isDirty)
	{
		OnInventoryChanged.Broadcast();
		isDirty = false;
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UInventoryComponent::SetupInputBindings(UInputComponent* InputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(IAInventory, ETriggerEvent::Started, this, &UInventoryComponent::ToggleInventory);
	}
}

bool UInventoryComponent::TryAddItem(AItemObject* ItemObject)
{
	if (!ItemObject) return false;

	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (IsRoomAvailable(ItemObject, Index))
		{
			AddItemAt(ItemObject, Index);
			return true;
		}
	}
	ItemObject->Rotate();
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (IsRoomAvailable(ItemObject, Index))
		{
			AddItemAt(ItemObject, Index);
			return true;
		}
	}
	return false;
}

void UInventoryComponent::ServerSpawnItem_Implementation(FVector3d SpawnLocation, TSubclassOf<AItem> ItemObjectClass)
{
	AItem* SpawnedItem = GetWorld()->SpawnActor<AItem>(
	   ItemObjectClass,
	   SpawnLocation,         
	   FRotator::ZeroRotator
   );
}

bool UInventoryComponent::IsRoomAvailable(AItemObject* ItemObject, int32 TopLeftIndex)
{
	FTileStruct StartTile = IndexToTile(TopLeftIndex);
	FIntPoint Dimension = ItemObject->GetDimensions();
	for (int32 XIndex = StartTile.X; XIndex < StartTile.X + Dimension.X; XIndex++)
	{
		for (int32 YIndex = StartTile.Y; YIndex < StartTile.Y + Dimension.Y; YIndex++)
		{
			FTileStruct Tile;
			Tile.X = XIndex;
			Tile.Y = YIndex;

			if (!IsTileValid(Tile))
				return false;
			
			AItemObject* ItemObjectAtIdx = GetItemAtIndex(TileToIndex(Tile));
			if (ItemObjectAtIdx)
				return false;
		}
	}
	return true;
}

FTileStruct UInventoryComponent::IndexToTile(int32 index)
{
	FTileStruct NewTile;

	NewTile.X = index % Columns;
	NewTile.Y = index / Columns;

	return NewTile;
}

int32 UInventoryComponent::TileToIndex(FTileStruct Tile)
{
	return Tile.X + Tile.Y * Columns;
}

bool UInventoryComponent::IsTileValid(FTileStruct Tile)
{
	return (Tile.X >= 0 && Tile.Y >= 0 && Tile.X < Columns && Tile.Y < Rows);
}

void UInventoryComponent::ServerDestroyItem_Implementation(AItem* ItemToRemove)
{
	if (ItemToRemove && ItemToRemove->HasAuthority())
	{
		ItemToRemove->Destroy();
	}
}

void UInventoryComponent::CreateInventoryWidget()
{
	APlayerController* PlayerController = Cast<APlayerController>(OwnerShooterCharacter->Controller);
	if (PlayerController && InventoryWidgetClass)
	{
		Inventory = CreateWidget<UInventory>(PlayerController, InventoryWidgetClass);
		Inventory->SetInventoryComponent(this);
		Inventory->SetTileSize(TileSize);
		Inventory->InitInventory();
	}
}

void UInventoryComponent::ToggleInventory()
{
	if (!Inventory) CreateInventoryWidget();
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerShooterCharacter->Controller);
	if (Inventory && Inventory->IsInViewport())
	{
		Inventory->RemoveFromParent();
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(false);
	}
	else if (Inventory && !Inventory->IsInViewport())
	{
		Inventory->AddToViewport();
		FInputModeGameAndUI InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
	}
}

AItemObject* UInventoryComponent::GetItemAtIndex(int32 index)
{
	if (!Items.IsValidIndex(index)) return nullptr;

	return Items[index];
}

void UInventoryComponent::AddItemAt(AItemObject* ItemObject, int32 TopLeftIndex)
{
	FTileStruct StartTile = IndexToTile(TopLeftIndex);
	FIntPoint Dimension = ItemObject->GetDimensions();
	for (int32 XIndex = StartTile.X; XIndex < StartTile.X + Dimension.X; XIndex++)
	{
		for (int32 YIndex = StartTile.Y; YIndex < StartTile.Y + Dimension.Y; YIndex++)
		{
			FTileStruct Tile;
			Tile.X = XIndex;
			Tile.Y = YIndex;
			
			Items[TileToIndex(Tile)] = ItemObject;
		}
	}
	isDirty = true;
}

void UInventoryComponent::RemoveItem(AItemObject* ItemObject)
{
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (Items[Index] == ItemObject)
		{
			Items[Index] = nullptr;
		}
	}
	isDirty = true;
}


TMap<AItemObject*, FTileStruct> UInventoryComponent::GetAllItems()
{
	TMap<AItemObject*, FTileStruct> ItemMap;

	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		AItemObject* CurrItem = Items[Index];
		if (CurrItem && !ItemMap.Contains(CurrItem))
		{
			ItemMap.Add(CurrItem, IndexToTile(Index));
		}
	}

	return ItemMap;
}


