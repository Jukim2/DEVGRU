// Fill out your copyright notice in the Description page of Project Settings.

#include "Grenade.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/Weapon/Projectile/Projectile.h"

#include "Engine/SkeletalMeshSocket.h"

void AGrenade::AttackInitiated()
{
	Super::AttackInitiated();
}

void AGrenade::AttackStopped()
{
	Super::AttackStopped();
}

void AGrenade::FireGrenade()
{
	if (!HasAuthority()) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FRotator TargetRotation = InstigatorPawn->GetControlRotation();
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
			}
		}
	}
	GetWeaponMesh()->SetVisibility(false);
}
