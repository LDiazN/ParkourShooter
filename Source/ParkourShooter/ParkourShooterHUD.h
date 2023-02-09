// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ParkourShooterHUD.generated.h"

UCLASS()
class AParkourShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	AParkourShooterHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

