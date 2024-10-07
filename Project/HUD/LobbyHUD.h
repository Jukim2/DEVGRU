// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyInfoWidget.h"
#include "LobbyHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category="Info")
	TSubclassOf<class UUserWidget> LobbyInfoWidgetClass;

	UPROPERTY()
	ULobbyInfoWidget* LobbyInfoWidget;

	void UpdatePlayerCntText(uint8 PlayerCnt);

private:
	void AddInfoWidget();
	uint8 LocalPlayerCnt = 0;
	
};
