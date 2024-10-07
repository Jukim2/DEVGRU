// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Project/Weapon/WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Stored UMETA(DisplayName = "Stored"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class PROJECT_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	virtual void AttackInitiated();
	virtual void AttackStopped();
	virtual void SetHUDAmmo();
	
	void ShowPickupWidget(bool bShowWidget);
	void Dropped();

protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquip();
	virtual void OnDrop();
	virtual void OnStored();

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;
	UPROPERTY(VisibleAnywhere, Category = "Widget")
	class UWidgetComponent* PickupWidget;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EWeaponType WeaponType;
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;
	UFUNCTION()
	void OnRep_WeaponState();
	
	// Owner
	UPROPERTY()
	class AShooterCharacter* ShooterOwnerCharacter;
	UPROPERTY()
	class AShooterController* ShooterOwnerController;

	// WeaponUI
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UTexture2D* WeaponUI;

	// Zoom
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomInterpSpeed = 20.f;

	// Fire
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float FireDelay = .15f;

	// EquipSound
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class USoundCue* EquipSound;

	// Spring Multiplier
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float SpringPitchMultiplier = 0.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float SpringRollMultiplier = 0.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float SpringYawMultiplier = 0.f;
	
public:
	void SetWeaponState(EWeaponState State);
	float GetZoomedFOV() const { return ZoomedFOV; }
	float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	float GetFireDelay() const { return FireDelay; }
	float GetSpringPitchMultiplier() const { return SpringPitchMultiplier; }
	float GetSpringRollMultiplier() const { return SpringRollMultiplier; }
	float GetSpringYawMultiplier() const { return SpringYawMultiplier; }
	USphereComponent* GetAreaSphere() const { return AreaSphere; }
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	EWeaponType GetWeaponType() const { return WeaponType; }
	USoundCue* GetEquipSound() const { return EquipSound; }
	UTexture2D* GetWeaponUI() const { return WeaponUI; }
};
