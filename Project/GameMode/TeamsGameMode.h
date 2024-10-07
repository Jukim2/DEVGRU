// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API ATeamsGameMode : public AShooterGameMode
{
	GENERATED_BODY()

public:
	ATeamsGameMode();
	bool HandleRoundOver();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(AShooterCharacter* ElimmedCharacter, AShooterController* VictimController, AShooterController* AttackerController) override;
	virtual void OnMatchStateSet() override;
	
protected:
	virtual void HandleMatchHasStarted() override;

private:
	FTimerHandle RestartTimer;
	float RestartTime = 2.f;
	void StartRestartTimer();
	void RestartTimerFinished();

	FTimerHandle GameEndTimer;
	float GameEndTime = 2.f;
	void StartGameEndTimer();
	void GameEndTimerFinished();

	bool GameOver = false;
	class UShooterGameInstance* ShooterGameInstance;
};
