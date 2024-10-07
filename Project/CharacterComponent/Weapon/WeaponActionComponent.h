#pragma once

#include "Project/HUD/ShooterHUD.h"
#include "Project/ShooterTypes/CharacterAction.h"

#include "InputAction.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponActionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_API UWeaponActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponActionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetupInputBindings(UInputComponent* InputComponent);
	
	void Attack();
	void AttackButtonReleased();
	void Aim();
	void StopAiming();
	void Reload();
	void HandleReload();
	
protected:
	virtual void BeginPlay() override;
	void SetAimDownSight(float DeltaTime);

private:
	UPROPERTY()
	class AShooterCharacter* OwnerShooterCharacter;
	UPROPERTY()
	class AShooterController* OwnerShooterController;
	UPROPERTY()
	class UWeaponManagerComponent* WeaponManagerComponent;
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
	FHitResult HitResult;
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// Aim
	void AimButtonPressed(const FInputActionValue& Value);
	void CompleteAiming(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	UPROPERTY(Replicated)
	bool bAiming;
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed = 200.f;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed = 125.f;
	
	// Fire
	void AttackButtonPressed();

	void Fire();
	bool bCanFire = true;
	bool bFireButtonPressed;
	FTimerHandle FireTimer;

	void StartFireTimer();
	void FireTimerFinished();
	void FireStop();
	bool CanFire();
	
	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire();

	void AimGrenade();

	UFUNCTION(Server, Reliable)
	void ServerAimGrenade();

	// FireEnd
	UFUNCTION(Server, Reliable)
	void ServerFireStop();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireStop();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	// Reload
	void ReloadButtonPressed();
	UFUNCTION(Server, Reliable)
	void ServerReload();
	
	// Hand Sway
	FRotator HandSwayRot;
	float SpringValue;

	void CalcHandSway(float DeltaTime);
	float CalcHandSwayPitch();
	float CalcHandSwayRoll();
	float CalcHandSwayYaw();

	// Crosshair
	UPROPERTY()
	class AShooterHUD* HUD;
	
	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	UPROPERTY(EditAnywhere)
	float MaxCrosshairInAirfactor = 2.25f;
	UPROPERTY(EditAnywhere)
	float MaxCrosshairAimfactor = 2.0f;
	
	void SetHUDCrosshairs(float DeltaTime);

	// ADC
	void CalcAimDownSight();
	
	FVector IronSightLocation;
	FRotator LookAtRotation;
	FVector ViewLocation;
	FRotator ViewRotation;

	// Input
	bool CanPerformAction(ECharacterAction TargetAction);
	
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* IAAim;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* IAFire;
	UPROPERTY(VisibleAnywhere, Category = Input)
	UInputAction* IAReload;

public:
	void SetOwnerShooterCharacter(AShooterCharacter* Owner) { OwnerShooterCharacter = Owner; }
	void SetWeaponManagerComponent(UWeaponManagerComponent* WeaponManager) { WeaponManagerComponent = WeaponManager; }
	bool GetbAiming() const { return bAiming; }
	bool GetbFireButtonPressed() const { return bFireButtonPressed; }
	FHitResult GetHitResult() const { return HitResult; }
	FRotator GetHandSwayRot() const { return HandSwayRot; }
};
