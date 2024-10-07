#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_SwapUnequipping UMETA(DisplayName = "Swap Unequipping"),
	ECS_SwapEquipping UMETA(DisplayName = "Swap Equipping"),
	ECS_AimingGrenade UMETA(DisplayName = "Aiming Grenade"),
	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),
	ECS_Eliminated UMETA(DisplayName = "Eliminated"),
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};