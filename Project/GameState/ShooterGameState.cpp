// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameState.h"
#include "Project/PlayerState/ShooterPlayerState.h"

#include "Net/UnrealNetwork.h"

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AShooterGameState, RedTeamKillCount);
	DOREPLIFETIME(AShooterGameState, BlueTeamKillCount);
	DOREPLIFETIME(AShooterGameState, RedTeamVictoryCnt);
	DOREPLIFETIME(AShooterGameState, BlueTeamVictoryCnt);
	DOREPLIFETIME(AShooterGameState, WinnerTeam);
}

void AShooterGameState::AddRedTeamKillCount()
{
	++RedTeamKillCount;
}

void AShooterGameState::AddBlueTeamKillCount()
{
	++BlueTeamKillCount;
}

void AShooterGameState::InitVariables()
{
	BlueTeamKillCount = 0;
	RedTeamKillCount = 0;
	WinnerTeam = ETeam::ET_NoTeam;
}

void AShooterGameState::SetWinnerTeam(ETeam Team)
{
	WinnerTeam = Team;
}
