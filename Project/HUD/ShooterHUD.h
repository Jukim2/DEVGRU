// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};


UCLASS()
class PROJECT_API AShooterHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override;

	// CharacterOverlay
	UPROPERTY(EditAnywhere, Category="HUD")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	void AddCharacterOverlay();

	// Announcement
	UPROPERTY(EditAnywhere, Category="HUD")
	TSubclassOf<class UUserWidget> AnnouncementClass;
	UPROPERTY()
	class UAnnouncement* Announcement;
	void AddAnnouncement();
	
	// Interact
	UPROPERTY(EditAnywhere, Category="HUD")
	TSubclassOf<class UUserWidget> InteractWidgetClass;
	UPROPERTY()
	UUserWidget* InteractWidget;
	void AddInteract();
	
private:
	// Crosshair
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2d ViewportCenter, FVector2d Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float BaseCrosshairSpread = 3.5f;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	bool bShouldDrawHUD = true;

protected:
	virtual void BeginPlay() override;
	
public:
	void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
	float GetCrosshairSpread() const { return HUDPackage.CrosshairSpread; }
	float GetBaseCrosshairSpread() const { return BaseCrosshairSpread; }
	void SetbShouldDrawHUD(bool Val) { bShouldDrawHUD = Val; }
	bool GetbShouldDrawHUD() const { return bShouldDrawHUD; }
};
