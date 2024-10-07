// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputAction.h"
#include "WeaponManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_API UWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponManagerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetupInputBindings(UInputComponent* InputComponent);
	
	// Set Weapon
	UFUNCTION(Server, Reliable)
	void ServerSetWeapons();
	
	void AttachWeaponToRightHand(class AWeapon* WeaponToAttach);
	
	void TryEquipWeapon(const FInputActionValue& Value);
	void PlayEquipWeaponSound(class AWeapon* WeaponToEquip);

	void RequestSwapWeapon(EWeaponType WeaponType);
	void SwapWeapon();
	UFUNCTION(BlueprintCallable)
	void SwapUnequip();
	UFUNCTION(BlueprintCallable)
	void SwapEquip();

	void UpdateCarriedAmmo();
	void UpdateAmmoValues();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class AShooterCharacter* OwnerShooterCharacter;
	UPROPERTY()
	class AShooterController* Controller;
	UPROPERTY()
	class AShooterHUD* HUD;
	
	// Weapon Class
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AWeapon> RifleWeapon;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> PistolWeapon;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> GrenadeWeapon;
	
	// Weapon Map
	TMap<EWeaponType, AWeapon*> WeaponMap;
	
	// Equip
	void EquipWeapon(AWeapon* WeaponToEquip);
	void StoreWeapon(AWeapon* WeaponToEquip);
	void AttachActorToBackpack(AWeapon* ActorToAttach);
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_StorePistol)
	AWeapon* StoredPistol;
	UPROPERTY(ReplicatedUsing = OnRep_StoreGrenade)
	AWeapon* StoredGrenade;

	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION()
	void OnRep_StorePistol();
	UFUNCTION()
	void OnRep_StoreGrenade();

	// Swap
	void SwapToRifle(const FInputActionValue& Value);
	void SwapToPistol(const FInputActionValue& Value);
	void SwapToGrenade(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void ServerSwapWeapon(EWeaponType WeaponType);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSwapWeapon(EWeaponType WeaponType);
	UFUNCTION(BlueprintCallable)
	void SwapEnd();
	EWeaponType CurrWeaponType;
	EWeaponType NextWeaponType;

	// Ammo
	TMap<EWeaponType, int32> CarriedAmmoMap;
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	UFUNCTION()
	void OnRep_CarriedAmmo();

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 15;
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeAmmo = 3;
	
	void InitializeCarriedAmmo();
	int32 AmountToReload();

	// WeaponUI
	void SetWeaponUI(AWeapon* Weapon, float Opacity);
	void SetWeaponUIOpacity(int8 WeaponIdx, float Opacity);
	void SwapWeaponUI();

	UPROPERTY(VisibleAnywhere, Category = Input)
	class UInputAction* IAEquip;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* IASwapToRifle;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* IASwapToPistol;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* IASwapToGrenade;

public:
	AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	void SetOwnerShooterCharacter(AShooterCharacter* Owner) { OwnerShooterCharacter = Owner; }
	EWeaponType GetCurrWeaponType() const { return CurrWeaponType; }
	EWeaponType GetNextWeaponType() const { return NextWeaponType; }
	int32 GetCarriedAmmo() const { return CarriedAmmo; }
	TSubclassOf<AWeapon> GetRifleClass() const { return RifleWeapon; }
	TSubclassOf<AWeapon> GetPistolClass() const { return PistolWeapon; }
	TSubclassOf<AWeapon> GetGrenadeClass() const { return GrenadeWeapon; }
	
};
