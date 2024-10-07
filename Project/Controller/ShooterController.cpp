// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterController.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Project/HUD/ShooterHUD.h"
#include "Project/HUD/CharacterOverlay.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/GameMode/ShooterGameMode.h"
#include "Project/HUD/Announcement.h"
#include "Project/GameState/ShooterGameState.h"
#include "Project/PlayerState/ShooterPlayerState.h"
#include "Project/CharacterComponent/Weapon/WeaponActionComponent.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void AShooterController::BeginPlay()
{
	Super::BeginPlay();
	
	ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	SetHUDTeamIcon();
	if (SpectatingCamera)
	{
		AShooterCharacter* SpectatedPlayer = AlivePlayers[CurrentSpectatedPlayerIndex];
		if (SpectatedPlayer)
		{
			USceneComponent* SpectatorComponent = SpectatedPlayer->GetSpectatorComponent();
			if (SpectatorComponent && SpectatingCamera)
			{
				SpectatingCamera->SetActorLocation(SpectatorComponent->GetComponentLocation());
				SpectatingCamera->SetActorRotation(SpectatorComponent->GetComponentRotation());
			}
		}
	}
}

void AShooterController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterController, MatchState);
}

void AShooterController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequesetServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AShooterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn);
	if (ShooterCharacter)
	{
		SetHUDHealth(ShooterCharacter->GetHealth(), ShooterCharacter->GetMaxHealth());
	}
}

void AShooterController::SetHUDHealth(float Health, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	
	bool bHUDValid = ShooterHUD
	&& ShooterHUD->CharacterOverlay
	&& ShooterHUD->CharacterOverlay->HealthBar
	&& ShooterHUD->CharacterOverlay->HealthText;
	
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		ShooterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		ShooterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AShooterController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &AShooterController::SpectateNextPlayer);
	InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &AShooterController::SpectatePrevPlayer);
}

void AShooterController::ClientBeginSpectating_Implementation(AShooterCharacter* ElimmedCharacter)
{
	HideHUD();
	AlivePlayers.Empty();
	TArray<AActor*> PlayerList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterCharacter::StaticClass(), PlayerList);

	for (AActor* Actor : PlayerList)
	{;
		AShooterCharacter* ShooterPlayer = Cast<AShooterCharacter>(Actor);
		if (ShooterPlayer && !ShooterPlayer->IsDead()
			&& IsSameTeam(ElimmedCharacter, ShooterPlayer)) 
		{
			AlivePlayers.Add(ShooterPlayer);
		}
	}

	if (AlivePlayers.Num() > 0)
		SetSpectatingViewTarget();
}

void AShooterController::SetSpectatingViewTarget()
{
	AShooterCharacter* SpectatedPlayer = AlivePlayers[CurrentSpectatedPlayerIndex];
	if (SpectatedPlayer)
	{
		USceneComponent* SpectatorComponent = SpectatedPlayer->GetSpectatorComponent();
		if (SpectatorComponent)
		{
			SpectatingCamera = GetWorld()->SpawnActor<ACameraActor>(SpectatorComponent->GetComponentLocation(), SpectatorComponent->GetComponentRotation());
			if (SpectatingCamera && SpectatingCamera->GetCameraComponent())
			{
				SpectatingCamera->GetCameraComponent()->SetFieldOfView(90.0f);
				SpectatingCamera->GetCameraComponent()->bConstrainAspectRatio = false;
			}
			SetViewTargetWithBlend(SpectatingCamera, 0.1f);
		}
	}
}

void AShooterController::SpectateNextPlayer()
{
	CycleSpectatorView(true);
}

void AShooterController::SpectatePrevPlayer()
{
	CycleSpectatorView(false);
}

void AShooterController::CycleSpectatorView(bool bNext)
{
	if (AlivePlayers.Num() <= 0) return;
		
	do
	{
		if (bNext)
			CurrentSpectatedPlayerIndex = (CurrentSpectatedPlayerIndex + 1) % AlivePlayers.Num();
		else
			CurrentSpectatedPlayerIndex = (CurrentSpectatedPlayerIndex - 1 + AlivePlayers.Num()) % AlivePlayers.Num();
	} while (AlivePlayers[CurrentSpectatedPlayerIndex]->IsDead());

	SetSpectatingCameraTransform();
}

void AShooterController::SetSpectatingCameraTransform()
{
	AShooterCharacter* SpectatedPlayer = AlivePlayers[CurrentSpectatedPlayerIndex];
	if (SpectatedPlayer)
	{
		USceneComponent* SpectatorComponent = SpectatedPlayer->GetSpectatorComponent();
		if (SpectatorComponent && SpectatingCamera)
		{
			SpectatingCamera->SetActorLocation(SpectatorComponent->GetComponentLocation());
			SpectatingCamera->SetActorRotation(SpectatorComponent->GetComponentRotation());
		}
	}
}

