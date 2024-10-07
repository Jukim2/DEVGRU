// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/Controller/ShooterController.h"
#include "Project/GameState/ShooterGameState.h"
#include "Project/PlayerState/ShooterPlayerState.h"
#include "Project/Weapon/Weapon.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
	const FName GameEnd = FName("GameEnd");
}

AShooterGameMode::AShooterGameMode()
{
	bDelayedStart = true;
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AShooterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
;	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + StateStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = MatchTime - GetWorld()->GetTimeSeconds() + StateStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime - GetWorld()->GetTimeSeconds() + StateStartingTime;
	}
}

void AShooterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	
	if (MatchState == MatchState::WaitingToStart)
		DestroyAllWeapons();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterController* ShooterController = Cast<AShooterController>(*It);
		if (ShooterController)
		{
			ShooterController->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

void AShooterGameMode::SetMatchState(FName NewState)
{
	Super::SetMatchState(NewState);

	StateStartingTime = GetWorld()->GetTimeSeconds();
}

void AShooterGameMode::PlayerEliminated(AShooterCharacter* ElimmedCharacter, AShooterController* VictimController,
                                        AShooterController* AttackerController)
{
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AShooterPlayerState* VictimPlayerState = VictimController ? Cast<AShooterPlayerState>(VictimController->PlayerState) : nullptr;

	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && ShooterGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
}

void AShooterGameMode::RespawnAllPlayer()
{
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (Controller)
		{
			ACharacter* Character = Cast<ACharacter>(Controller->GetPawn());
			if (Character)
			{
				RequestRespawn(Character, Controller);
			}
		}
	}
}

void AShooterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void AShooterGameMode::DestroyAllWeapons()
{
	TArray<AActor*> WeaponActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeapon::StaticClass(), WeaponActors);
	
	for (AActor* WeaponActor : WeaponActors)
	{
		if (WeaponActor)
		{
			WeaponActor->Destroy();
		}
	}
}

float AShooterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

