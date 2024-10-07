

#include "WeaponActionComponent.h"

#include "EnhancedInputComponent.h"
#include "Project/Weapon/Weapon.h"
#include "Project/Weapon/RangedWeapon/RangedWeapon.h"
#include "Project/Weapon/RangedWeapon/Gun/Gun.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/Controller/ShooterController.h"
#include "Project/ShooterTypes/CharacterAction.h"

#include "WeaponManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

#define TRACE_LENGTH 80000.f;

UWeaponActionComponent::UWeaponActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Aim
	(TEXT("/Game/Character/Input/Actions/IA_Aim"));
	if (IA_Aim.Succeeded())
	{
		IAAim = IA_Aim.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Fire
	(TEXT("/Game/Character/Input/Actions/IA_Fire"));
	if (IA_Fire.Succeeded())
	{
		IAFire = IA_Fire.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Reload
	(TEXT("/Game/Character/Input/Actions/IA_Reload"));
	if (IA_Reload.Succeeded())
	{
		IAReload = IA_Reload.Object;
	}
}

void UWeaponActionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponActionComponent::SetAimDownSight(float DeltaTime)
{
	if (OwnerShooterCharacter && OwnerShooterCharacter->GetFollowCamera()
		&& OwnerShooterCharacter->GetEquippedWeapon() && OwnerShooterCharacter->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_Rifle)
	{
		UCameraComponent* FollowCamera = OwnerShooterCharacter->GetFollowCamera();
		if (bAiming)
		{
			FVector LerpedLocation = FMath::Lerp(FollowCamera->GetComponentLocation(), IronSightLocation, 10.f * DeltaTime);
			FRotator LerpedRotation = FMath::Lerp(FollowCamera->GetComponentRotation(), LookAtRotation, 10.f * DeltaTime);
			FollowCamera->SetWorldLocation(LerpedLocation);
			FollowCamera->SetWorldRotation(LerpedRotation);
		}
		else
		{
			FVector LerpedLocation = FMath::Lerp(FollowCamera->GetComponentLocation(), ViewLocation, 10.f * DeltaTime);
			FRotator LerpedRotation = FMath::Lerp(FollowCamera->GetComponentRotation(), ViewRotation, 10.f * DeltaTime);
			FollowCamera->SetWorldLocation(LerpedLocation);
			FollowCamera->SetWorldRotation(LerpedRotation);
		}
	}
}

void UWeaponActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TraceUnderCrosshairs(HitResult);
	if (OwnerShooterCharacter && OwnerShooterCharacter->IsLocallyControlled())
	{
		CalcHandSway(DeltaTime);
		SetHUDCrosshairs(DeltaTime);
	}
	CalcAimDownSight();
	SetAimDownSight(DeltaTime);
}

void UWeaponActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponActionComponent, bAiming);
}

void UWeaponActionComponent::SetupInputBindings(UInputComponent* InputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(IAAim, ETriggerEvent::Started, this, &UWeaponActionComponent::AimButtonPressed);
		EnhancedInputComponent->BindAction(IAAim, ETriggerEvent::Completed, this, &UWeaponActionComponent::CompleteAiming);
		EnhancedInputComponent->BindAction(IAFire, ETriggerEvent::Started, this, &UWeaponActionComponent::AttackButtonPressed);
		EnhancedInputComponent->BindAction(IAFire, ETriggerEvent::Completed, this, &UWeaponActionComponent::AttackButtonReleased);
		EnhancedInputComponent->BindAction(IAReload, ETriggerEvent::Started, this, &UWeaponActionComponent::ReloadButtonPressed);
	}
}

bool UWeaponActionComponent::CanPerformAction(ECharacterAction TargetAction)
{
	if (OwnerShooterCharacter && OwnerShooterCharacter->GetbDisableGameplay()) return false;

	switch (TargetAction)
	{
	case ECharacterAction::ECA_Fire:
		return !(OwnerShooterCharacter->IsSwapping() || OwnerShooterCharacter->IsReloading());
	case ECharacterAction::ECA_Equip:
		return !(OwnerShooterCharacter->IsJumping() || !OwnerShooterCharacter->IsUnoccupied());
	case ECharacterAction::ECA_Aim:
		return !(OwnerShooterCharacter->IsJumping() || !OwnerShooterCharacter->IsUnoccupied());
	case ECharacterAction::ECA_Reload:
		return !(OwnerShooterCharacter->IsJumping() || !OwnerShooterCharacter->IsUnoccupied());
	default:
		return true;
	}
}

