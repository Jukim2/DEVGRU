
#include "ShooterCharacter.h"

#include <functional>

#include "Project/Project.h"
#include "EnhancedInputComponent.h"
#include "Project/Weapon/Weapon.h"
#include "Project/Controller/ShooterController.h"
#include "Project/PlayerState/ShooterPlayerState.h"
#include "Project/CharacterComponent/Inventory/ItemObject.h"
#include "Project/CharacterComponent/Health/HealthComponent.h"
#include "Project/CharacterComponent/Weapon/WeaponActionComponent.h"
#include "Project/CharacterComponent/Weapon/WeaponManagerComponent.h"
#include "Project/CharacterComponent/Inventory//InventoryComponent.h"
#include "Project/CharacterComponent/Movement//ShooterMovementComponent.h"
#include "Project/CharacterComponent/Animation/CharacterAnimationComponent.h"

#include "Net/UnrealNetwork.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));
	ShooterMovementComponent = CreateDefaultSubobject<UShooterMovementComponent>(TEXT("ShooterMovmentComponent"));
	WeaponManagerComponent = CreateDefaultSubobject<UWeaponManagerComponent>(TEXT("WeaponMangerComponent"));
	WeaponManagerComponent->SetIsReplicated(true);
	WeaponActionComponent = CreateDefaultSubobject<UWeaponActionComponent>(TEXT("WeaponActionComponent"));
	WeaponActionComponent->SetIsReplicated(true);
	CharacterAnimationComponent = CreateDefaultSubobject<UCharacterAnimationComponent>(TEXT("CharacterAnimationComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Interact
	(TEXT("/Game/Character/Input/Actions/IA_Interact"));
	if (IA_Interact.Succeeded())
	{
		IAInteract = IA_Interact.Object;
	}
	
	NetUpdateFrequency = 66;
	MinNetUpdateFrequency = 33;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraBoom = FindComponentByClass<USpringArmComponent>();
	FollowCamera = FindComponentByClass<UCameraComponent>();
	ViewSceneComponent = FindSceneComponentByName(FName("ViewComponent"));
	SpectatorSceneComponent = FindSceneComponentByName(FName("SpectatorCamera"));
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	InitPlayerState();
	SetNearestInteractable();
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	ShooterController = !ShooterController ? Cast<AShooterController>(Controller) : ShooterController;
	if (ShooterController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(ShooterController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ShooterMappingContext, 0);
			ShooterMovementComponent->SetupInputBindings(PlayerInputComponent);
			InventoryComponent->SetupInputBindings(PlayerInputComponent);
			WeaponManagerComponent->SetupInputBindings(PlayerInputComponent);
			WeaponActionComponent->SetupInputBindings(PlayerInputComponent);
		}
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
		if (EnhancedInputComponent)
		{
			EnhancedInputComponent->BindAction(IAInteract, ETriggerEvent::Started, this, &AShooterCharacter::Interact);
		}
	}
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AShooterCharacter, CharacterState);
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ShooterMovementComponent)
	{
		ShooterMovementComponent->SetOwnerShooterController(this);
	}
	if (CharacterAnimationComponent)
	{
		CharacterAnimationComponent->SetOwnerShooterCharacter(this);
	}
	if (HealthComponent)
	{
		HealthComponent->SetOwnerShooterCharacter(this);
	}
	if (InventoryComponent)
	{
		InventoryComponent->SetOwnerShooterCharacter(this);
	}
	if (WeaponManagerComponent)
	{
		WeaponManagerComponent->SetOwnerShooterCharacter(this);
	}
	if (WeaponActionComponent)
	{
		WeaponActionComponent->SetOwnerShooterCharacter(this);
		WeaponActionComponent->SetWeaponManagerComponent(WeaponManagerComponent);
	}
}

// Init
void AShooterCharacter::InitPlayerState()
{
	if (ShooterPlayerState == nullptr)
	{
		ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
		if (ShooterPlayerState)
		{
			ShooterPlayerState->AddToScore(0.f);
			ShooterPlayerState->AddToDefeats(0);
		}
	}
}

// Elim
void AShooterCharacter::Destroyed()
{
	Super::Destroyed();
	if (GetEquippedWeapon() && GetbDisableGameplay())
	{
		GetEquippedWeapon()->Destroy();
	}
}

// Overlap Events
void AShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

// Interact
void AShooterCharacter::Interact()
{
	if (NearestInteractable)
	{
		NearestInteractable->Interact(this);
	}
}

