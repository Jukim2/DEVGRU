// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project/ShooterTypes/Team.h"
#include "Project/PlayerState/ShooterPlayerState.h"

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShooterGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API AShooterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Teams
	void AddRedTeamKillCount();
	void AddBlueTeamKillCount();

	UPROPERTY()
	TArray<AShooterPlayerState*> RedTeam;
	UPROPERTY()
	TArray<AShooterPlayerState*> BlueTeam;

	// Team Kill Count
	UPROPERTY(Replicated)
	float RedTeamKillCount = 0.f;
	UPROPERTY(Replicated)
	float BlueTeamKillCount = 0.f;

	void InitVariables();

	// Round WinnerTeam
	UPROPERTY(Replicated)
	ETeam WinnerTeam = ETeam::ET_NoTeam;

	void SetWinnerTeam(ETeam Team);

	// Round Victory Cnt
	UPROPERTY(Replicated)
	int BlueTeamVictoryCnt = 0;
	UPROPERTY(Replicated)
	int RedTeamVictoryCnt = 0;
		
private:
	float TopScore = 0.f;
};
