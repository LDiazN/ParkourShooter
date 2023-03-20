// Fill out your copyright notice in the Description page of Project Settings.


#include "GraplingHookComponent.h"
#include "CableComponent.h"
#include "ParkourShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	if (!IsValid(HookClass))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not spawn graple hook actor since it's not a valid subclass. Did you forget to set the class to be spawned?"));
		return;
	}

	if (!IsValid(CableClass))
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
			HookClass, 
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
		CableClass, 
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
	// If nothing to cancel, just return
	if (!IsInUse())
		return;

	// Sanity check: If the world is not valid, we have nothing to do
	if (!IsValid(GetWorld()))
		return;

	// Try to destroy grapple hook and cable
	if (IsValid(HookObject))
		GetWorld()->DestroyActor(HookObject);

	if (IsValid(CableObject))
		GetWorld()->DestroyActor(CableObject);

	// Reset Variables
	HookObject = nullptr;
	CableObject = nullptr;

	// Reset state
	GrapplingState PrevState = CurrentState;
	CurrentState = GrapplingState::ReadyToFire;

	// If Prev state was attached, we have to reset movement properties
	if (PrevState != GrapplingState::Attached)
		return;

	// Reset movement properties
	AActor* OwnerActor = GetOwner();
	AParkourShooterCharacter* OwnerCharacter = Cast<AParkourShooterCharacter>(OwnerActor);

	// Check if Cast was valid
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("Can't get parkour shooter character owner: Failed cast"));
		return;
	}

	// Now get movement component
	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	if (MovementComp == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't get movement component from owning Parkour Shooter Character"));
		return;
	}

	SetMovementProperties(MovementComp, PreviousProperties);
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
	// Change state to attached since we hit something to attach to
	CurrentState = GrapplingState::Attached;

	// Now we have to change the movement controller so that it's easier to pull the character to the attach point
	AActor* OwnerActor = GetOwner();
	AParkourShooterCharacter* OwnerCharacter = Cast<AParkourShooterCharacter>(OwnerActor);
	
	// Check if Cast was valid
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("Can't get parkour shooter character owner: Failed cast"));
		return;
	}

	// Now get movement component
	UCharacterMovementComponent * MovementComp = OwnerCharacter->GetCharacterMovement();
	if (MovementComp == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't get movement component from owning Parkour Shooter Character"));
		return;
	}

	// Now we have to set all movement properties to values that are suitable for pulling the character to the attach point
	PreviousProperties = GetMovementProperties(MovementComp);
	SetMovementProperties(MovementComp, { 0.0f, 0.0f, 0.2f });

	FVector ToHook = ToGrappleHook();
	MovementComp->Velocity = PullInitialSpeed * ToGrappleHook();
	
	InitialHookDirection2D = FVector2D(ToHook);
	InitialHookDirection2D.Normalize();

	UE_LOG(LogTemp, Warning, TEXT("Hit something!"));
}

void UGraplingHookComponent::OnGrappleDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Hook destroyed"));
}

void UGraplingHookComponent::SetMovementProperties(UCharacterMovementComponent* MovementComponent, const UGraplingHookComponent::MovementProperties& Properties) const
{
	MovementComponent->GroundFriction = Properties.GroundFriction;
	MovementComponent->GravityScale = Properties.GravityScale;
	MovementComponent->AirControl = Properties.AirControl;
}

UGraplingHookComponent::MovementProperties UGraplingHookComponent::GetMovementProperties(UCharacterMovementComponent* MovementComponent) const
{
	return MovementProperties{MovementComponent->GroundFriction, MovementComponent->GravityScale, MovementComponent->AirControl};
}

FVector UGraplingHookComponent::ToGrappleHook() const
{
	if (!IsValid(HookObject))
		return FVector::ZeroVector;

	FVector Direction = HookObject->GetActorLocation() - GetOwner()->GetActorLocation();
	Direction.Normalize();

	return Direction;
}

bool UGraplingHookComponent::IsTooCloseToHook() const
{
	return FVector::DistSquared(GetOwner()->GetActorLocation(), HookObject->GetActorLocation()) < MinDistanceToPull * MinDistanceToPull;
}

bool UGraplingHookComponent::IsTooFarFromHook() const
{
	if (!IsValid(GetOwner()) || !IsValid(HookObject))
		return false;

	return FVector::DistSquared(GetOwner()->GetActorLocation(), HookObject->GetActorLocation()) > MaxHookDistanceFromCharacter * MaxHookDistanceFromCharacter;
}

bool UGraplingHookComponent::HookPassed() const
{
	FVector ToHook = ToGrappleHook();
	if (ToHook == FVector::ZeroVector)
		return false;

	FVector2D ToHook2D(ToHook);


	return FVector2D::DotProduct(ToHook2D, InitialHookDirection2D) < 0;
}


// Called every frame
void UGraplingHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentState == GrapplingState::Firing && IsTooFarFromHook())
	{
		CancelGrapple();
		return;
	}

	// If not attached, nothing to do
	if (CurrentState != GrapplingState::Attached)
		return;


	// If attached, pull the character towards the hook every frame
	AActor* OwnerActor = GetOwner();
	AParkourShooterCharacter* OwnerCharacter = Cast<AParkourShooterCharacter>(OwnerActor);
	if (!IsValid(OwnerCharacter))
		return;

	// Check if should cancel attachement. You cancel it if you pass the hook or if you're too close
	if (IsTooCloseToHook() || HookPassed())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hook Reached"));
		CancelGrapple();
	}

	// Pull to the specified direction
	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	FVector Direction = ToGrappleHook();
	MovementComponent->AddForce(Direction*ContinousPullSpeed);

}