void AShooterCharacter::SetNearestInteractable()
{
	float Dist = 1000.f;
	for (IInteractableObject* Item : OverlappingInteractables)
	{
		AActor* InteractableActor = Cast<AActor>(Item);

		if (InteractableActor && GetDistanceTo(InteractableActor) < Dist)
		{
			Dist = GetDistanceTo(InteractableActor);
			NearestInteractable = Item;
		}
	}
	if (!OverlappingInteractables.Num())
		NearestInteractable = nullptr;
	
	ShooterController = !ShooterController ? Cast<AShooterController>(Controller) : ShooterController;
	if (ShooterController)
	{
		ShooterController->SetHUDInteract(NearestInteractable != nullptr);
	}
}

void AShooterCharacter::AddOverlappingInteractable(IInteractableObject* Interactable)
{
	OverlappingInteractables.AddUnique(Interactable);
}

void AShooterCharacter::RemoveOverlappingInteractable(IInteractableObject* Interactable)
{
	OverlappingInteractables.Remove(Interactable);
}

// Inventory
bool AShooterCharacter::CheckInventory(EItemType ItemType)
{
	TMap<AItemObject*, FTileStruct> ItemMap = InventoryComponent->GetAllItems();
	
	for (auto& Elem : ItemMap)
	{
		AItemObject* ItemObject = Elem.Key;

		if (ItemObject->GetItemType() == ItemType)
			return true;
	}
	return false;
}

void AShooterCharacter::TryAddInteractableItem(AItem *Item)
{
	AItemObject* ItemObjectInstance = NewObject<AItemObject>(this, Item->GetItemObjectClass());
	
	if (ItemObjectInstance)
	{
		if (InventoryComponent->TryAddItem(ItemObjectInstance))
		{
			RemoveOverlappingInteractable(Item);
			InventoryComponent->ServerDestroyItem(Item);
		}
	}
}

void AShooterCharacter::SetWeapons()
{
	if (WeaponManagerComponent)
	{
		WeaponManagerComponent->ServerSetWeapons();
	}
}

// Handler
void AShooterCharacter::HandleRequest(ECharacterAction RequestedAction)
{
	if (IsRunning())
	{
		ShooterMovementComponent->ForceStopRun();
		
		ExecuteDelayedAction([this, RequestedAction]()
		{
			HandleRequest(RequestedAction);
		}, 0.2f);
		return;
	}
	switch (RequestedAction)
	{
	case ECharacterAction::ECA_Fire:
		WeaponActionComponent->Attack();
		break;
	case ECharacterAction::ECA_Aim:
		WeaponActionComponent->Aim();
		break;
	case ECharacterAction::ECA_Reload:
		WeaponActionComponent->Reload();
		break;
	case ECharacterAction::ECA_Swap:
		WeaponManagerComponent->SwapWeapon();
		break;
	}
}

void AShooterCharacter::ExecuteDelayedAction(std::function<void()> Action, float Delay)
{
	GetWorldTimerManager().SetTimer(
		RunEndTimer,
		[Action]() { Action(); },
		Delay,
		false
	);
}

void AShooterCharacter::HandleCharacterState()
{
	switch (CharacterState)
	{
	case ECharacterState::ECS_Reloading:
		WeaponActionComponent->StopAiming();
		CharacterAnimationComponent->PlayReloadMontage();
		break;
	case ECharacterState::ECS_AimingGrenade:
		CharacterAnimationComponent->PlayThowGrenadeMontage(false);
		break;
	case ECharacterState::ECS_ThrowingGrenade:
		CharacterAnimationComponent->PlayThowGrenadeMontage(true);
		if (GetEquippedWeapon())
			GetEquippedWeapon()->AttackInitiated();
		break;
	case ECharacterState::ECS_SwapEquipping:
		CharacterAnimationComponent->PlaySwapEquipMontage();
		break;
	case ECharacterState::ECS_SwapUnequipping:
		WeaponActionComponent->StopAiming();
		CharacterAnimationComponent->PlaySwapUnequipMontage();
		break;
	case ECharacterState::ECS_Eliminated:
		HandleElimination();
		break;
	default:
		break;
	}
}

