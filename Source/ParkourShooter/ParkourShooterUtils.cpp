// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourShooterUtils.h"

ParkourShooterUtils::ParkourShooterUtils()
{
}

bool ParkourShooterUtils::FloorIsWalkable(const FVector FloorNormal, float MaxWalkableAngle)
{
	// Check if this is the ceilling
	if (FloorNormal.Z < -0.05)
		return false;

	// Now we have to check surface angle. We can do it by taking the dot product between the normal 
	// vector and its own XY projection
	FVector Projection(FloorNormal.X, FloorNormal.Y, 0);
	Projection.Normalize();
	float FloorAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FloorNormal, Projection)));
	FloorAngle = 180 - FloorAngle - 90;

	return  MaxWalkableAngle >= FloorAngle;
}

bool ParkourShooterUtils::FloorIsWalkableZ(const FVector FloorNormal, float MaxWalkableZ)
{
	return FloorNormal.Z >= MaxWalkableZ;
}

ParkourShooterUtils::~ParkourShooterUtils()
{
}