bool AShooterController::IsSameTeam(AShooterCharacter* FirstCharacter, AShooterCharacter* SecondCharacter)
{
	AShooterPlayerState* TmpPlayerState;
	ETeam FirstTeam = ETeam::ET_NoTeam, SecondTeam = ETeam::ET_NoTeam;
	if (FirstCharacter)
	{
		TmpPlayerState = Cast<AShooterPlayerState>(FirstCharacter->GetPlayerState());
		if (TmpPlayerState)
			FirstTeam = TmpPlayerState->GetTeam();
	}
	if (SecondCharacter)
	{
		TmpPlayerState = Cast<AShooterPlayerState>(SecondCharacter->GetPlayerState());
		if (TmpPlayerState)
			SecondTeam = TmpPlayerState->GetTeam();
	}
	return (FirstTeam == SecondTeam);
}


// HUD Ammo
void AShooterController::SetHUDWeaponAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD
	&& ShooterHUD->CharacterOverlay
	&& ShooterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AShooterController::SetHUDCarriedAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD
	&& ShooterHUD->CharacterOverlay
	&& ShooterHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AShooterController::SetHUDMatchCountdown(uint32 CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD
	&& ShooterHUD->CharacterOverlay
	&& ShooterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			ShooterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::Clamp(CountdownTime / 60, 0, 100) % 100;
		int32 Seconds = FMath::Clamp(CountdownTime - Minutes * 60, 0, 60) % 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AShooterController::SetHUDAnnouncementCountdown(uint32 CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD
	&& ShooterHUD->Announcement
	&& ShooterHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			ShooterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::Clamp(CountdownTime / 60, 0, 100) % 100;
		int32 Seconds = FMath::Clamp(CountdownTime - Minutes * 60, 0, 60) % 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AShooterController::SetHUDTeamIcon()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD
	&& ShooterHUD->CharacterOverlay;

	AShooterGameState* BGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (bHUDValid && BGameState)
	{
		switch (BGameState->RedTeamVictoryCnt)
		{
		case 3:
			ShooterHUD->CharacterOverlay->RedTeamIcon3->SetVisibility(ESlateVisibility::Visible);
		case 2:
			ShooterHUD->CharacterOverlay->RedTeamIcon2->SetVisibility(ESlateVisibility::Visible);
		case 1:
			ShooterHUD->CharacterOverlay->RedTeamIcon1->SetVisibility(ESlateVisibility::Visible);
			break;
		default:
			break;
		}
		switch (BGameState->BlueTeamVictoryCnt)
		{
		case 3:
			ShooterHUD->CharacterOverlay->BlueTeamIcon3->SetVisibility(ESlateVisibility::Visible);
		case 2:
			ShooterHUD->CharacterOverlay->BlueTeamIcon2->SetVisibility(ESlateVisibility::Visible);
		case 1:
			ShooterHUD->CharacterOverlay->BlueTeamIcon1->SetVisibility(ESlateVisibility::Visible);
			break;
		default:
			break;
		}
	}
}

void AShooterController::SetHUDInteract(bool bShow)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD
	&& ShooterHUD->InteractWidget;

	if (bHUDValid)
	{
		if (bShow && ShooterHUD->GetbShouldDrawHUD())
			ShooterHUD->InteractWidget->SetVisibility(ESlateVisibility::Visible);
		else
			ShooterHUD->InteractWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AShooterController::SetHUDTime()
{
	float TimeLeft = 0;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + StateStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = MatchTime - GetServerTime() + StateStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime - GetServerTime() + StateStartingTime;
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		ShooterGameMode = ShooterGameMode == nullptr ? Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this)) : ShooterGameMode;
		if (ShooterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(ShooterGameMode->GetCountdownTime());
		}
	}
	
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(SecondsLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(SecondsLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void AShooterController::HideHUD()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	
	if (ShooterHUD && ShooterHUD->CharacterOverlay)
	{
		ShooterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Hidden);
		ShooterHUD->InteractWidget->SetVisibility(ESlateVisibility::Hidden);
		ShooterHUD->SetbShouldDrawHUD(false);
	}
}

void AShooterController::ShowHUD()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	
	if (ShooterHUD && ShooterHUD->CharacterOverlay)
	{
		ShooterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Visible);
		ShooterHUD->InteractWidget->SetVisibility(ESlateVisibility::Visible);
		ShooterHUD->SetbShouldDrawHUD(true);
	}
}

