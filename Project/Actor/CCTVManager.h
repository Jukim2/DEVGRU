#pragma once

#include "Project/Interfaces/InteractableObject.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CCTVManager.generated.h"

UCLASS()
class PROJECT_API ACCTVManager : public AActor, public IInteractableObject
{
	GENERATED_BODY()
	
public:	
	ACCTVManager();
	virtual void Interact(AActor* Interactor) override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class AShooterController* ShooterController;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* SkeletalMeshComponent;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereCollisionComponent;
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TurnOffCCTV(class AShooterCharacter* ShooterCharacter);
	void AddCCTVWidget();
	void TurnOnCCTV(AShooterCharacter* ShooterCharacter);
	void WatchNextCCTV();
	void WatchPrevCCTV();
	
	UPROPERTY()
	TArray<AActor*> CameraActors;
	UPROPERTY()
	class UCCTVWidget* CCTVWidget;
	UPROPERTY(VisibleAnywhere)
	TSubclassOf<UCCTVWidget> CCTVWidgetClass;
	bool bInteracting = false;
	int CurrentCameraIndex;
};
