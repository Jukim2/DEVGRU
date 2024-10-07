// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/Controller/ShooterController.h"

#include "Net/UnrealNetwork.h"

void ARangedWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARangedWeapon, Ammo);
}

// Attack
void ARangedWeapon::AttackInitiated()
{
	Super::AttackInitiated();

	SpendRound();
}

void ARangedWeapon::AttackStopped()
{
	Super::AttackStopped();
}

// Rep_Owner
void ARangedWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	if (Owner && ShooterOwnerCharacter && ShooterOwnerCharacter->GetEquippedWeapon() && ShooterOwnerCharacter->GetEquippedWeapon() == this)
		SetHUDAmmo();
}

// Ammo
void ARangedWeapon::SetHUDAmmo()
{
	Super::SetHUDAmmo();
	
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : ShooterOwnerCharacter;
	if (ShooterOwnerCharacter)
	{
		ShooterOwnerController = ShooterOwnerController == nullptr ? Cast<AShooterController>(ShooterOwnerCharacter->Controller) : ShooterOwnerController;
		if (ShooterOwnerController)
		{
			ShooterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void ARangedWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void ARangedWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void ARangedWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}


bool ARangedWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

bool ARangedWeapon::IsEmpty()
{
	return Ammo <= 0;
}
