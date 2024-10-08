#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableObject.generated.h"

UINTERFACE(MinimalAPI)
class UInteractableObject : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_API IInteractableObject
{
	GENERATED_BODY()

public:
	virtual void Interact(AActor* Interactor) = 0;
};
