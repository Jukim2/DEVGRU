// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TileStruct.h"
#include "GameFramework/Actor.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UInventoryComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetupInputBindings(UInputComponent* InputComponent);

	// Get & Add & Remove Item
	TMap<class AItemObject*, FTileStruct> GetAllItems();
	AItemObject* GetItemAtIndex(int32 index);
	void RemoveItem(AItemObject* ItemObject);
	bool TryAddItem(class AItemObject* ItemObject);
	void AddItemAt(AItemObject* ItemObject, int32 TopLeftIndex);

	UFUNCTION(Server, Reliable)
	void ServerSpawnItem(FVector3d SpawnLocation, TSubclassOf<AItem> ItemObjectClass);

	// Util Functions
	bool IsRoomAvailable(AItemObject* ItemObject, int32 TopLeftIndex);
	FTileStruct IndexToTile(int32 index);
	int32 TileToIndex(FTileStruct Tile);
	bool IsTileValid(FTileStruct Tile);
	UFUNCTION(Server, Reliable)
	void ServerDestroyItem(AItem* ItemToRemove);
	
	// Event
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryChanged OnInventoryChanged;
	
private:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	class AShooterCharacter* OwnerShooterCharacter;
	UPROPERTY(VisibleAnywhere, Category = Input)
	class UInputAction* IAInventory;
	
	// Properties
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 Columns = 6;
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 Rows = 5;
	
	TArray<AItemObject*> Items;
	bool isDirty = false;

	// Inventory
	UPROPERTY(EditAnywhere, Category="Widget")
	TSubclassOf<class UUserWidget> InventoryWidgetClass;

	UPROPERTY(EditAnywhere)
	float TileSize = 50.f;

	UPROPERTY()
	class UInventory* Inventory;

	void CreateInventoryWidget();
	void ToggleInventory();
	
public:
	int32 GetColumns() const { return Columns; }
	int32 GetRows() const { return Rows; }
	void SetOwnerShooterCharacter(AShooterCharacter* Owner) { OwnerShooterCharacter = Owner; }
};
