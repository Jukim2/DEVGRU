// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemObject.h"

void AItemObject::Rotate()
{
	Rotated = !Rotated;
}

FIntPoint AItemObject::GetDimensions() const
{
	if (Rotated)
	{
		return Dimensions;
	}
	else
	{
		FIntPoint RotatedDimension(Dimensions.Y, Dimensions.X);
		return RotatedDimension;
	}
}