// Aim
void UWeaponActionComponent::AimButtonPressed(const FInputActionValue& Value)
{
	if (!CanPerformAction(ECharacterAction::ECA_Aim)) return;
	
	OwnerShooterCharacter->HandleRequest(ECharacterAction::ECA_Aim);
}

void UWeaponActionComponent::Aim()
{
	bAiming = true;
	ServerSetAiming(bAiming);
	if (OwnerShooterCharacter)
	{
		OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
	}
}

void UWeaponActionComponent::CompleteAiming(const FInputActionValue& Value)
{
	bAiming = false;
	ServerSetAiming(bAiming);
	if (OwnerShooterCharacter)
	{
		OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UWeaponActionComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (OwnerShooterCharacter)
	{
		OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming == true ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UWeaponActionComponent::StopAiming()
{
	bAiming = false;
	ServerSetAiming(bAiming);
	if (OwnerShooterCharacter)
	{
		OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UWeaponActionComponent::CalcAimDownSight()
{	
	AWeapon* EquippedWeapon = OwnerShooterCharacter->GetEquippedWeapon();

	if (!EquippedWeapon || EquippedWeapon->GetWeaponType() != EWeaponType::EWT_Rifle)
		return;
		
	if (bAiming)
	{
		IronSightLocation = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("IronSight")).GetLocation();
		FRotator IronSightRotation = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("IronSight")).GetRotation().Rotator();

		FVector CameraLocation = OwnerShooterCharacter->GetFollowCamera()->GetComponentLocation();
		FRotator CameraRotation = OwnerShooterCharacter->GetFollowCamera()->GetComponentRotation();
		FVector CameraRotVec = FRotationMatrix(IronSightRotation).GetUnitAxis(EAxis::Y);
		FVector CameraForwardVector = OwnerShooterCharacter->GetFollowCamera()->GetForwardVector();

		LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CameraForwardVector, CameraRotVec + CameraForwardVector);
	}
	else
	{
		ViewLocation = OwnerShooterCharacter->GetViewComponent()->GetComponentLocation();
		ViewRotation = OwnerShooterCharacter->GetViewComponent()->GetComponentRotation();
	}
}

// Attack
void UWeaponActionComponent::AttackButtonPressed()
{
	if (!CanPerformAction(ECharacterAction::ECA_Fire)) return;
	if (!OwnerShooterCharacter || OwnerShooterCharacter->GetEquippedWeapon() == nullptr || !CanFire()) return;

	bFireButtonPressed = true;
	OwnerShooterCharacter->HandleRequest(ECharacterAction::ECA_Fire);
}

void UWeaponActionComponent::Attack()
{
	if (OwnerShooterCharacter->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_Grenade)
		AimGrenade();
	else
		Fire();
}


// AimGrenade
void UWeaponActionComponent::AimGrenade()
{
	if (CanFire())
	{
		OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_AimingGrenade);
		if (!OwnerShooterCharacter->HasAuthority())
			OwnerShooterCharacter->HandleCharacterState();
		ServerAimGrenade();
	}
}

void UWeaponActionComponent::ServerAimGrenade_Implementation()
{
	OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_AimingGrenade);
	OwnerShooterCharacter->HandleCharacterState();
}

// Fire
void UWeaponActionComponent::Fire()
{
	if (CanFire())
	{
		CrosshairShootingFactor += 1.f;
		if (!OwnerShooterCharacter->HasAuthority())
			OwnerShooterCharacter->GetEquippedWeapon()->AttackInitiated();
		ServerFire();
		StartFireTimer();
	}
}

void UWeaponActionComponent::ServerFire_Implementation()
{
	MulticastFire();
}

void UWeaponActionComponent::MulticastFire_Implementation()
{
	if (OwnerShooterCharacter && OwnerShooterCharacter->GetCharacterState() == ECharacterState::ECS_Unoccupied)
	{
		if (OwnerShooterCharacter && OwnerShooterCharacter->IsLocallyControlled() && !OwnerShooterCharacter->HasAuthority())
			return;
		OwnerShooterCharacter->GetEquippedWeapon()->AttackInitiated();
	}
}

// AttackStop
void UWeaponActionComponent::AttackButtonReleased()
{
	if (OwnerShooterCharacter->GetEquippedWeapon() == nullptr) return;

	bFireButtonPressed = false;
	if (OwnerShooterCharacter->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_Grenade)
		ThrowGrenade();
	else
		FireStop();
}

// Throw Grenade
void UWeaponActionComponent::ThrowGrenade()
{
	if (OwnerShooterCharacter->GetCharacterState() != ECharacterState::ECS_AimingGrenade) return;
	
	OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_ThrowingGrenade);
	if (!OwnerShooterCharacter->HasAuthority())
	{
		OwnerShooterCharacter->HandleCharacterState();
	}
	ServerThrowGrenade();
}

