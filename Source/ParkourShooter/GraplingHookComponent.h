// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapleHook.h"
#include "GraplingHookComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARKOURSHOOTER_API UGraplingHookComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGraplingHookComponent();

	enum class GrapplingState
	{
		ReadyToFire,
		Firing,
		Attached
	};

	/// <summary>
	/// Get the current state
	/// </summary>
	/// <returns> Current state </returns>
	GrapplingState GetState() const { return CurrentState; }

	/// <summary>
	/// Checks if the grappling hook is currently being used
	/// </summary>
	/// <returns> true if current state is firing or attached </returns>
	bool IsInUse() const { return CurrentState == GrapplingState::Firing || CurrentState == GrapplingState::Attached; }

	void FireGrapple(const FVector& Target, const FVector& LocalOffset);

	void CancelGrapple();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	/// <summary>
	/// Get direction to move given the target position
	/// </summary>
	/// <param name="Target">Position to move to</param>
	/// <returns> Direction to move along to </returns>
	FVector GetMovementDirection(const FVector& Target, const FVector& LocalOffset) const;

	FVector GrapplingHookStartLocation(const FVector& LocalOffset) const;

	/// <summary>
	/// Direction we're currently traveling to 
	/// </summary>
	FVector FireDirection;

	UPROPERTY(EditDefaultsOnly)
	float HookSpeed = 100;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	GrapplingState CurrentState;

	UPROPERTY(EditDefaultsOnly, Category = "Hook")
	TSubclassOf<AGrapleHook> GrapleHookClass;
};
