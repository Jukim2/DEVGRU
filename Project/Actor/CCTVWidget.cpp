// Fill out your copyright notice in the Description page of Project Settings.


#include "CCTVWidget.h"

#include "Components/TextBlock.h"

void UCCTVWidget::SetCCTVNum(int32 Num)
{
	CCTVNum->SetText(FText::AsNumber(Num));
}