void UWeaponActionComponent::ServerThrowGrenade_Implementation()
{
	OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_ThrowingGrenade);
	OwnerShooterCharacter->HandleCharacterState();
}

// FireStop
void UWeaponActionComponent::FireStop()
{
	ServerFireStop();
}

void UWeaponActionComponent::ServerFireStop_Implementation()
{
	MulticastFireStop();
}

void UWeaponActionComponent::MulticastFireStop_Implementation()
{
	if (OwnerShooterCharacter->GetEquippedWeapon() == nullptr) return;
	OwnerShooterCharacter->GetEquippedWeapon()->AttackStopped();
}

void UWeaponActionComponent::StartFireTimer()
{
	if (OwnerShooterCharacter->GetEquippedWeapon() == nullptr) return;

	bCanFire = false;
	OwnerShooterCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UWeaponActionComponent::FireTimerFinished,
		OwnerShooterCharacter->GetEquippedWeapon()->GetFireDelay()
		);
}

void UWeaponActionComponent::FireTimerFinished()
{
	bCanFire = true;
	if (bFireButtonPressed && Cast<AGun>(OwnerShooterCharacter->GetEquippedWeapon()) && Cast<AGun>(OwnerShooterCharacter->GetEquippedWeapon())->GetbAuotomatic())
	{
		Fire();
	}
}

bool UWeaponActionComponent::CanFire()
{
	if (OwnerShooterCharacter->GetEquippedWeapon() == nullptr) return false;
	
	bool IsAmmoExist = Cast<ARangedWeapon>(OwnerShooterCharacter->GetEquippedWeapon()) && !Cast<ARangedWeapon>(OwnerShooterCharacter->GetEquippedWeapon())->IsEmpty();
	bool IsNotRangedWeapon = !Cast<ARangedWeapon>(OwnerShooterCharacter->GetEquippedWeapon());
	bool IsFirableState = OwnerShooterCharacter->GetCharacterState() == ECharacterState::ECS_Unoccupied;
	
	return (IsAmmoExist || IsNotRangedWeapon) && bCanFire && IsFirableState;
}

void UWeaponActionComponent::ThrowGrenadeFinished()
{
	OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_Unoccupied);
}

// Reload
void UWeaponActionComponent::ReloadButtonPressed()
{
	if (!CanPerformAction(ECharacterAction::ECA_Reload)) return;
	
	if (WeaponManagerComponent->GetCarriedAmmo() > 0 && OwnerShooterCharacter->GetCharacterState() == ECharacterState::ECS_Unoccupied
		&& (OwnerShooterCharacter->GetEquippedWeapon() && Cast<ARangedWeapon>(OwnerShooterCharacter->GetEquippedWeapon()) && !Cast<ARangedWeapon>(OwnerShooterCharacter->GetEquippedWeapon())->IsFull()))
	{
		OwnerShooterCharacter->HandleRequest(ECharacterAction::ECA_Reload);
	}
}

void UWeaponActionComponent::Reload()
{
	if (!OwnerShooterCharacter->HasAuthority())
	{
		OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_Reloading);
		OwnerShooterCharacter->HandleCharacterState();
		HandleReload();
	}
	ServerReload();
}

void UWeaponActionComponent::ServerReload_Implementation()
{
	OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_Reloading);
	OwnerShooterCharacter->HandleCharacterState();
	HandleReload();
}

void UWeaponActionComponent::HandleReload()
{
	if (Cast<AGun>(OwnerShooterCharacter->GetEquippedWeapon()))
		Cast<AGun>(OwnerShooterCharacter->GetEquippedWeapon())->PlayReloadAnim();
	if (OwnerShooterCharacter->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_Grenade)
		OwnerShooterCharacter->GetEquippedWeapon()->GetWeaponMesh()->SetVisibility(true);
}

