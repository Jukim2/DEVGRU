// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

namespace MatchState
{
	extern PROJECT_API const FName Cooldown; // Display winner and cooldown timer
	extern PROJECT_API const FName GameEnd;
}

/**
 * 
 */
UCLASS()
class PROJECT_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AShooterGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	virtual void PlayerEliminated(class AShooterCharacter* ElimmedCharacter, class AShooterController* VictimController, AShooterController* AttackerController);

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	void RespawnAllPlayer();
	void DestroyAllWeapons();

	UPROPERTY(EditDefaultsOnly)
	float CountdownTime = 0.f;
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 5.f;
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	float LevelStartingTime = 0.f;
	float StateStartingTime = 0.f;
	bool bTeamsMatch = false;
	
public:
	virtual void SetMatchState(FName NewState) override;
	float GetCountdownTime() const { return CountdownTime; }
	float GetWarmpupTime() const { return WarmupTime; }
	float GetMatchTime() const { return MatchTime; }
	float GetLevelStartingTime() const { return LevelStartingTime; }
	float GetStateStartingTime() const { return StateStartingTime; }
	float GetCooldownTime() const { return CooldownTime; }
};
