#pragma once

#include "CoreMinimal.h"
#include "TileStruct.generated.h"

USTRUCT(BlueprintType)
struct FTileStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Tile")
	int X = 0;

	UPROPERTY(EditAnywhere, Category = "Tile")
	int Y = 0;
};