void UWeaponActionComponent::FinishReloading()
{
	if (OwnerShooterCharacter == nullptr) return;

	if (OwnerShooterCharacter->HasAuthority())
	{
		OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_Unoccupied);
		WeaponManagerComponent->UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UWeaponActionComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	if (!OwnerShooterCharacter) return;
	
	FVector CrosshairWorldPosition = OwnerShooterCharacter->GetViewComponent()->GetComponentLocation();
	FVector CrosshairWorldDirection = OwnerShooterCharacter->GetViewComponent()->GetForwardVector();
	FVector Start = CrosshairWorldPosition;

	if (OwnerShooterCharacter)
	{
		float DistanceToCharacter = (OwnerShooterCharacter->GetActorLocation() - Start).Size();
		Start += CrosshairWorldDirection * (DistanceToCharacter + 50.f);
	}
	FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerShooterCharacter); 
	GetWorld()->LineTraceSingleByChannel(
		TraceHitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility,
		CollisionParams
	);
	if (!TraceHitResult.bBlockingHit)
	{
		TraceHitResult.ImpactPoint = End;
	}
	if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->ActorHasTag(FName("Player")))
	{
		HUDPackage.CrosshairColor = FLinearColor::Red;
	}
	else
	{
		HUDPackage.CrosshairColor = FLinearColor::White;
	}
}

void UWeaponActionComponent::CalcHandSway(float DeltaTime)
{
	if (!OwnerShooterCharacter || !OwnerShooterCharacter->GetEquippedWeapon()) return;
	
	float NewPitch = CalcHandSwayPitch();
	float NewRoll = CalcHandSwayRoll();
	float NewYaw = CalcHandSwayYaw();

	HandSwayRot.Pitch = FMath::FInterpTo(HandSwayRot.Pitch, NewPitch, DeltaTime, 10);
	HandSwayRot.Roll = FMath::FInterpTo(HandSwayRot.Roll, NewRoll, DeltaTime, 10);
	HandSwayRot.Yaw = FMath::FInterpTo(HandSwayRot.Yaw, NewYaw, DeltaTime, 10);
}

float UWeaponActionComponent::CalcHandSwayPitch()
{
	FVector2d OutRange;
	if (bAiming)
	{
		OutRange.X = -2.f;
		OutRange.Y = 2.f;
	}
	else
	{
		OutRange.X = -8.f;
		OutRange.Y = 8.f;
	}
	return (FMath::GetMappedRangeValueClamped(FVector2d(-1.f, 1.f), OutRange, 
	OwnerShooterCharacter->GetMoveX() + SpringValue * OwnerShooterCharacter->GetEquippedWeapon()->GetSpringPitchMultiplier()));
}

float UWeaponActionComponent::CalcHandSwayYaw()
{
	FVector2d OutRange;
	if (bAiming)
	{
		OutRange.X = 2.f;
		OutRange.Y = -2.f;
	}
	else
	{
		OutRange.X = 12.f;
		OutRange.Y = -12.f;
	}
	return (FMath::GetMappedRangeValueClamped(FVector2d(-1.f, 1.f), OutRange, 
	OwnerShooterCharacter->GetMouseX() + SpringValue * OwnerShooterCharacter->GetEquippedWeapon()->GetSpringYawMultiplier()));
}

float UWeaponActionComponent::CalcHandSwayRoll()
{
	FVector2d OutRange;
	if (bAiming)
	{
		OutRange.X = -2.f;
		OutRange.Y = 2.f;
	}
	else
	{
		OutRange.X = 20.f;
		OutRange.Y = -20.f;
	}
	return (FMath::GetMappedRangeValueClamped(FVector2d(-1.f, 1.f), OutRange, 
	OwnerShooterCharacter->GetMouseY() + SpringValue * OwnerShooterCharacter->GetEquippedWeapon()->GetSpringRollMultiplier()));
}

// Crosshairs
void UWeaponActionComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (OwnerShooterCharacter == nullptr || OwnerShooterCharacter->Controller == nullptr) return;

	OwnerShooterController = OwnerShooterController == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : OwnerShooterController;
	if (OwnerShooterController)
	{
		HUD = HUD == nullptr ? Cast<AShooterHUD>(OwnerShooterController->GetHUD()) : HUD;
		if (HUD)
		{
			if (OwnerShooterCharacter->GetEquippedWeapon() && Cast<AGun>(OwnerShooterCharacter->GetEquippedWeapon()))
			{
				AGun* EquippedGun = Cast<AGun>(OwnerShooterCharacter->GetEquippedWeapon());
				HUDPackage.CrosshairsCenter = EquippedGun->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedGun->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedGun->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedGun->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedGun->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			// Calc crosshair spread
			FVector2D WalkSpeedRange(0.f, OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.5f);
			FVector Velocity = OwnerShooterCharacter->GetVelocity();
			Velocity.Z = 0.f;
			
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			if (OwnerShooterCharacter->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, MaxCrosshairInAirfactor, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, MaxCrosshairAimfactor, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
			
			HUDPackage.CrosshairSpread = HUD->GetBaseCrosshairSpread()
			+ CrosshairVelocityFactor
			+ CrosshairInAirFactor
			- CrosshairAimFactor
			+ CrosshairShootingFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}


