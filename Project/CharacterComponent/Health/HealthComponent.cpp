

#include "HealthComponent.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/Controller/ShooterController.h"
#include "Project/GameMode/ShooterGameMode.h"

#include "Net/UnrealNetwork.h"

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
	DOREPLIFETIME(UHealthComponent, bDisableGameplay);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();
	if (OwnerShooterCharacter == nullptr)
	{
		OwnerShooterCharacter = Cast<AShooterCharacter>(GetOwner());
	}
	if (OwnerShooterCharacter && OwnerShooterCharacter->HasAuthority())
	{
		OwnerShooterCharacter->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::ReceiveDamage);
	}
}

void UHealthComponent::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
                                     AController* InstigatorController, AActor* DamageCausor)
{
	ShooterGameMode =  ShooterGameMode == nullptr ? GetWorld()->GetAuthGameMode<AShooterGameMode>() : ShooterGameMode;
	if (bElimmed || ShooterGameMode == nullptr) return;
	Damage = ShooterGameMode->CalculateDamage(InstigatorController, OwnerShooterCharacter->Controller, Damage);
	
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	if (Health == 0.f)
	{
		ShooterController = ShooterController == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : ShooterController;
		AShooterController* AttackerController = Cast<AShooterController>(InstigatorController);
		ShooterGameMode->PlayerEliminated(OwnerShooterCharacter, ShooterController, AttackerController);
		OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_Eliminated);
		OwnerShooterCharacter->HandleCharacterState();
	}
}

void UHealthComponent::OnRep_Health()
{
	UpdateHUDHealth();
}

void UHealthComponent::UpdateHUDHealth()
{
	ShooterController = ShooterController == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : ShooterController;
	if (ShooterController)
	{
		ShooterController->SetHUDHealth(Health, MaxHealth);
	}
}

void UHealthComponent::Elim()
{
	bElimmed = true;
	bDisableGameplay = true;
}
