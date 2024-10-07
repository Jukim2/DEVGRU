
// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/HUD/LobbyHUD.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)	
	{
		uint8 PlayerCnt = GameState.Get()->PlayerArray.Num();
		UpdateAllPlayerHUD(PlayerCnt, nullptr);
		Cast<AShooterCharacter>(NewPlayer->GetPawn())->SetbDisableGameplay(false);
		
		if (PlayerCnt == 4)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = false;
				World->ServerTravel(FString("/Game/Maps/MilitaryCamp?listen"));
			}
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GameState)
	{
		int32 PlayerCnt = GameState.Get()->PlayerArray.Num();

		UpdateAllPlayerHUD(PlayerCnt, Exiting);
	}
}

void ALobbyGameMode::UpdateAllPlayerHUD(uint8 PlayerCnt, AController* Exiting)
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PlayerController = It->Get();
			if (PlayerController && PlayerController->GetHUD() && PlayerController != Exiting)
			{
				Cast<ALobbyHUD>(PlayerController->GetHUD())->UpdatePlayerCntText(PlayerCnt);
			}
		}
	}
}

void ALobbyGameMode::GoIn()
{
	UWorld* World = GetWorld();
	if (World)
	{
		bUseSeamlessTravel = false;
		World->ServerTravel(FString("/Game/Maps/MilitaryCamp?listen"));
	}
}
