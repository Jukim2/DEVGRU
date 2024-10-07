

#include "WeaponManagerComponent.h"

#include "EnhancedInputComponent.h"
#include "Components/Image.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Project/Character/ShooterCharacter.h"
#include "Project/Controller/ShooterController.h"
#include "Project/HUD/CharacterOverlay.h"
#include "Project/ShooterTypes/CharacterStates.h"
#include "Project/Weapon/Weapon.h"
#include "Project/Weapon/WeaponTypes.h"
#include "Project/Weapon/RangedWeapon/RangedWeapon.h"
#include "Project/HUD/ShooterHUD.h"
#include "Sound/SoundCue.h"

UWeaponManagerComponent::UWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_Equip
	(TEXT("/Game/Character/Input/Actions/IA_Equip"));
	if (IA_Equip.Succeeded())
	{
		IAEquip = IA_Equip.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_SwapToRifle
	(TEXT("/Game/Character/Input/Actions/IA_SwapToRifle"));
	if (IA_SwapToRifle.Succeeded())
	{
		IASwapToRifle = IA_SwapToRifle.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_SwapToPistol
	(TEXT("/Game/Character/Input/Actions/IA_SwapToPistol"));
	if (IA_SwapToPistol.Succeeded())
	{
		IASwapToPistol = IA_SwapToPistol.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction>IA_SwapToGrenade
	(TEXT("/Game/Character/Input/Actions/IA_SwapToGrenade"));
	if (IA_SwapToGrenade.Succeeded())
	{
		IASwapToGrenade = IA_SwapToGrenade.Object;
	}
}

void UWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerShooterCharacter = Cast<AShooterCharacter>(GetOwner());
	Controller = OwnerShooterCharacter->GetShooterController();

	if (OwnerShooterCharacter && OwnerShooterCharacter->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
}

void UWeaponManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!OwnerShooterCharacter)
		OwnerShooterCharacter = Cast<AShooterCharacter>(GetOwner());
	if (OwnerShooterCharacter && !Controller)
		Controller = OwnerShooterCharacter->GetShooterController();

}

void UWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UWeaponManagerComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UWeaponManagerComponent, EquippedWeapon);
	DOREPLIFETIME(UWeaponManagerComponent, StoredPistol);
	DOREPLIFETIME(UWeaponManagerComponent, StoredGrenade);
}

void UWeaponManagerComponent::SetupInputBindings(UInputComponent* InputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(IAEquip, ETriggerEvent::Triggered, this, &UWeaponManagerComponent::TryEquipWeapon);
		EnhancedInputComponent->BindAction(IASwapToRifle, ETriggerEvent::Started, this, &UWeaponManagerComponent::SwapToRifle);
		EnhancedInputComponent->BindAction(IASwapToPistol, ETriggerEvent::Started, this, &UWeaponManagerComponent::SwapToPistol);
		EnhancedInputComponent->BindAction(IASwapToGrenade, ETriggerEvent::Started, this, &UWeaponManagerComponent::SwapToGrenade);
	}
}

void UWeaponManagerComponent::ServerSetWeapons_Implementation()
{
	AWeapon* WeaponToEquip;
	if (RifleWeapon)
	{
		WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(RifleWeapon);
		EquipWeapon(WeaponToEquip);
	}
	if (PistolWeapon)
	{
		WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(PistolWeapon);
		StoreWeapon(WeaponToEquip);
	}
	if (GrenadeWeapon)
	{
		WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(GrenadeWeapon);
		StoreWeapon(WeaponToEquip);
	}
}

