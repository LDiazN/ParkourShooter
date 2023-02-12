// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Simple class with utility functions used in many places
 */
class PARKOURSHOOTER_API ParkourShooterUtils
{
public:
	~ParkourShooterUtils();

	static bool FloorIsWalkable(const FVector FloorNormal, float MaxWalkableAngle);
	static bool FloorIsWalkableZ(const FVector FloorNormal, float MaxWalkableZ);

private:
	// We will use only static functions here, so the constructor is not necessary
	ParkourShooterUtils();

};
