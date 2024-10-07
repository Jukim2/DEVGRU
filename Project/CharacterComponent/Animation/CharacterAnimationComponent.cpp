

#include "CharacterAnimationComponent.h"
#include "Project/Weapon/Weapon.h"
#include "Project/Weapon/WeaponTypes.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/CharacterComponent/Weapon/WeaponManagerComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UCharacterAnimationComponent::UCharacterAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AimOffset(DeltaTime);
}

// Montage
void UCharacterAnimationComponent::PlayFireMontage(bool bAiming)
{
	UAnimInstance* AnimInstance = OwnerShooterCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("Aim") : FName("Idle");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void UCharacterAnimationComponent::PlayReloadMontage()
{
	if (!OwnerShooterCharacter) return;
	
	UAnimInstance* AnimInstance = OwnerShooterCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (OwnerShooterCharacter->GetEquippedWeapon()->GetWeaponType())
		{
		case EWeaponType::EWT_Rifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Grenade:
			SectionName = FName("Grenade");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void UCharacterAnimationComponent::PlayElimMontage()
{
	UAnimInstance* AnimInstance = OwnerShooterCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void UCharacterAnimationComponent::PlayThowGrenadeMontage(bool bThrow)
{
	UAnimInstance* AnimInstance = OwnerShooterCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
		FName SectionName = bThrow ? FName("Throw") : FName("Default");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void UCharacterAnimationComponent::PlaySwapEquipMontage()
{
	if (!OwnerShooterCharacter) return;
	
	UAnimInstance* AnimInstance = OwnerShooterCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapEquipMontage)
	{
		AnimInstance->Montage_Play(SwapEquipMontage);
		FName SectionName;
		switch (OwnerShooterCharacter->GetWeaponManager()->GetNextWeaponType())
		{
		case EWeaponType::EWT_Rifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Grenade:
			SectionName = FName("Grenade");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void UCharacterAnimationComponent::PlaySwapUnequipMontage()
{
	if (!OwnerShooterCharacter) return;

	UAnimInstance* AnimInstance =OwnerShooterCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapUnequipMontage)
	{
		AnimInstance->Montage_Play(SwapUnequipMontage);
		FName SectionName;
		switch (OwnerShooterCharacter->GetWeaponManager()->GetCurrWeaponType())
		{
		case EWeaponType::EWT_Rifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Grenade:
			SectionName = FName("Grenade");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

// Aimoffset
void UCharacterAnimationComponent::AimOffset(float DeltaTime)
{
	if (OwnerShooterCharacter->GetbDisableGameplay())
	{
		OwnerShooterCharacter->bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	FVector Velocity = OwnerShooterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = OwnerShooterCharacter->GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, OwnerShooterCharacter->GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRoatation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRoatation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		OwnerShooterCharacter->bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, OwnerShooterCharacter->GetBaseAimRotation().Yaw, 0.f);
		OwnerShooterCharacter->bUseControllerRotationYaw = true;
		AO_Yaw = 0.f;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = OwnerShooterCharacter->GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.f && !OwnerShooterCharacter->IsLocallyControlled()) // Server로 보낼때 압축
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

// Turn in place
void UCharacterAnimationComponent::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 3.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, OwnerShooterCharacter->GetBaseAimRotation().Yaw, 0.f);
		}
	}
}