// Equip (Only Server)
void UWeaponManagerComponent::TryEquipWeapon(const FInputActionValue& Value)
{
	if (OwnerShooterCharacter == nullptr || !OwnerShooterCharacter->GetOverlappingWeapon() || OwnerShooterCharacter->GetCharacterState() != ECharacterState::ECS_Unoccupied) return;

	AWeapon* WeaponToEquip = OwnerShooterCharacter->GetOverlappingWeapon();
	if (WeaponMap.Contains(WeaponToEquip->GetWeaponType()))
	{
		WeaponMap[WeaponToEquip->GetWeaponType()]->Dropped();
	}
	else if (EquippedWeapon == nullptr)
	{
		EquipWeapon(WeaponToEquip);
	}
	else
	{
		StoreWeapon(WeaponToEquip);
	}
}

void UWeaponManagerComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	CurrWeaponType = WeaponToEquip->GetWeaponType();
	WeaponMap.Emplace(CurrWeaponType, WeaponToEquip);
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachWeaponToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(OwnerShooterCharacter); // Owner is already Replicated
	
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(WeaponToEquip);	
	
	if (OwnerShooterCharacter)
	{
		SetWeaponUI(EquippedWeapon, 1.f);
	}
}

void UWeaponManagerComponent::StoreWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Pistol)
		StoredPistol = WeaponToEquip;
	else if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Grenade)
		StoredGrenade = WeaponToEquip;
	
	WeaponToEquip->SetWeaponState(EWeaponState::EWS_Stored);
	WeaponToEquip->SetOwner(OwnerShooterCharacter);
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);
	
	WeaponMap.Emplace(WeaponToEquip->GetWeaponType(), WeaponToEquip);
	
	if (OwnerShooterCharacter)
	{
		SetWeaponUI(WeaponToEquip, 0.2);
	}
}

void UWeaponManagerComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && OwnerShooterCharacter)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachWeaponToRightHand(EquippedWeapon);
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo();
		
		CurrWeaponType = EquippedWeapon->GetWeaponType();
		WeaponMap.Emplace(CurrWeaponType, EquippedWeapon);
	}
}

void UWeaponManagerComponent::OnRep_StorePistol()
{
	if (StoredPistol && OwnerShooterCharacter)
	{
		StoredPistol->SetWeaponState(EWeaponState::EWS_Stored); 
		AttachActorToBackpack(StoredPistol);
		PlayEquipWeaponSound(StoredPistol);

		WeaponMap.Emplace(StoredPistol->GetWeaponType(), StoredPistol);

		if (OwnerShooterCharacter && OwnerShooterCharacter->IsLocallyControlled())
		{
			SetWeaponUI(StoredPistol, 0.2);
		}
	}
}

void UWeaponManagerComponent::OnRep_StoreGrenade()
{
	if (StoredGrenade && OwnerShooterCharacter)
	{
		StoredGrenade->SetWeaponState(EWeaponState::EWS_Stored); 
		AttachActorToBackpack(StoredGrenade);
		PlayEquipWeaponSound(StoredGrenade);

		WeaponMap.Emplace(StoredGrenade->GetWeaponType(), StoredGrenade);

		if (OwnerShooterCharacter && OwnerShooterCharacter->IsLocallyControlled())
		{
			SetWeaponUI(StoredGrenade, 0.2);
		}
	}
}

void UWeaponManagerComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip && WeaponToEquip->GetEquipSound())
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->GetEquipSound(),
			OwnerShooterCharacter->GetActorLocation()
		);
	}
}

// Swap
void UWeaponManagerComponent::SwapToRifle(const FInputActionValue& Value)
{
	RequestSwapWeapon(EWeaponType::EWT_Rifle);
}

void UWeaponManagerComponent::SwapToPistol(const FInputActionValue& Value)
{
	RequestSwapWeapon(EWeaponType::EWT_Pistol);
}

void UWeaponManagerComponent::SwapToGrenade(const FInputActionValue& Value)
{
	RequestSwapWeapon(EWeaponType::EWT_Grenade);
}

void UWeaponManagerComponent::RequestSwapWeapon(EWeaponType WeaponType)
{
	if (!EquippedWeapon || 
		(WeaponMap.Contains(WeaponType) && EquippedWeapon == WeaponMap[WeaponType]))
		return;

	NextWeaponType = WeaponType;
	OwnerShooterCharacter->HandleRequest(ECharacterAction::ECA_Swap);
}

