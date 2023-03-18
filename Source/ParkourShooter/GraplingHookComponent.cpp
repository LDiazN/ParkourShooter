// Fill out your copyright notice in the Description page of Project Settings.


#include "GraplingHookComponent.h"
#include "CableComponent.h"
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
	// Some sanity checks first thing in the morning:
	
	// If already in use, nothing to do
	if (IsInUse() || GetWorld() == nullptr)
		return;

	// You can crash the app if you don't check if this class is valid
	if (!IsValid(GrapleHookClass))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not spawn graple hook actor since it's not a valid subclass. Did you forget to set the class to be spawned?"));
		return;
	}

	if (!IsValid(GrapleCableClass))
	{
		UE_LOG(LogTemp, Error, TEXT("Can't spawn grapple hook cable since it's not a valid subclass. Did you forget to set the cable class to be spawned?"))
		return;
	}

	CurrentState = GrapplingState::Firing;

	// We want to get the direction we will be moving on, we get that by substracting target
	// location by the start location in world coordinates
	FVector GrappleDirection = GetMovementDirection(Target, LocalOffset);
	GrappleDirection.Normalize();
	FireDirection = GrappleDirection;


	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	FVector StartLocation = GrapplingHookStartLocation(LocalOffset);
	HookObject =
		GetWorld()->SpawnActor<AGrapleHook>(
			GrapleHookClass, 
			StartLocation,
			GetOwner()->GetActorRotation(),
			SpawnParams
		);

	if (HookObject == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not spawn hook actor"));
		CurrentState = GrapplingState::ReadyToFire;
		return;
	}

	// Set up hook object: Set velocity and bind relevant events
	HookObject->SetVelocity(GrappleDirection * HookSpeed);
	// Use OnHit Event to know when you hit some wall
	HookObject->OnActorHit.AddDynamic(this, &UGraplingHookComponent::OnHookHit); 

	// Use OnDestroyed to know when to stop pulling to the hook
	HookObject->OnDestroyed.AddDynamic(this, &UGraplingHookComponent::OnGrappleDestroyed);

	// Now spawn the cable from the start point to the contact point
	CableObject = GetWorld()->SpawnActor<AGrapleCableActor>(
		GrapleCableClass, 
		StartLocation, 
		GrappleDirection.Rotation(),
		SpawnParams
		);

	// If failed to spawn, reset everything
	if (CableObject == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not spawn hook cable actor"));
		GetWorld()->DestroyActor(HookObject);
		HookObject = nullptr;
		CurrentState = GrapplingState::ReadyToFire;
		return;
	}

	// Now set up end points of cable to corresponding locations
	FAttachmentTransformRules Rules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);

	// Attach to start
	CableObject->AttachToActor(GetOwner(), Rules);

	// Attach to end
	CableObject->CableComponent->SetAttachEndTo(HookObject, NAME_None);
	CableObject->CableComponent->EndLocation = FVector::ZeroVector;

	UE_LOG(LogTemp, Warning, TEXT("Spawned with speed of %f"), HookSpeed);
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

void UGraplingHookComponent::OnHookHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit something!"));
}

void UGraplingHookComponent::OnGrappleDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Hook destroyed"));
}

// Called every frame
void UGraplingHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

