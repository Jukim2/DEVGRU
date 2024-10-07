// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"

void UCharacterOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	WeaponUIArray.Add(RifleUI);
	WeaponUIArray.Add(PistolUI);
	WeaponUIArray.Add(GrenadeUI);
}
