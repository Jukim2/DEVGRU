// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API AShooterController : public APlayerController
{
	GENERATED_BODY() 

public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override; // Sync with server clock asap
	virtual void SetupInputComponent() override;
	virtual float GetServerTime();

	// HUD
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(uint32 CountdownTime);
	void SetHUDAnnouncementCountdown(uint32 CountdownTime);
	void SetHUDTeamIcon();
	void SetHUDInteract(bool bShow);
	void HideHUD();
	void ShowHUD();
	UFUNCTION(BlueprintCallable)
	void SetNightVisionUI(float Opacity);

	// MatchState
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleWaitingToStart();
	void HandleMatchHasStarted();
	void HandleCooldown();
	void HandleGameEnd();

	// Spectating
	UFUNCTION(Client, Reliable)
	void ClientBeginSpectating(AShooterCharacter* ElimmedCharacter);
	void SpectateNextPlayer();
	void SpectatePrevPlayer();
	void CycleSpectatorView(bool bNext);
	void SetSpectatingViewTarget();
	void SetSpectatingCameraTransform();
	bool IsSameTeam(AShooterCharacter* FirstCharacter, AShooterCharacter* SecondCharacter);
	UPROPERTY()
	ACameraActor* SpectatingCamera;
	
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	
	// Sync Time between Client and server
	UFUNCTION(Server, Reliable)
	void ServerRequesetServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecievedClientRequeset);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	
private:
	UPROPERTY()
	class AShooterHUD* ShooterHUD;
	UPROPERTY()
	class AShooterGameMode* ShooterGameMode;

	// Match Time
	float LevelStartingTime;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	float StateStartingTime = 0.f;
	uint32 CountdownInt = 0;

	// Spectator
	TArray<class AShooterCharacter*> AlivePlayers;
	int32 CurrentSpectatedPlayerIndex;

	// MatchState
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	// Game End Timer
	FTimerHandle GameEndTimer;
	float GameEndTime = 5.f;
	void StartGameEndTimer();
	void GameEndTimerFinished();
};
