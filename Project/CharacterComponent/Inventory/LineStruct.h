#pragma once

#include "CoreMinimal.h"
#include "LineStruct.generated.h"

USTRUCT(BlueprintType)
struct FLineStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Line")
	FVector2D Start = FVector2d::Zero();

	UPROPERTY(EditAnywhere, Category = "line")
	FVector2D End = FVector2d::Zero();
};