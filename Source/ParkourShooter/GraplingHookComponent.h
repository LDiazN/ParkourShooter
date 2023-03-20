// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapleHook.h"
#include "GrapleCableActor.h"
#include "GraplingHookComponent.generated.h"

class UCharacterMovementComponent;

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

	struct MovementProperties
	{
		float GroundFriction;
		float GravityScale;
		float AirControl;
	};

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
	/// Set movement properties of this movement component so they're better fit for pulling to 
	/// attach point
	/// </summary>
	/// <param name="MovementComponent"> Movemenet component to modify </param>
	void SetMovementProperties(UCharacterMovementComponent* MovementComponent, const MovementProperties& Properties) const;
	
	/// <summary>
	/// Get *relevant* movement properties from movement component
	/// </summary>
	/// <returns>Relevant properties of this movement component</returns>
	MovementProperties GetMovementProperties(UCharacterMovementComponent* MovementComponent) const;

	/// <summary>
	/// Return a unit vector pointing from the character to the grapple hook head. Returns 0 if no 
	/// valid HookObject is currently active
	/// </summary>
	/// <returns>Unit vector pointing from character to hook. Zero vector if no hook in scene</returns>
	FVector ToGrappleHook() const;

	/// <summary>
	/// Checks if too close to hook and should cancel pulling
	/// </summary>
	/// <returns> If too close to hook </returns>
	bool IsTooCloseToHook() const;

	/// <summary>
	/// Checks if the character is too far from the hook.
	/// </summary>
	/// <returns> If too far from hook </returns>
	bool IsTooFarFromHook() const;

	/// <summary>
	/// Checks if hook is already passed, like behinde the character
	/// </summary>
	/// <returns> If you already passed the hook </returns>
	bool HookPassed() const;

	/// <summary>
	/// Direction we're currently traveling to 
	/// </summary>
	FVector FireDirection;

	/** How fast will the hook travel to its target */
	UPROPERTY(EditAnywhere, Category = "Hook")
	float HookSpeed = 200;

	/** Initial speed to pull the character towards the hook the first time the hook hits a wall */
	UPROPERTY(EditAnywhere, Category = "Hook")
	float PullInitialSpeed = 2000;

	/** Force to apply to character every frame towards the hook */
	UPROPERTY(EditAnywhere, Category = "Hook")
	float ContinousPullSpeed = 100000;

	/** Radius around the hook to cancel the pull. If the character is in a sphere of this radius around the hook, cancel attachement */
	UPROPERTY(EditAnywhere, Category = "Hook")
	float MinDistanceToPull = 100;

	UPROPERTY(EditAnywhere, Category = "Hook")
	float MaxHookDistanceFromCharacter = 100000;

	// Direction we started to pull to. We don't care about the Z component,
	// we only care about the direction in the XY plane.
	FVector2D InitialHookDirection2D;

	// Currently active hook in scene
	AGrapleHook* HookObject = nullptr;

	// Currently active cable in scene
	AGrapleCableActor* CableObject = nullptr;

	// Movement properties before taking the player off the ground with the graple hook
	MovementProperties PreviousProperties;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	GrapplingState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hook")
	TSubclassOf<AGrapleHook> HookClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hook")
	TSubclassOf<AGrapleCableActor> CableClass;
};
