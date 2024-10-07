// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Project/Weapon/RangedWeapon/RangedWeapon.h"
#include "Gun.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API AGun : public ARangedWeapon
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void AttackInitiated() override;
	virtual void AttackStopped() override;
	
	// Crosshair
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;
	
	// Montage
	void PlayReloadAnim();

protected:
	// Casing
	UPROPERTY(EditAnywhere, Category = "Gun property")
	TSubclassOf<class ACasing> CasingClass;

	// Check FirstShot for Recoil
	bool bIsFirstShot = true;
	
	// Automatic fire
	UPROPERTY(EditAnywhere, Category = "Gun property")
	bool bAutomatic = true;

	// Controller Recoil
	UPROPERTY(EditAnywhere, Category = "Gun property")
	float RecoilMinPitch = -0.25f;
	UPROPERTY(EditAnywhere, Category = "Gun property")
	float RecoilMaxPitch = 0.15f;
	UPROPERTY(EditAnywhere, Category = "Gun property")
	float RecoilMinYaw = -0.15f;
	UPROPERTY(EditAnywhere, Category = "Gun property")
	float RecoilMaxYaw = 0.15f;
	UPROPERTY(EditAnywhere, Category = "Gun property")
	float RecoilMultiplier = 1.f;

	// Motion Recoil
	FTransform WeaponDefaultRecoil;
	FTransform CalcRecoil();
	FTransform FollowerRecoil;
	FTransform LeaderRecoil;
	void InterpolateWeaponRecoil(float DeltaTime);
	void AddControllerRecoil();

	UPROPERTY(EditAnywhere)
	float HandSwayRandPitch = 1.f;
	UPROPERTY(EditAnywhere)
	float HandSwayRandYaw = 1.f;
	UPROPERTY(EditAnywhere)
	float HandSwayRandRoll = 1.f;
	
	// Montage
	UPROPERTY(EditAnywhere)
	class UAnimSequence* WeaponReloadAnimation;
	
public:
	FTransform GetFollowerRecoil() const { return FollowerRecoil; }
	FTransform GetLeaderRecoil() const { return LeaderRecoil; }
	float GetHandSwayRandYaw() const { return HandSwayRandYaw; }
	float GetHandSwayRandPitch() const { return HandSwayRandPitch; }
	float GetHandSwayRandRoll() const { return HandSwayRandRoll; }
	bool GetbAuotomatic() const { return bAutomatic; }
};
