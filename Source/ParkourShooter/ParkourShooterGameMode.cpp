// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkourShooterGameMode.h"
#include "ParkourShooterHUD.h"
#include "ParkourShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AParkourShooterGameMode::AParkourShooterGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AParkourShooterHUD::StaticClass();
}
