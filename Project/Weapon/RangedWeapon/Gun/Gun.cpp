// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "Project/Weapon/Casing/Casing.h"
#include "Project/Character/ShooterCharacter.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Project/Controller/ShooterController.h"

void AGun::BeginPlay()
{
	Super::BeginPlay();
	
	// Calc Size of Recoil
	WeaponDefaultRecoil = CalcRecoil();
}

void AGun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	InterpolateWeaponRecoil(DeltaSeconds);
}

void AGun::AttackInitiated()
{
	Super::AttackInitiated();
	
	LeaderRecoil = WeaponDefaultRecoil;

	AddControllerRecoil();
	
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				FRotator RandomRotation = SocketTransform.GetRotation().Rotator();
				RandomRotation.Yaw += FMath::RandRange(-50.0f, 50.0f);
				RandomRotation.Pitch += FMath::RandRange(-50.0f, 50.0f);
				RandomRotation.Roll += FMath::RandRange(-50.0f, 50.0f);

				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					RandomRotation
					);
			}
		}
	}
}

void AGun::AttackStopped()
{
	Super::AttackStopped();
	bIsFirstShot = true;
}

// Recoil
FTransform AGun::CalcRecoil()
{
	float Roll = FMath::RandRange(-5.0f, -2.5f) * RecoilMultiplier;
	float Pitch = FMath::RandRange(-0.8f, 0.8f) * RecoilMultiplier;
	float Yaw = FMath::RandRange(-1.6f, 1.6f) * RecoilMultiplier;
	
	FRotator RecoilRotator = FRotator(Pitch, Yaw, Roll);

	float X = FMath::RandRange(-0.16f, 0.16f) * RecoilMultiplier;
	float Y = FMath::RandRange(-1.1f, -2.1f) * RecoilMultiplier;

	FVector RecoilLocation = FVector(X, Y, 0);

	return FTransform(FQuat(RecoilRotator), RecoilLocation);
}

void AGun::InterpolateWeaponRecoil(float DeltaTime)
{
	FollowerRecoil.SetLocation(FMath::VInterpTo(FollowerRecoil.GetLocation(), LeaderRecoil.GetLocation(), DeltaTime, 25.f));
	FollowerRecoil.SetRotation(FMath::RInterpTo(FollowerRecoil.GetRotation().Rotator(), LeaderRecoil.GetRotation().Rotator(), DeltaTime, 25.f).Quaternion());

	LeaderRecoil.SetLocation(FMath::VInterpTo(LeaderRecoil.GetLocation(), FVector::ZeroVector, DeltaTime, 15.f));
	LeaderRecoil.SetRotation(FMath::RInterpTo(LeaderRecoil.GetRotation().Rotator(), FRotator::ZeroRotator, DeltaTime, 15.f).Quaternion());
}

void AGun::AddControllerRecoil()
{
	float RecoilPitch = FMath::RandRange(RecoilMinPitch, RecoilMaxPitch);
	float RecoilYaw = FMath::RandRange(RecoilMinYaw, RecoilMaxYaw);
	
	if (bIsFirstShot)
	{
		RecoilPitch = -RecoilMaxPitch * 1.3f;
		bIsFirstShot = false;
	}
	
	float CrosshairSpreadRatio = 1.f;
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (PlayerController)
	{
		AShooterHUD* ShooterHUD = Cast<AShooterHUD>(PlayerController->GetHUD());
		if (ShooterHUD)
		{
			CrosshairSpreadRatio = ShooterHUD->GetCrosshairSpread() / ShooterHUD->GetBaseCrosshairSpread();
		}
	}

	APawn* Character = Cast<APawn>(GetOwner()) != nullptr ? Cast<APawn>(GetOwner()) : nullptr;
	if (Character)
	{
		Character->AddControllerPitchInput(RecoilPitch * CrosshairSpreadRatio);
		Character->AddControllerYawInput(RecoilYaw * CrosshairSpreadRatio);
	}
}

void AGun::PlayReloadAnim()
{
	if (WeaponReloadAnimation)
	{
		GetWeaponMesh()->PlayAnimation(WeaponReloadAnimation, false);
	}
}


