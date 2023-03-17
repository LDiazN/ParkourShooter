// Fill out your copyright notice in the Description page of Project Settings.


#include "GraplingHookComponent.h"
#include "GrapleHook.h"

// Sets default values for this component's properties
UGraplingHookComponent::UGraplingHookComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UGraplingHookComponent::FireGrapple(const FVector& Target, const FVector& LocalOffset)
{
	// If already in use, nothing to do
	if (IsInUse())
		return;

	CurrentState = GrapplingState::Firing;

	// We want to get the direction we will be moving on, we get that by substracting target
	// location by the start location in world coordinates
	FVector GrappleDirection = GetMovementDirection(Target, LocalOffset);
	GrappleDirection.Normalize();
	FireDirection = GrappleDirection;

	if (GetWorld() == nullptr)
		return;

	// You can crash the app if you don't check if this class is valid
	if (!IsValid(GrapleHookClass))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not spawn graple hook actor since it's not a valid subclass"));
		return;
	}


	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	AGrapleHook* GrappleHookObject = 
		GetWorld()->SpawnActor<AGrapleHook>(
			GrapleHookClass, 
			GrapplingHookStartLocation(LocalOffset), 
			GetOwner()->GetActorRotation(),
			SpawnParams
		);
	GrappleHookObject->SetVelocity(GrappleDirection * HookSpeed);
	UE_LOG(LogTemp, Warning, TEXT("Spawned"));
}

void UGraplingHookComponent::CancelGrapple()
{
}

// Called when the game starts
void UGraplingHookComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentState = GrapplingState::ReadyToFire;
	
}

FVector UGraplingHookComponent::GetMovementDirection(const FVector& Target, const FVector& LocalOffset) const
{
	// Now, the object vector is relative to the player, so we need to transform it by the player's 
	// transform to get a world-space vector that makes sense as starting point for the grapple hook
	FVector WorldStartLocation = GrapplingHookStartLocation(LocalOffset);
	FVector Direction = Target - WorldStartLocation;
	Direction.Normalize();
	return Direction;
}

FVector UGraplingHookComponent::GrapplingHookStartLocation(const FVector& LocalOffset) const
{
	return GetOwner()->GetActorTransform().TransformPosition(LocalOffset);
}

// Called every frame
void UGraplingHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

