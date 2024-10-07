// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project/ShooterTypes/TuringInPlace.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterAnimationComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_API UCharacterAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterAnimationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Montages
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThowGrenadeMontage(bool bThrow);
	void PlaySwapUnequipMontage();
	UFUNCTION(BlueprintCallable)
	void PlaySwapEquipMontage();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class AShooterCharacter* OwnerShooterCharacter;
	
	// Montages
	UPROPERTY(VisibleAnywhere, Category="Combat")
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(VisibleAnywhere, Category="Combat")
	UAnimMontage* ReloadMontage;
	UPROPERTY(VisibleAnywhere, Category="Combat")
	UAnimMontage* ElimMontage;
	UPROPERTY(VisibleAnywhere, Category="Combat")
	UAnimMontage* ThrowGrenadeMontage;
	UPROPERTY(VisibleAnywhere, Category="Combat")
	UAnimMontage* SwapEquipMontage;
	UPROPERTY(VisibleAnywhere, Category="Combat")
	UAnimMontage* SwapUnequipMontage;

	// Aimoffset
	void AimOffset(float DeltaTime);
	
	// TurnInPlace
	bool bRotateRootBone;
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	FRotator PrevRotation;

	ETurningInPlace TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	void TurnInPlace(float DeltaTime);

public:
	void SetOwnerShooterCharacter(AShooterCharacter* Owner) { OwnerShooterCharacter = Owner; }
	ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	float GetAO_Yaw() const { return AO_Yaw; }
	float GetAO_Pitch() const { return AO_Pitch; }
	bool GetbRotateRootBone() const { return bRotateRootBone; }
};
