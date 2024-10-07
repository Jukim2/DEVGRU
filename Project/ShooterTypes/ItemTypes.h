#pragma once

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Key UMETA(DisplayName = "Key"),
	EIT_Shield UMETA(DisplayName = "Shield"),
	EIT_MAX UMETA(DisplayName = "DefaultMax")
};