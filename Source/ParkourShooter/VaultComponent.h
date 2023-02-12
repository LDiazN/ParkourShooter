// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VaultComponent.generated.h"

class UUSerWidget;
class AParkourShooterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARKOURSHOOTER_API UVaultComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVaultComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	enum class VaultingState
	{
		NotVaulting, 
		Vaulting
	};

	/**This is the acceptable distance in front of the player to check for an object to vault on*/
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting")
	float DistanceFromPlayer = 70;

	UPROPERTY(EditDefaultsOnly, Category="Vaulting")
	AParkourShooterCharacter* ShooterCharacter;

	UPROPERTY(EditAnywhere, Category = "Vaulting")
	TSubclassOf<class UUserWidget> VaultSuggestionClass;

	UUserWidget* VaultSuggestionWidget;

	VaultingState CurrentState;

	FVector EndingLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Vaulting")
	float MinVaultingHeight = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Vaulting")
	float MaxVaultingHeight = 170;

	/** Time in seconds to perform vault */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting")
	float TimeToVault = 1;

	// Marks how much progress we have with vaulting
	float Progress = 0;

	// Where we started vaulting
	FVector StartingLocation;

	// Where we finish vaulting
	FVector EndLocation;

	VaultingState GetCurrentState() const;

	void SetVaultingState(VaultingState NewVaultState);

	/// <summary>
	/// Checks if the specified location is "vaultable", meaning that you can vault to that 
	/// location in space. In order for a location to be vaultable, you need:
	/// 1) To be able to walk on that surface
	/// 2) To be tall enough to reach the surface
	/// 3) To be small enough to fit in that space (try to stand in a small window frame, for example)
	/// </summary>
	/// <param name="Hit">
	/// Data about the location you want to vault to, this is the result from a raycast 
	/// checking for any surface
	/// </param>
	/// <param name="OutFinalPosition"> Resulting position after performing vault</param>
	/// <returns> True if can vault, false otherwise </returns>
	bool CanVaultToLocation(const FHitResult& Hit, FVector& OutFinalPosition) const;

	/// <summary>
	/// Add vaulting widget to viewport if argument is true, or remove it if false.
	/// Does nothing if already in the correct state
	/// </summary>
	/// <param name="Add">If true, will add it to viewport, otherwise it will remove it</param>
	void ModifyWidgetToViewport(bool Add);

	UFUNCTION()
	void UpdateVault(float DeltaSeconds);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/// <summary>
	/// Check if you can actually vault, and if so, set the output as the resulting position after vault
	/// </summary>
	/// <param name="OutFinalPosition">Position after performing vault</param>
	/// <returns>if can vault</returns>
	bool CanVault(FVector& OutFinalPosition) const;

	/// <summary>
	/// Start a vaulting 
	/// </summary>
	UFUNCTION()
	void BeginVault(FVector NewLocation);

	UFUNCTION()
	bool IsVaulting() const { return CurrentState == VaultingState::Vaulting; }

};