void UWeaponManagerComponent::SwapWeapon()
{
	SwapWeaponUI();
	ServerSwapWeapon(NextWeaponType);
}

void UWeaponManagerComponent::ServerSwapWeapon_Implementation(EWeaponType WeaponType)
{
	MulticastSwapWeapon(WeaponType);
}

void UWeaponManagerComponent::MulticastSwapWeapon_Implementation(EWeaponType WeaponType)
{
	NextWeaponType = WeaponType;
	if (!EquippedWeapon)
		OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_SwapEquipping);
	else
		OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_SwapUnequipping);
	OwnerShooterCharacter->HandleCharacterState();
}

void UWeaponManagerComponent::SwapUnequip()
{
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Stored);
	AttachActorToBackpack(EquippedWeapon);
}

void UWeaponManagerComponent::SwapEquip() 
{
	CurrWeaponType = NextWeaponType;
	if (WeaponMap.Contains(CurrWeaponType))
	{
		EquippedWeapon = WeaponMap[CurrWeaponType];
		
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachWeaponToRightHand(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo();	
		UpdateCarriedAmmo();
		PlayEquipWeaponSound(EquippedWeapon);
	}
}


void UWeaponManagerComponent::SwapEnd()
{
	if (OwnerShooterCharacter)
		OwnerShooterCharacter->SetCharacterState(ECharacterState::ECS_Unoccupied);
}

// Attach Weapon
void UWeaponManagerComponent::AttachActorToBackpack(AWeapon* WeaponToAttach)
{
	if (OwnerShooterCharacter == nullptr || OwnerShooterCharacter->GetMesh() == nullptr || WeaponToAttach == nullptr) return;

	const USkeletalMeshSocket* WeaponStoreSocket = nullptr;
	if (WeaponToAttach->GetWeaponType() == EWeaponType::EWT_Rifle)
		WeaponStoreSocket = OwnerShooterCharacter->GetMesh()->GetSocketByName(FName("RifleStoreSocket"));
	else if (WeaponToAttach->GetWeaponType() == EWeaponType::EWT_Pistol)
		WeaponStoreSocket = OwnerShooterCharacter->GetMesh()->GetSocketByName(FName("PistolStoreSocket"));
	else if (WeaponToAttach->GetWeaponType() == EWeaponType::EWT_Grenade)
		WeaponStoreSocket = OwnerShooterCharacter->GetMesh()->GetSocketByName(FName("GrenadeStoreSocket"));
	if (WeaponStoreSocket)
	{
		WeaponStoreSocket->AttachActor(WeaponToAttach, OwnerShooterCharacter->GetMesh());
	}
}

void UWeaponManagerComponent::AttachWeaponToRightHand(AWeapon* WeaponToAttach)
{
	if (OwnerShooterCharacter == nullptr || OwnerShooterCharacter->GetMesh() == nullptr || WeaponToAttach == nullptr || EquippedWeapon == nullptr) return;
	
	const USkeletalMeshSocket* HandSocket = nullptr;
	if (WeaponToAttach->GetWeaponType() == EWeaponType::EWT_Rifle)
		HandSocket = OwnerShooterCharacter->GetMesh()->GetSocketByName(FName("RifleSocket"));
	else if (WeaponToAttach->GetWeaponType() == EWeaponType::EWT_Pistol)
		HandSocket = OwnerShooterCharacter->GetMesh()->GetSocketByName(FName("PistolSocket"));
	else if (WeaponToAttach->GetWeaponType() == EWeaponType::EWT_Grenade)
		HandSocket = OwnerShooterCharacter->GetMesh()->GetSocketByName(FName("GrenadeSocket"));	
	if (HandSocket)
	{
		// propagates down to client
		HandSocket->AttachActor(WeaponToAttach, OwnerShooterCharacter->GetMesh());
	}
}

// Ammo
void UWeaponManagerComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Rifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Grenade, StartingGrenadeAmmo);
}

