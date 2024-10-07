// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Project/Weapon/RangedWeapon/RangedWeapon.h"
#include "Grenade.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API AGrenade : public ARangedWeapon
{
	GENERATED_BODY()
	
public:
	virtual void AttackInitiated() override;
	virtual void AttackStopped() override;

private:
	class AShooterCharacter* Character;
	UFUNCTION(BlueprintCallable)
	void FireGrenade();
};
