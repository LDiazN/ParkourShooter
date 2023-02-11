// Fill out your copyright notice in the Description page of Project Settings.

#include "ParkourShooterCharacter.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
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
	if (VaultSuggestionWidget != nullptr)
		VaultSuggestionWidget->AddToViewport();
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
	return false;
}

void UVaultComponent::ModifyWidgetToViewport(bool Add)
{
	if (VaultSuggestionWidget != nullptr &&  Add && !VaultSuggestionWidget->IsInViewport())
		VaultSuggestionWidget->AddToViewport();
	else if (VaultSuggestionWidget != nullptr && !Add && VaultSuggestionWidget->IsInViewport())
		VaultSuggestionWidget->RemoveFromViewport();
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
	case VaultingState::WantsToVault:
		break;
	case VaultingState::Vaulting:
		break;
	default:
		break;
	}
}

