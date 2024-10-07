#pragma once

#include <functional>
#include "Project/ShooterTypes/TuringInPlace.h"
#include "Project/ShooterTypes/CharacterStates.h"
#include "Project/ShooterTypes/Team.h"
#include "Project/ShooterTypes/ItemTypes.h"
#include "Project/CharacterComponent/Movement//ShooterMovementComponent.h"
#include "Project/CharacterComponent/Health/HealthComponent.h"
#include "Project/CharacterComponent/Weapon/WeaponActionComponent.h"
#include "Project/CharacterComponent/Inventory/Item.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class PROJECT_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;
	
	// Handler
	void HandleRequest(ECharacterAction RequestedAction);
	void HandleCharacterState();

	// Interactable
	void AddOverlappingInteractable(IInteractableObject* Interactable);
	void RemoveOverlappingInteractable(IInteractableObject* Interactable);

	// Inventory
	void TryAddInteractableItem(AItem* Item);
	bool CheckInventory(EItemType ItemType);
	
protected:
	virtual void BeginPlay() override;
	void InitPlayerState();

private:
	// Shooter Property
	UPROPERTY()
	class AShooterController* ShooterController;
	UPROPERTY()
	class AShooterPlayerState* ShooterPlayerState;
	UPROPERTY(VisibleAnywhere)
	class UInputMappingContext* ShooterMappingContext;
	
	// Components
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* ViewSceneComponent;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SpectatorSceneComponent;
	
	UPROPERTY(VisibleAnywhere)
	class UHealthComponent* HealthComponent;
	UPROPERTY(VisibleAnywhere)
	class UInventoryComponent* InventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UShooterMovementComponent* ShooterMovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWeaponManagerComponent* WeaponManagerComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWeaponActionComponent* WeaponActionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCharacterAnimationComponent* CharacterAnimationComponent;

	// Character State
	UPROPERTY(ReplicatedUsing = OnRep_CharacterState)
	ECharacterState CharacterState = ECharacterState::ECS_Unoccupied;
	UFUNCTION()
	void OnRep_CharacterState();
	
	// Overlapping Interactable
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* IAInteract;
	TArray<IInteractableObject*> OverlappingInteractables;
	IInteractableObject* NearestInteractable;
	void Interact();
	void SetNearestInteractable();

	// Overlapping Weapon
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	
	// Elim
	void HandleElimination();

	// Util
	USceneComponent* FindSceneComponentByName(FName ComponentName);

	// Timer
	FTimerHandle RunEndTimer;
	void ExecuteDelayedAction(std::function<void()> Action, float Delay);

public: // Getter & Setter
	void SetWeapons();
	void SetOverlappingWeapon(AWeapon* Weapon);
	AWeapon* GetOverlappingWeapon() const { return OverlappingWeapon; }

	// Character Components
	AShooterController* GetShooterController() const { return ShooterController; }
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	USceneComponent* GetViewComponent() const { return ViewSceneComponent; }
	USceneComponent* GetSpectatorComponent() const { return SpectatorSceneComponent; }
	UWeaponManagerComponent* GetWeaponManager() const { return WeaponManagerComponent; }
	UWeaponActionComponent* GetWeaponActionComponent() const { return WeaponActionComponent; }
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	
	// Character State
	ECharacterState GetCharacterState() { return CharacterState; }
	void SetCharacterState(ECharacterState NewState) { CharacterState = NewState; }
	bool IsWeaponEquipped();
	bool IsAiming();
	bool IsFiring();
	bool IsJumping();
	bool IsIdle();
	bool IsSwapping();
	bool IsReloading();
	bool IsUnoccupied();
	bool IsThrowingGrenade();
	bool IsRunning();

	// Animation Component
	ETurningInPlace GetTurningInPlace();
	float GetAO_Yaw();
	float GetAO_Pitch();
	bool ShouldRotateRootBone() const;

	// Health Component
	float GetHealth() const;
	float GetMaxHealth() const;
	bool GetbDisableGameplay();
	void SetbDisableGameplay(bool val); 
	
	// MoveInput Component
	float GetMoveX() const { return ShooterMovementComponent->GetMoveX(); }
	float GetMouseX() const { return ShooterMovementComponent->GetMouseX(); }
	float GetMouseY() const { return ShooterMovementComponent->GetMouseY(); }
	
	// For Anim Instance
	UFUNCTION(BlueprintCallable)
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FRotator GetHandSwayRot() const {if (WeaponActionComponent) return WeaponActionComponent->GetHandSwayRot(); else return FRotator(); }
	bool IsElimmed() const { if (HealthComponent) return HealthComponent->GetbElimmed(); else return false; }
	bool IsDead() const {  if (HealthComponent) return HealthComponent->GetbElimmed(); else return false;}
};
