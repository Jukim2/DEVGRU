
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCausor);
	void Elim();
	
protected:
	virtual void BeginPlay() override;

private:
	// Game properties
	UPROPERTY()
	class AShooterGameMode* ShooterGameMode;
	UPROPERTY()
	class AShooterController* ShooterController;
	UPROPERTY()
	class AShooterPlayerState* ShooterPlayerState;
	UPROPERTY()
	class AShooterCharacter* OwnerShooterCharacter;
	
	// Health
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health();
	void UpdateHUDHealth();

	// Elim
	bool bElimmed = false;
	
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

public:
	void SetOwnerShooterCharacter(AShooterCharacter* Owner) { OwnerShooterCharacter = Owner; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetHealth() const { return Health; }
	bool GetbDisableGameplay() const { return bDisableGameplay; }
	void SetbDisableGameplay(bool Val) { bDisableGameplay = Val; }
	bool GetbElimmed() const { return bElimmed; }
	
};