void AShooterController::SetNightVisionUI(float Opacity)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD
	&& ShooterHUD->CharacterOverlay;
	
	if (bHUDValid)
	{
		FLinearColor NewColor = ShooterHUD->CharacterOverlay->NightVisionUI->GetColorAndOpacity();
		
		NewColor.A = Opacity;
		
		ShooterHUD->CharacterOverlay->NightVisionUI->SetColorAndOpacity(NewColor);
	}
}

// Server Time
void AShooterController::ServerRequesetServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AShooterController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerRecievedClientRequeset)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerRecievedClientRequeset + 0.5f * RoundTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AShooterController::GetServerTime()
{
	if (HasAuthority())
		return GetWorld()->GetTimeSeconds();
	else
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequesetServerTime(GetWorld()->GetTimeSeconds());
	}
}

// Match State
void AShooterController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::WaitingToStart)
	{
		HandleWaitingToStart();
	}
	else if (MatchState == MatchState::InProgress)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
		ShooterCharacter->SetbDisableGameplay(false);
		ShooterCharacter->SetWeapons();
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
	else if (MatchState == MatchState::GameEnd)
	{
		HandleGameEnd();
	}
}

void AShooterController::OnRep_MatchState()
{
	StateStartingTime = GetServerTime();
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleWaitingToStart();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
	else if (MatchState == MatchState::GameEnd)
	{
		HandleGameEnd();
	}
}

void AShooterController::HandleWaitingToStart()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		if (ShooterHUD->CharacterOverlay)
			ShooterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Hidden);
		bool bHUDValid = ShooterHUD->Announcement
		&& ShooterHUD->Announcement->AnnouncementText
		&& ShooterHUD->Announcement->InfoText; 
		if (bHUDValid)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("Round Starts In:");
			ShooterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			FString InfoTextString("Ready for the Operation");
			ShooterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
		}
	}
}


void AShooterController::HandleMatchHasStarted()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		ShooterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Visible);
		if (ShooterHUD->Announcement)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AShooterController::HandleCooldown()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		ShooterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Hidden);
		bool bHUDValid = ShooterHUD->Announcement
		&& ShooterHUD->Announcement->AnnouncementText
		&& ShooterHUD->Announcement->InfoText; 
		if (bHUDValid)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("Round Over:");
			ShooterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
			AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
			if (ShooterGameState && ShooterPlayerState)
			{
				FString InfoTextString;
				// 2. 마지막 문구
				if (ShooterGameState->WinnerTeam == ETeam::ET_BlueTeam)
				{
					InfoTextString = FString("Blue Team Win!");
				}
				else if (ShooterGameState->WinnerTeam == ETeam::ET_RedTeam)
				{
					InfoTextString = FString("Red Team Win!");
				}
				else
				{
					InfoTextString = FString("Draw");					
				}
				ShooterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
	
	if (ShooterCharacter && ShooterCharacter->GetWeaponActionComponent())
	{
		ShooterCharacter->SetbDisableGameplay(true);
		ShooterCharacter->GetWeaponActionComponent()->AttackButtonReleased();
	}
}

void AShooterController::HandleGameEnd()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		ShooterHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Hidden);
		bool bHUDValid = ShooterHUD->Announcement
		&& ShooterHUD->Announcement->AnnouncementText
		&& ShooterHUD->Announcement->InfoText; 
		if (bHUDValid)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("Game End!");
			ShooterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			FString WarmupTimeText("Well done");
			ShooterHUD->Announcement->WarmupTime->SetText(FText::FromString(WarmupTimeText));
			AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
			AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
			if (ShooterGameState && ShooterPlayerState)
			{
				FString InfoTextString;
				if (ShooterGameState->BlueTeamVictoryCnt > ShooterGameState->RedTeamVictoryCnt)
					InfoTextString = FString("Blue Team Win!");
				else if (ShooterGameState->BlueTeamVictoryCnt < ShooterGameState->RedTeamVictoryCnt)
					InfoTextString = FString("Red Team Win!");
				else
					InfoTextString = FString("Draw");
				ShooterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	StartGameEndTimer();
}

// Game End
void AShooterController::StartGameEndTimer()
{
	GetWorldTimerManager().SetTimer(
		GameEndTimer,
		this,
		&AShooterController::GameEndTimerFinished,
		GameEndTime
		);
}

void AShooterController::GameEndTimerFinished()
{
	UGameInstance* GameInstance = GetGameInstance();
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>(); 
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}
