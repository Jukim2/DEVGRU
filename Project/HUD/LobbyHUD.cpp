// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyHUD.h"
#include "Components/TextBlock.h"

void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	AddInfoWidget();
}

void ALobbyHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (LocalPlayerCnt > 0)
	{
		UpdatePlayerCntText(LocalPlayerCnt);
	}
}

void ALobbyHUD::AddInfoWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && LobbyInfoWidgetClass)
	{
		LobbyInfoWidget = CreateWidget<ULobbyInfoWidget>(PlayerController, LobbyInfoWidgetClass);
		LobbyInfoWidget->AddToViewport();
	}
}

void ALobbyHUD::UpdatePlayerCntText(uint8 PlayerCnt)
{
	if (!LobbyInfoWidget)
	{
		LocalPlayerCnt = PlayerCnt;
	}
	
	if (LobbyInfoWidget && LobbyInfoWidget->PlayerCntText)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), PlayerCnt);
		LobbyInfoWidget->PlayerCntText->SetText(FText::FromString(ScoreText));
		LocalPlayerCnt = 0;
	}
}
