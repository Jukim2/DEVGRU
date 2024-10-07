// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Project/Weapon/Weapon.h"
#include "RangedWeapon.generated.h"

/**
 * 
 */
UCLASS() 
class PROJECT_API ARangedWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	virtual void AttackInitiated() override;
	virtual void AttackStopped() override;
	void SetHUDAmmo() override;
	void AddAmmo(int32 AmmoToAdd);
	bool IsFull();
	bool IsEmpty();
	
protected:
	// Ammo
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();
	
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// Projectile
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

public:
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
};
