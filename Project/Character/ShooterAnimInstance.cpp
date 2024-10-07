// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "Project/Weapon/Weapon.h"
#include "Project/ShooterTypes/CharacterStates.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Project/Weapon/RangedWeapon/Gun/Gun.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!ShooterCharacter)
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	if (!ShooterCharacter) return;

	FVector Velocity = ShooterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bWeaponEquipped = ShooterCharacter->IsWeaponEquipped();
	EquippedWeapon = ShooterCharacter->GetEquippedWeapon();
	bIsCrouched = ShooterCharacter->bIsCrouched;
	bAiming = ShooterCharacter->IsAiming();
	TurningInPlace = ShooterCharacter->GetTurningInPlace();
	bElimmed = ShooterCharacter->IsElimmed();
	bRotateRootBone = ShooterCharacter->ShouldRotateRootBone();
	bIsRun = ShooterCharacter->IsRunning();
	bIsSwapping = ShooterCharacter->IsSwapping();
	AnimHandSwayRotator = ShooterCharacter->GetHandSwayRot();
	
	FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	AO_Yaw = ShooterCharacter->GetAO_Yaw();
	AO_Pitch = ShooterCharacter->GetAO_Pitch();

	// Recoil
	if (Cast<AGun>(EquippedWeapon))
	{
		AGun* EquippedGun = Cast<AGun>(EquippedWeapon);
		FollowerRecoil = EquippedGun->GetFollowerRecoil();
		LeaderRecoil = EquippedGun->GetLeaderRecoil();
	}

	// Adjust Weapon center
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ShooterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		
		ShooterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), LeftHandTransform.GetRotation().Rotator(), OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		EquippedWeapon->GetWeaponMesh()->TransformToBoneSpace(FName("LeftHandSocket"), FVector::ZeroVector, LeftHandTransform.GetRotation().Rotator(), OutPosition, OutRotation);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (ShooterCharacter->IsLocallyControlled() && !ShooterCharacter->GetbDisableGameplay())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = ShooterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - ShooterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}
	AnimIdx = CalcWeaponIdx();
	bUseFABRIK = ShooterCharacter->GetCharacterState() == ECharacterState::ECS_Unoccupied
	&& (bWeaponEquipped && EquippedWeapon->GetWeaponType() != EWeaponType::EWT_Grenade);
	bReloading = ShooterCharacter->GetCharacterState() == ECharacterState::ECS_Reloading;
	bUseAimOffset = ShooterCharacter->GetCharacterState() == ECharacterState::ECS_Unoccupied && !ShooterCharacter->GetbDisableGameplay();
}

int UShooterAnimInstance::CalcWeaponIdx() const
{
	if (!ShooterCharacter || !ShooterCharacter->GetEquippedWeapon())
		return StaticCast<int>(EWeaponType::EWT_MAX);
	return StaticCast<int>(ShooterCharacter->GetEquippedWeapon()->GetWeaponType());
}
