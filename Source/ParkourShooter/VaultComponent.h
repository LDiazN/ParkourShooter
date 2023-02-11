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
		WantsToVault,
		Vaulting
	};


	UPROPERTY(EditDefaultsOnly, Category="Vaulting")
	AParkourShooterCharacter* ShooterCharacter;

	UPROPERTY(EditAnywhere, Category = "Vaulting")
	TSubclassOf<class UUserWidget> VaultSuggestionClass;

	UUserWidget* VaultSuggestionWidget;

	VaultingState CurrentState;

	FVector EndingLocation;

	VaultingState GetCurrentState() const;


	void SetVaultingState(VaultingState NewVaultState);

	/// <summary>
	/// Check if you can actually vault, and if so, set the output as the resulting position after vault
	/// </summary>
	/// <param name="OutFinalPosition">Position after performing vault</param>
	/// <returns>if can vault</returns>
	bool CanVault(FVector& OutFinalPosition) const;

	/// <summary>
	/// Add vaulting widget to viewport if argument is true, or remove it if false.
	/// Does nothing if already in the correct state
	/// </summary>
	/// <param name="Add">If true, will add it to viewport, otherwise it will remove it</param>
	void ModifyWidgetToViewport(bool Add);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
