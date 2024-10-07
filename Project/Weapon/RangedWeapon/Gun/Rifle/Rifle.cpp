// Fill out your copyright notice in the Description page of Project Settings.

#include "Rifle.h"
#include "Project/Weapon/Projectile/Projectile.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Project/Character/ShooterCharacter.h"

void ARifle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ARifle::AttackInitiated()
{
	Super::AttackInitiated();
	
	AShooterCharacter* InstigatorPawn = Cast<AShooterCharacter>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = InstigatorPawn->GetHitTarget() - SocketTransform.GetLocation();

		FVector StartLocation = SocketTransform.GetLocation();
		FVector EndLocation = StartLocation + ToTarget;
		
		FRotator TargetRotation = ToTarget.Rotation();
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
}

void ARifle::AttackStopped()
{
	Super::AttackStopped();
}

