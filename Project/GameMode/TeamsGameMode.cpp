// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Project/GameState/ShooterGameState.h"
#include "Project/PlayerState/ShooterPlayerState.h"
#include "Project/Controller/ShooterController.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Project/Character/ShooterCharacter.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState == MatchState::WaitingToStart)
	{
		GameOver = false;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		if (CountdownTime <= 0.f && !GameOver)
		{
			if (HandleRoundOver()) return;
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		if (CountdownTime <= 0.f)
		{
			RespawnAllPlayer();
			AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();
			if (ShooterGameState)
			{
				ShooterGameState->InitVariables();
			}
			SetMatchState(MatchState::WaitingToStart);
		}
	}
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AShooterGameState* BGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		AShooterPlayerState* BPState = NewPlayer->GetPlayerState<AShooterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AShooterGameState* BGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	AShooterPlayerState* BPState = Exiting->GetPlayerState<AShooterPlayerState>();

	if (BGameState && BPState)
	{
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}
}

void ATeamsGameMode::PlayerEliminated(AShooterCharacter* ElimmedCharacter, AShooterController* VictimController,
                                      AShooterController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	AShooterGameState* BGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->AddBlueTeamKillCount();
		}
		else if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->AddRedTeamKillCount();
		}
		if (BGameState->RedTeamKillCount >= BGameState->BlueTeam.Num())
		{
			BGameState->SetWinnerTeam(ETeam::ET_RedTeam);
			BGameState->RedTeamVictoryCnt++;
			if (BGameState->RedTeamVictoryCnt >= 2)
			{
				SetMatchState(MatchState::GameEnd);
			}
			else
				StartRestartTimer(); 
		}
		else if (BGameState->BlueTeamKillCount >= BGameState->RedTeam.Num())
		{
			BGameState->SetWinnerTeam(ETeam::ET_BlueTeam);
			BGameState->BlueTeamVictoryCnt++;
			if (BGameState->BlueTeamVictoryCnt >= 2)
			{
				SetMatchState(MatchState::GameEnd);
			}
			else
				StartRestartTimer();
		}
	}
	
	VictimController->ClientBeginSpectating(ElimmedCharacter);
}



bool ATeamsGameMode::HandleRoundOver()
{
	AShooterGameState* BGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState->WinnerTeam == ETeam::ET_NoTeam)
	{
		if (BGameState->RedTeamKillCount > BGameState->BlueTeamKillCount)
		{
			BGameState->RedTeamVictoryCnt++;
			BGameState->SetWinnerTeam(ETeam::ET_RedTeam);
		}
		else if (BGameState->RedTeamKillCount < BGameState->BlueTeamKillCount)
		{
			BGameState->BlueTeamVictoryCnt++;
			BGameState->SetWinnerTeam(ETeam::ET_BlueTeam);
		}
		else
		{
			BGameState->RedTeamVictoryCnt++;
			BGameState->BlueTeamVictoryCnt++;
		}
		if (BGameState->RedTeamVictoryCnt >= 2 || BGameState->BlueTeamVictoryCnt >= 2)
		{
			StartGameEndTimer();
			GameOver = true;
			return true;
		}
	}
	return false;
}

void ATeamsGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
}

void ATeamsGameMode::StartRestartTimer()
{
	GetWorldTimerManager().SetTimer(
		RestartTimer,
		this,
		&ATeamsGameMode::RestartTimerFinished,
		RestartTime
		);
}

void ATeamsGameMode::RestartTimerFinished()
{
	SetMatchState(MatchState::Cooldown);
}

void ATeamsGameMode::StartGameEndTimer()
{
	GetWorldTimerManager().SetTimer(
		GameEndTimer,
		this,
		&ATeamsGameMode::GameEndTimerFinished,
		GameEndTime
		);
}

void ATeamsGameMode::GameEndTimerFinished()
{
	SetMatchState(MatchState::GameEnd);
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AShooterGameState* BGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		// 맵에 있는 모든 PlayerStart를 가져옴
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

		// 팀별로 사용할 PlayerStart 목록을 구분
		TArray<APlayerStart*> RedTeamStarts;
		TArray<APlayerStart*> BlueTeamStarts;

		for (AActor* PlayerStartActor : PlayerStarts)
		{
			APlayerStart* PlayerStart = Cast<APlayerStart>(PlayerStartActor);
			if (PlayerStart)
			{
				if (PlayerStart->ActorHasTag(TEXT("RedTeam")))
				{
					RedTeamStarts.Add(PlayerStart);
				}
				else if (PlayerStart->ActorHasTag(TEXT("BlueTeam")))
				{
					BlueTeamStarts.Add(PlayerStart);
				}
			}
		}
		
		for (auto PState : BGameState->PlayerArray)
		{
			AShooterPlayerState* BPState = Cast<AShooterPlayerState>(PState.Get());
			AController* PlayerController = BPState ? BPState->GetOwner<AController>() : nullptr;
			APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;

			if (BPState && PlayerPawn)
			{
				if (BPState->GetTeam() == ETeam::ET_NoTeam)
				{
					// 팀 배정
					if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
					{
						BGameState->RedTeam.AddUnique(BPState);
						BPState->SetTeam(ETeam::ET_RedTeam);
					}
					else
					{
						BGameState->BlueTeam.AddUnique(BPState);
						BPState->SetTeam(ETeam::ET_BlueTeam);
					}
				}

				// 팀에 따라 플레이어 위치를 설정
				APlayerStart* ChosenPlayerStart = nullptr;

				if (BPState->GetTeam() == ETeam::ET_RedTeam && RedTeamStarts.Num() > 0)
				{
					ChosenPlayerStart = RedTeamStarts[FMath::RandRange(0, RedTeamStarts.Num() - 1)];
				}
				else if (BPState->GetTeam() == ETeam::ET_BlueTeam && BlueTeamStarts.Num() > 0)
				{
					ChosenPlayerStart = BlueTeamStarts[FMath::RandRange(0, BlueTeamStarts.Num() - 1)];
				}

				// 플레이어를 PlayerStart 위치로 이동
				if (ChosenPlayerStart)
				{
					PlayerPawn->SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
				}
			}
		}
	}
}


// Calc Damgae
float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AShooterPlayerState* AttackerPState = Attacker->GetPlayerState<AShooterPlayerState>();
	AShooterPlayerState* VictimPState = Victim->GetPlayerState<AShooterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (AttackerPState == VictimPState) return BaseDamage;
	if (AttackerPState->GetTeam() == VictimPState->GetTeam()) return 0.f;
	return BaseDamage;
}
