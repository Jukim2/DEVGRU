#pragma once

#include "ShooterMovementComponent.h"
#include "Project/Controller/ShooterController.h"
#include "Project/Character/ShooterCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

class UEnhancedInputLocalPlayerSubsystem;

UShooterMovementComponent::UShooterMovementComponent()
{
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Move
	(TEXT("/Game/Character/Input/Actions/IA_Move"));
	if (IA_Move.Succeeded())
	{
		IAMove = IA_Move.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Look
	(TEXT("/Game/Character/Input/Actions/IA_Look"));
	if (IA_Look.Succeeded())
	{
		IALook = IA_Look.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Crouch
	(TEXT("/Game/Character/Input/Actions/IA_Crouch"));
	if (IA_Crouch.Succeeded())
	{
		IACrouch = IA_Crouch.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Jump
	(TEXT("/Game/Character/Input/Actions/IA_Jump"));
	if (IA_Jump.Succeeded())
	{
		IAJump = IA_Jump.Object;
	}
	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Sprint
	(TEXT("/Game/Character/Input/Actions/IA_Sprint"));
	if (IA_Sprint.Succeeded())
	{
		IASprint = IA_Sprint.Object;
	}
}

void UShooterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UShooterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShooterMovementComponent, bIsRun);
}

void UShooterMovementComponent::SetupInputBindings(UInputComponent* InputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(IAMove, ETriggerEvent::Triggered, this, &UShooterMovementComponent::Move);
		EnhancedInputComponent->BindAction(IAMove, ETriggerEvent::Completed, this, &UShooterMovementComponent::MoveEnd);
		EnhancedInputComponent->BindAction(IALook, ETriggerEvent::Triggered, this, &UShooterMovementComponent::Look);
		EnhancedInputComponent->BindAction(IALook, ETriggerEvent::Completed, this, &UShooterMovementComponent::LookEnd);
		EnhancedInputComponent->BindAction(IACrouch, ETriggerEvent::Started, this, &UShooterMovementComponent::Crouch);
		EnhancedInputComponent->BindAction(IAJump, ETriggerEvent::Started, this, &UShooterMovementComponent::Jump);
		EnhancedInputComponent->BindAction(IASprint, ETriggerEvent::Started, this, &UShooterMovementComponent::RunStart);
		EnhancedInputComponent->BindAction(IASprint, ETriggerEvent::Completed, this, &UShooterMovementComponent::RunEnd);
	}
}

void UShooterMovementComponent::Move(const FInputActionValue& Value)
{
	if (OwnerShooterCharacter->GetbDisableGameplay()) return;
	FVector2D MovementVector = Value.Get<FVector2D>();

	float ForwardValue = MovementVector.Y;
	float RightValue = MovementVector.X;
	
	ShooterController = ShooterController == nullptr ? OwnerShooterCharacter->GetShooterController() : ShooterController;
	if (ShooterController != nullptr && ForwardValue)
	{
		const FRotator YawRotation(0.f, ShooterController->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		OwnerShooterCharacter->AddMovementInput(Direction, ForwardValue);

	}
	if (ShooterController != nullptr && RightValue)
	{
		const FRotator YawRotation(0.f, ShooterController->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		OwnerShooterCharacter->AddMovementInput(Direction, RightValue);
		MoveX = RightValue;
	}
}

void UShooterMovementComponent::MoveEnd(const FInputActionValue& Value)
{
	MoveX = 0;
}


void UShooterMovementComponent::Look(const FInputActionValue& Value)
{
	if (OwnerShooterCharacter->GetbDisableGameplay()) return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	float YawValue = MovementVector.X;
	float PitchValue = MovementVector.Y;

	ShooterController = ShooterController == nullptr ? OwnerShooterCharacter->GetShooterController() : ShooterController;
	if (ShooterController != nullptr && YawValue)
	{
		OwnerShooterCharacter->AddControllerYawInput(YawValue);
		MouseX = YawValue;
	}
	if (ShooterController != nullptr && PitchValue)
	{
		OwnerShooterCharacter->AddControllerPitchInput(PitchValue);
		MouseY = PitchValue;
	}
}

void UShooterMovementComponent::LookEnd(const FInputActionValue& Value)
{
	MouseX = 0;
	MouseY = 0;
}

void UShooterMovementComponent::Crouch(const FInputActionValue& Value)
{
	if (!CanPerformAction(ECharacterAction::ECA_Crouch)) return;
	if (bIsRun) ServerRunEnd();
	if (!OwnerShooterCharacter->bIsCrouched)
		OwnerShooterCharacter->Crouch();
	else
		OwnerShooterCharacter->UnCrouch();
}

void UShooterMovementComponent::Jump(const FInputActionValue& Value)
{
	if (!CanPerformAction(ECharacterAction::ECA_Jump)) return;
	if (OwnerShooterCharacter->bIsCrouched)
		OwnerShooterCharacter->UnCrouch();
	else
		OwnerShooterCharacter->Jump();
}

void UShooterMovementComponent::RunStart(const FInputActionValue& Value)
{
	if (!CanPerformAction(ECharacterAction::ECA_Run)) return;
	ServerRunStart();
}

void UShooterMovementComponent::ServerRunStart_Implementation()
{
	OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = 400.f;
	bIsRun = true;
}

void UShooterMovementComponent::RunEnd(const FInputActionValue& Value)
{
	ServerRunEnd();
}

void UShooterMovementComponent::ForceStopRun()
{
	ServerRunEnd();
}

void UShooterMovementComponent::ServerRunEnd_Implementation()
{
	OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = 200.f;
	bIsRun = false; 
}

void UShooterMovementComponent::OnRep_bIsRun()
{
	if (bIsRun)
		OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = 400.f;
	else
		OwnerShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = 200.f;
}

bool UShooterMovementComponent::CanPerformAction(ECharacterAction TargetAction)
{
	OwnerShooterCharacter = OwnerShooterCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : OwnerShooterCharacter;
	if (!OwnerShooterCharacter || OwnerShooterCharacter->GetbDisableGameplay()) return false;
	// Move : Default / Jump / Run / Crouch
	// Combat : Equip / Fire / Aim / Swap / Reload / Grenade
	
	switch (TargetAction)
	{
	case ECharacterAction::ECA_Jump:
		return !OwnerShooterCharacter->IsJumping() && OwnerShooterCharacter->IsUnoccupied();
	case ECharacterAction::ECA_Crouch:
		return !OwnerShooterCharacter->IsJumping();
		// crouch
	case ECharacterAction::ECA_Run:
		return (OwnerShooterCharacter->IsUnoccupied() && !OwnerShooterCharacter->IsFiring()
			&& !OwnerShooterCharacter->IsAiming() && !OwnerShooterCharacter->bIsCrouched);
	default:
		return true;
	}
}

