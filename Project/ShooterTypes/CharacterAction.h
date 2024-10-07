#pragma once

UENUM(BlueprintType)
enum class ECharacterAction : uint8
{
	ECA_Default UMETA(DisplayName = "Default"),
	ECA_Jump UMETA(DisplayName = "Jump"),
	ECA_Run UMETA(DisplayName = "Run"),
	ECA_Crouch UMETA(DisplayName = "Crouch"),

	ECA_Combat UMETA(DisplayName = "Combat"),
	ECA_Fire UMETA(DisplayName = "Fire"),
	ECA_Equip UMETA(DisplayName = "Equip"),
	ECA_Aim UMETA(DisplayName = "Aim"),
	ECA_Swap UMETA(DisplayName = "Swap"),
	ECA_Reload UMETA(DisplayName = "Reload"),
	
	ECA_MAX UMETA(DisplayName = "DefaultMAX")
};