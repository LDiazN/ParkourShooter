// Fill out your copyright notice in the Description page of Project Settings.

#include "ParkourShooterUtils.h"
#include "ParkourShooterCharacter.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VaultComponent.h"

// Sets default values for this component's properties
UVaultComponent::UVaultComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UVaultComponent::BeginPlay()
{
	Super::BeginPlay();
	ShooterCharacter =  Cast<AParkourShooterCharacter>(GetOwner());
	VaultSuggestionWidget = CreateWidget(Cast<APlayerController>(ShooterCharacter->GetController()), VaultSuggestionClass);
	// if (VaultSuggestionWidget != nullptr)
	// 	VaultSuggestionWidget->AddToViewport();
}

UVaultComponent::VaultingState UVaultComponent::GetCurrentState() const
{
	return CurrentState;
}

void UVaultComponent::SetVaultingState(VaultingState NewVaultState)
{
	CurrentState = NewVaultState;
}

bool UVaultComponent::CanVault(FVector& OutFinalPosition) const
{
	// If you're already vaulting, the you can't vault
	if (CurrentState != VaultingState::NotVaulting)
		return false;

	UCapsuleComponent * Capsule = ShooterCharacter->GetCapsuleComponent();

	// We want to cast a ray right in front of our character
	// pointing to the ground, to check height of the object we would 
	// want to jump over. 
	FVector Start =
		ShooterCharacter->GetActorLocation() +
		ShooterCharacter->GetActorForwardVector() * DistanceFromPlayer + // How much to the front
		ShooterCharacter->GetActorUpVector() * Capsule->GetScaledCapsuleHalfHeight();       // How much Upwards
		 
	// End is in the ground just in front of you
	FVector End = Start - ShooterCharacter->GetActorUpVector() * Capsule->GetScaledCapsuleHalfHeight() * 2;


	FHitResult Hit;
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.AddIgnoredActor(ShooterCharacter);
	
	// DEBUG ONLY, DELETE LATER  -------
	// const FName MyTag("VaultTracing");
	// GetWorld()->DebugDrawTraceTag = MyTag;
	// Params.TraceTag = MyTag;
	// ---------------------------------

	// Check if there's something to grab to 
	bool HitSomething = GetWorld()->LineTraceSingleByChannel(
		Hit, 
		Start, End, 
		ECollisionChannel::ECC_Visibility, 
		Params
	);

	if (!HitSomething)
		return false;

	FVector FinalPosition;
	if (!CanVaultToLocation(Hit, FinalPosition))
		return false;

	OutFinalPosition = FinalPosition;

	return true;
}

bool UVaultComponent::CanVaultToLocation(const FHitResult& Hit, FVector& OutFinalPosition) const
{

	// Check if the place we want to go is walkable to start with
	if (!ParkourShooterUtils::FloorIsWalkableZ(Hit.Normal, ShooterCharacter->GetCharacterMovement()->GetWalkableFloorZ()))
		return false;

	// Get height of place we want to vault into so we can decide what kind of vault we do
	float Height = Hit.Location.Z - Hit.TraceEnd.Z;

	// If too high or too low, we dont vault
	if (Height > MaxVaultingHeight || Height < MinVaultingHeight)
		return false;

	UCapsuleComponent *Capsule = ShooterCharacter->GetCapsuleComponent();

	// Check if we fit: We spawn a capsule in the position we want to be and if it doesn't hits anything, everything is ok
	// We first search the position for the capsule to spawn: Hit location plus half height of capsule
	// We add capsule radius to account for slope surfaces
	FVector CapsuleLocation = FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight() + Capsule->GetScaledCapsuleRadius()) + Hit.Location;
	FHitResult CapsuleHit;
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.AddIgnoredActor(ShooterCharacter);

	// DEBUG ONLY, DELETE LATER  -------
	// const FName MyTag("VaultTracing");
	// GetWorld()->DebugDrawTraceTag = MyTag;
	// Params.TraceTag = MyTag;
	// FCollisionObjectQueryParams ObjectTypes;
	// ---------------------------------

	// TODO you have to properly set up what you want to hit here since we might want to vault objects with enemies 
	// over them, for example
	bool HitSomething = GetWorld()->SweepSingleByChannel(
		CapsuleHit,
		CapsuleLocation,
		CapsuleLocation,
		ShooterCharacter->GetActorRotation().Quaternion(),
		ECC_Visibility,
		FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()),
		Params
	);

	// If not enough space, return
	if (HitSomething)
		return false;


	OutFinalPosition = FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight()) + Hit.Location;
	return true;
}

void UVaultComponent::ModifyWidgetToViewport(bool Add)
{
	//if (VaultSuggestionWidget != nullptr && Add && !VaultSuggestionWidget->IsInViewport())
	//	VaultSuggestionWidget->AddToViewport();
	//else if (VaultSuggestionWidget != nullptr && !Add && VaultSuggestionWidget->IsInViewport())
	//	VaultSuggestionWidget->RemoveFromViewport();
}

void UVaultComponent::BeginVault(FVector NewLocation)
{
	Progress = 0;
	StartingLocation = ShooterCharacter->GetActorLocation();
	EndLocation = NewLocation;
	CurrentState = VaultingState::Vaulting;
}

void UVaultComponent::UpdateVault(float DeltaSeconds)
{
	Progress += DeltaSeconds / TimeToVault;
	Progress = FMath::Clamp(Progress, 0.f, 1.f);

	FVector NewLocation = FMath::Lerp(StartingLocation, EndLocation, Progress);

	ShooterCharacter->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

	// Check if all required time just passed or if we are near enough
	float DistSquared =  (ShooterCharacter->GetActorLocation() - EndLocation).SizeSquared();
	if (Progress >= 1.f || DistSquared <= 10 * 10)
		CurrentState = VaultingState::NotVaulting;
}

// Called every frame
void UVaultComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FVector NewLocation;
	switch (CurrentState)
	{
	case VaultingState::NotVaulting:
		if (CanVault(NewLocation))
			ModifyWidgetToViewport(true);
		else 
			ModifyWidgetToViewport(false);

		break;
	case VaultingState::Vaulting:
		UpdateVault(DeltaTime);
		break;
	default:
		break;
	}
}

