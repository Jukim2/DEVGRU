// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemObject.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "ItemWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetDestroyed, AItemObject*, RemovedItem);
/**
 * 
 */
UCLASS()
class PROJECT_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Initialize
	void SetVariables(float _TileSize, AItemObject* _ItemObject);
	void Refresh();

	// Event
	UPROPERTY(BlueprintAssignable, Category = "Events") FOnWidgetDestroyed OnWidgetDestroyed;
	
private:
	float TileSize;
	FVector2D Size;
	UPROPERTY()
	AItemObject* ItemObject;

	// Widget Components
	UPROPERTY(meta = (BindWidget)) UImage* ItemImage;
	UPROPERTY(meta = (BindWidget)) USizeBox* BackgroundSizeBox;
	UPROPERTY(meta = (BindWidget)) UBorder* Backgroundborder;
	
	FSlateBrush MakeBrushFromMaterial(UMaterialInterface* Material, FVector2D ImageSize);

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};


