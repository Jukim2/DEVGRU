// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "Project/ShooterTypes/CharacterAction.h"
#include "ShooterMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_API UShooterMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShooterMovementComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetupInputBindings(UInputComponent* InputComponent);
	
	UPROPERTY(EditAnywhere, Category="Move")
	float RunSpeed = 400.f;
	UPROPERTY(EditAnywhere, Category="Move")
	float WalkSpeed = 200.f;
	UPROPERTY(EditAnywhere, Category="Move")
	float AimSpeed = 125.f;
	UPROPERTY(EditAnywhere, Category="Move")
	float CrouchSpeed = 125.f;

	void ForceStopRun();

protected:
	virtual void BeginPlay() override;

private:
	void Move(const FInputActionValue& Value);
	void MoveEnd(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void LookEnd(const FInputActionValue& Value);
	void Crouch(const FInputActionValue& Value);
	void Jump(const FInputActionValue& Value);
	void RunStart(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void ServerRunStart();
	void RunEnd(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void ServerRunEnd();
	
	UPROPERTY(VisibleAnywhere, Category = Input)
	class UInputAction* IAMove;
	UPROPERTY(VisibleAnywhere, Category = Input)
	class UInputAction* IALook;
	UPROPERTY(VisibleAnywhere, Category = Input)
	class UInputAction* IACrouch;
	UPROPERTY(VisibleAnywhere, Category = Input)
	class UInputAction* IAJump;
	UPROPERTY(VisibleAnywhere, Category = Input)
	class UInputAction* IASprint;

	UPROPERTY()
	class AShooterCharacter* OwnerShooterCharacter;
	UPROPERTY()
	class AShooterController* ShooterController;

	bool CanPerformAction(ECharacterAction TargetAction);

	UPROPERTY(ReplicatedUsing = OnRep_bIsRun)
	bool bIsRun = false;
	float MoveX = 0.f;
	float MouseX = 0.f;
	float MouseY = 0.f;
	
	UFUNCTION()
	void OnRep_bIsRun();

public:
	void SetOwnerShooterController(AShooterCharacter* Owner) { OwnerShooterCharacter = Owner; }
	float GetMoveX() const { return MoveX; }
	float GetMouseX() const { return MouseX; }
	float GetMouseY() const { return MouseY; }
	bool GetbIsRun() const { return bIsRun; }
};
