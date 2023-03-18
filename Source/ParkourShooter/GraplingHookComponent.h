// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapleHook.h"
#include "GrapleCableActor.h"
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

	/// <summary>
	/// Where to start the grappling hook shoot given the local offset to the character
	/// </summary>
	/// <param name="LocalOffset"> Offset relative to the owning player to start shooting the hook </param>
	/// <returns> Location in world space where the hook should start at </returns>
	FVector GrapplingHookStartLocation(const FVector& LocalOffset) const;

	UFUNCTION()
	void OnHookHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnGrappleDestroyed(AActor* DestroyedActor);

	/// <summary>
	/// Direction we're currently traveling to 
	/// </summary>
	FVector FireDirection;

	/** How fast will the hook trable to its target */
	UPROPERTY(EditDefaultsOnly)
	float HookSpeed = 200;

	// Currently active hook in scene
	AGrapleHook* HookObject = nullptr;

	AGrapleCableActor* CableObject = nullptr;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	GrapplingState CurrentState;

	UPROPERTY(EditDefaultsOnly, Category = "Hook")
	TSubclassOf<AGrapleHook> GrapleHookClass;

	UPROPERTY(EditDefaultsOnly, Category = "Hook")
	TSubclassOf<AGrapleCableActor> GrapleCableClass;
};
