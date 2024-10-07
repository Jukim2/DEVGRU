#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Rifle UMETA(DisplayName = "Rifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Grenade UMETA(DisplayName = "Grenade"),
	EWT_Knife UMETA(DisplayName = "Knife"),
	EWT_MAX UMETA(DisplayName = "DefaultMax")
};