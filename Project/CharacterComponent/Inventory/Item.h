#pragma once

#include "Project/Interfaces/InteractableObject.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Item.generated.h"

UCLASS()
class PROJECT_API AItem : public AActor, public IInteractableObject
{
	GENERATED_BODY()
	
public:	
	AItem();
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereCollisionComponent;
	UPROPERTY(VisibleAnywhere)
	TSubclassOf<class AItemObject> ItemObjectClass;
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Interact(AActor* Interactor) override;

public:
	TSubclassOf<AItemObject> GetItemObjectClass() const { return ItemObjectClass; }
};