void UWeaponManagerComponent::UpdateCarriedAmmo()
{
	if (CarriedAmmoMap.Contains(CurrWeaponType))
	{
		CarriedAmmo = CarriedAmmoMap[CurrWeaponType];
	}
	Controller = Controller == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UWeaponManagerComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UWeaponManagerComponent::UpdateAmmoValues()
{
	if (OwnerShooterCharacter == nullptr || Cast<ARangedWeapon>(EquippedWeapon) == nullptr) return;
	
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	Cast<ARangedWeapon>(EquippedWeapon)->AddAmmo(-ReloadAmount);
}

int32 UWeaponManagerComponent::AmountToReload()
{
	if (Cast<ARangedWeapon>(EquippedWeapon) == nullptr) return 0;
	int32 RoomInMag = Cast<ARangedWeapon>(EquippedWeapon)->GetMagCapacity() - Cast<ARangedWeapon>(EquippedWeapon)->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}


// WeaponUI
void UWeaponManagerComponent::SetWeaponUI(AWeapon* Weapon, float Opacity)
{
	if (OwnerShooterCharacter)
		Controller = Controller == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : Controller;
	if (Controller)
		HUD = HUD == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : HUD;

	uint8 WeaponIdx = static_cast<uint8>(Weapon->GetWeaponType());
	if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->WeaponUIArray[WeaponIdx])
	{
		FLinearColor CurrentColor = HUD->CharacterOverlay->WeaponUIArray[WeaponIdx]->GetColorAndOpacity();
		CurrentColor.A = Opacity;
		HUD->CharacterOverlay->WeaponUIArray[WeaponIdx]->SetColorAndOpacity(CurrentColor);
	}
}

void UWeaponManagerComponent::SetWeaponUIOpacity(int8 WeaponIdx, float Opacity)
{
	if (OwnerShooterCharacter)
		Controller = Controller == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : Controller;
	if (Controller)
		HUD = HUD == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : HUD;
	if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->WeaponUIArray[WeaponIdx])
	{
		FLinearColor CurrentColor = HUD->CharacterOverlay->WeaponUIArray[WeaponIdx]->GetColorAndOpacity();
		CurrentColor.A = Opacity;
		HUD->CharacterOverlay->WeaponUIArray[WeaponIdx]->SetColorAndOpacity(CurrentColor);
	}
}

void UWeaponManagerComponent::SwapWeaponUI()
{
	if (OwnerShooterCharacter)
		Controller = Controller == nullptr ? Cast<AShooterController>(OwnerShooterCharacter->Controller) : Controller;
	if (Controller)
		HUD = HUD == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : HUD;

	uint8 CurrWeaponIdx = static_cast<uint8>(CurrWeaponType);
	uint8 NextWeaponIdx = static_cast<uint8>(NextWeaponType);
	if (HUD && HUD->CharacterOverlay && HUD->CharacterOverlay->WeaponUIArray[CurrWeaponIdx]
		&& HUD->CharacterOverlay->WeaponUIArray[NextWeaponIdx])
	{
		FLinearColor CurrentColor = HUD->CharacterOverlay->WeaponUIArray[CurrWeaponIdx]->GetColorAndOpacity();
		CurrentColor.A = 0.2f;
		HUD->CharacterOverlay->WeaponUIArray[CurrWeaponIdx]->SetColorAndOpacity(CurrentColor);

		CurrentColor = HUD->CharacterOverlay->WeaponUIArray[NextWeaponIdx]->GetColorAndOpacity();
		CurrentColor.A = 1.f;
		HUD->CharacterOverlay->WeaponUIArray[NextWeaponIdx]->SetColorAndOpacity(CurrentColor);
	}
}

