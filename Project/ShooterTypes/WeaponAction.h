#pragma once

UENUM(BlueprintType)
enum class EWeaponAction : uint8
{
	EWA_Equip UMETA(DisplayName = "Equip"),
	EWA_AimTriggered UMETA(DisplayName = "AimTriggered"),
	EWA_AimCompleted UMETA(DisplayName = "AimCompleted"),
	EWA_FireStarted UMETA(DisplayName = "FireStarted"),
	EWA_FireCompleted UMETA(DisplayName = "FireCompleted"),
	EWA_Reload UMETA(DisplayName = "Reload"),
	EWA_SwapToRifle UMETA(DisplayName = "SwapToRifle"),
	EWA_SwapToPistol UMETA(DisplayName = "SwapToPistol"),
	EWA_SwapToGrenade UMETA(DisplayName = "SwapToGrenade"),
	EWA_MAX UMETA(DisplayName = "DefaultMAX")
};