void AShooterCharacter::OnRep_CharacterState()
{
	switch (CharacterState)
	{
	case ECharacterState::ECS_Reloading:
		if (!HasAuthority() && !IsLocallyControlled())
		{
			CharacterAnimationComponent->PlayReloadMontage();
			WeaponActionComponent->StopAiming();
			WeaponActionComponent->HandleReload();
		}
		break;
	case ECharacterState::ECS_AimingGrenade:
		if (!HasAuthority() && !IsLocallyControlled())
			CharacterAnimationComponent->PlayThowGrenadeMontage(false);
		break;
	case ECharacterState::ECS_ThrowingGrenade:
		if (!HasAuthority() && !IsLocallyControlled())
		{
			CharacterAnimationComponent->PlayThowGrenadeMontage(true);
			GetEquippedWeapon()->AttackInitiated();
		}
		break;
	case ECharacterState::ECS_Eliminated:
		HandleElimination();
		break;
	}
}

void AShooterCharacter::HandleElimination()
{
	if (GetEquippedWeapon()) GetEquippedWeapon()->Dropped();
	CharacterAnimationComponent->PlayElimMontage();
	WeaponActionComponent->AttackButtonReleased();
	HealthComponent->Elim();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


// Check Functions
bool AShooterCharacter::IsWeaponEquipped()
{
	return GetEquippedWeapon() != nullptr;
}

bool AShooterCharacter::IsAiming()
{
	return WeaponActionComponent && WeaponActionComponent->GetbAiming();
}

bool AShooterCharacter::IsFiring()
{
	return WeaponActionComponent && WeaponActionComponent->GetbFireButtonPressed();
}

bool AShooterCharacter::IsJumping()
{
	return GetCharacterMovement()->IsFalling();
}

bool AShooterCharacter::IsIdle()
{
	return GetCharacterMovement()->Velocity.Length() == 0;
}

bool AShooterCharacter::IsSwapping()
{
	return CharacterState == ECharacterState::ECS_SwapUnequipping || CharacterState == ECharacterState::ECS_SwapEquipping;
}

bool AShooterCharacter::IsReloading()
{
	return CharacterState == ECharacterState::ECS_Reloading;
}

bool AShooterCharacter::IsUnoccupied()
{
	return CharacterState == ECharacterState::ECS_Unoccupied;
}

bool AShooterCharacter::IsThrowingGrenade()
{
	return CharacterState == ECharacterState::ECS_AimingGrenade ||
		CharacterState == ECharacterState::ECS_ThrowingGrenade;
}

bool AShooterCharacter::IsRunning()
{
	if (ShooterMovementComponent)
		return ShooterMovementComponent->GetbIsRun();
	else
		return false;
}

// Getter
ETurningInPlace AShooterCharacter::GetTurningInPlace()
{
	if (CharacterAnimationComponent)
		return CharacterAnimationComponent->GetTurningInPlace();
	else
		return ETurningInPlace::ETIP_NotTurning;
}

float AShooterCharacter::GetAO_Yaw()
{
	if (CharacterAnimationComponent)
		return CharacterAnimationComponent->GetAO_Yaw();
	else
		return 0;
}

float AShooterCharacter::GetAO_Pitch()
{
	if (CharacterAnimationComponent)
		return CharacterAnimationComponent->GetAO_Pitch();
	else
		return 0;
}

bool AShooterCharacter::ShouldRotateRootBone() const
{
	if (CharacterAnimationComponent)
		return CharacterAnimationComponent->GetbRotateRootBone();
	else
		return true;
}

float AShooterCharacter::GetHealth() const
{
	if (HealthComponent)
		return HealthComponent->GetHealth();
	else
		return 0;
}

float AShooterCharacter::GetMaxHealth() const
{
	if (HealthComponent)
		return HealthComponent->GetMaxHealth();
	else
		return 0;
}

bool AShooterCharacter::GetbDisableGameplay()
{
	if (HealthComponent)
		return HealthComponent->GetbDisableGameplay();
	else
		return false;
}

void AShooterCharacter::SetbDisableGameplay(bool val)
{
	if (HealthComponent)
	{
		HealthComponent->SetbDisableGameplay(val);
	}
}

AWeapon* AShooterCharacter::GetEquippedWeapon()
{
	if (WeaponManagerComponent == nullptr) return nullptr;

	return WeaponManagerComponent->GetEquippedWeapon();
}

FVector AShooterCharacter::GetHitTarget() const
{
	if (WeaponActionComponent == nullptr) return FVector();

	return WeaponActionComponent->GetHitResult().ImpactPoint;
}


// Utils
USceneComponent* AShooterCharacter::FindSceneComponentByName(FName ComponentName)
{
	TArray<USceneComponent*> SceneComponents;
	GetComponents(SceneComponents);
	
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (SceneComponent && SceneComponent->GetName() == ComponentName.ToString())
		{
			return SceneComponent;
		}
	}
	return nullptr;
}