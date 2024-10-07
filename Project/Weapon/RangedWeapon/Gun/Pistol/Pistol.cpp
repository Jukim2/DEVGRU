// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"
#include "Project/Character/ShooterCharacter.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void APistol::AttackInitiated()
{
	Super::AttackInitiated();
	
	AShooterCharacter* OwnerPawn = Cast<AShooterCharacter>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (OwnerPawn->GetHitTarget() - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility
				);
			if (FireHit.bBlockingHit)
			{
				AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(FireHit.GetActor());
				if (ShooterCharacter)
				{
					if (HasAuthority())
					{
						UGameplayStatics::ApplyDamage(
							ShooterCharacter,
							Damage,
							InstigatorController,
							this,
							UDamageType::StaticClass()
							);
					}
				}
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
						);
				}
			}
		}
	}
}

void APistol::AttackStopped()
{
	Super::AttackStopped();
}
