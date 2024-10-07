// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	class UImage* RedTeamIcon1;

	UPROPERTY(meta = (BindWidget))
	UImage* RedTeamIcon2;

	UPROPERTY(meta = (BindWidget))
	UImage* RedTeamIcon3;

	UPROPERTY(meta = (BindWidget))
	UImage* BlueTeamIcon1;

	UPROPERTY(meta = (BindWidget))
	UImage* BlueTeamIcon2;

	UPROPERTY(meta = (BindWidget))
	UImage* BlueTeamIcon3;
	
	UPROPERTY(meta = (BindWidget))
	UImage* RifleUI;

	UPROPERTY(meta = (BindWidget))
	UImage* PistolUI;

	UPROPERTY(meta = (BindWidget))
	UImage* GrenadeUI;

	UPROPERTY(meta = (BindWidget))
	UImage* NightVisionUI;
	
	TArray<UImage*> WeaponUIArray;
};
