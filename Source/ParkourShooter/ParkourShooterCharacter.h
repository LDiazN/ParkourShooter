// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Components/TimelineComponent.h"
#include "ParkourShooterCharacter.generated.h"

class UInputComponent;
class UVaultComponent;
class UGraplingHookComponent;

UENUM()
enum  MovementState
{
	Walking UMETA(DisplayName = "Walking"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Crouching UMETA(DisplayName = "Crouching"),
	Sliding UMETA(DisplayName = "Sliding")
};

UCLASS(config=Game)
class AParkourShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	AParkourShooterCharacter();

protected:

	virtual void BeginPlay();

	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl;

	struct MovementProperties
	{
		float GravityScale;
		float AirControl;
		float GroundFriction;
		float BrakingDeceleration;
	};

	MovementProperties OriginalProperties;

	// -- < Sliding > --------------------------------------------------------------------

protected:


	MovementState CurrentMovementState = MovementState::Walking;
	bool IsCrouchKeyDown;
	bool IsSprintKeyDown = true; // If we choose to have a sprint button, this might be more helpful. For now it will be always true

	/**Max speed when in walking state. Remember that you are almost always RUNNING instead of walking*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxWalkSpeed = 800;

	/**Max speed when in crouching state. Remember that you are almost always RUNNING and crounching only happens under a small ceiling*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxCrouchSpeed = 400;

	/**Max speed when in sliding state. Note that sliding might be affected by slope of surface, so this is MAX speed, not THE speed*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSlideSpeed = 1600;

	/**Max speed when sprinting. This will be your state most of the time*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSprintSpeed = 1200;

	float StandingHalfHeight;
	float StandingCameraZOffset;

	/** Half Height of player capsule on crouch */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float CrouchHalfHeight = 45;

	/** Height of camera when crouching */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float CrouchCameraZOffset = 25;

	UPROPERTY(EditAnywhere, Category = "Slide")
	float FloorInfluenceForce = 500000;

	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float MinBrakingDecelerationOnSlide = 1000;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float MinFrictionOnSlide = 1;

	/** Force applied to you the moment you start a slide */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float ForwardSlideInitialPush = 800;

	void Slide();
	void SlideRelease();
	void Sprint();
	void SprintRelease();

	UFUNCTION(BlueprintImplementableEvent, Category = "Slide")
	void BeginSlideBP();

	void BeginSlide();

	UFUNCTION(BlueprintCallable, Category = "Slide")
	void UpdateSlide();

	UFUNCTION(BlueprintImplementableEvent, Category = "Slide")
	void EndSlideBP();

	void EndSlide();

	UFUNCTION(BlueprintImplementableEvent)
	void BeginCrouch();

	UFUNCTION(BlueprintCallable)
	void UpdateCrouch(float Progress);

	UFUNCTION(BlueprintImplementableEvent)
	void EndCrouch();

	MovementState ResolveMovementState() const;
	void SetMovementState(MovementState NewState);
	void OnMovementStateChanged(MovementState OldState, MovementState NewState);

	// Compute floor direction downwards
	FVector ComputeFloorInfluence(FVector FloorNormal) const;
	bool CanSprint() const;
	bool CanStand() const;
	bool GetSprintKeyDown() const { return true; }
	bool GetCrouchKeyDown() const { return IsCrouchKeyDown; }

	UFUNCTION(BlueprintCallable)
	MovementState GetMovementState() const { return CurrentMovementState; }

	// -- < End of Sliding > -------------------------------------------------------------
	// -- < Vaulting > -------------------------------------------------------------------
	// Vaulting is like grabbing on ledges to jump over things
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting")
	UVaultComponent* VaultComponent;

	/// <summary>
	/// Vault function called when holding the jump button in the air
	/// </summary>
	virtual void VaultOnHold();

public:
	UFUNCTION(BlueprintCallable)
	bool IsVaulting() const;

	// -- < End Vaulting > ---------------------------------------------------------------

	// -- < WALLRUN > --------------------------------------------------------------------

protected:
	enum class WallrunSide
	{
		Right, 
		Left
	};

	enum WallrunEndReason
	{
		Fall, 
		JumpOff
	};

	UFUNCTION(BlueprintPure)
	int32 JumpCount() const { return JumpCurrentCount; }

	bool IsOnWall() const { return bIsWallRunning; };

	void BeginWallrun();

	UFUNCTION()
	void UpdateWallrun();

	void EndWallrun(WallrunEndReason Reason);

	void BeginCameraTilt();

	UFUNCTION(BlueprintCallable)
	void UpdateCameraTilt(float NewTilt);

	void EndCameraTilt();

	void FindWallrunDirectionAndSide(const FVector& SurfaceNormal, FVector& OutDirection, WallrunSide& OutSide) const;

	void ResetJumps(int NewJumps);

	bool ConsumeJump();

	bool CanRunInWall(FVector SurfaceNormal) const;

	/// <summary>
	/// Checks if the character is facing away from the wall. You don't want to start 
	/// wallrunning when you're running straight to the wall (your forward vector and normal surface would be anti parallel)
	/// </summary>
	/// <param name="SurfaceNormal">Normal of wall you want to check if your facing away</param>
	/// <returns></returns>
	bool IsFacingAwayEnoughFromWall(const FVector& SurfaceNormal) const;

	//void OnLanded();
	UFUNCTION()
	void OnWallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void Jump() override;

	virtual void Landed(const FHitResult& Hit) override;

	FVector FindLaunchVelocity() const;

	bool AreRequiredKeysDown(WallrunSide Side) const;

	FVector2D GetHorizontalVelocity() const;

	void SetHorizontalVelocity(FVector2D NewVelocity);
	
	void ClampHorizontalVelocity();

	virtual void Tick(float DeltaSeconds) override;

	bool IsFastEnoughToWallrun() const;

	WallrunSide CurrentSide;
	bool bIsWallRunning;
	float ForwardAxis, RightAxis;
	FVector WallrunDirection;

	/**This is the maximum angle between wall and forward vector to accept to start a wallrun*/
	UPROPERTY(EditDefaultsOnly, Category = "Wallrun")
	float ToleranceDegreesToStartWallrun = 45;

	// Timer to manage wallrun update
	FTimerHandle WallrunTimerHandle;

	// Data to manage timeline for tilting camera
	UPROPERTY(EditDefaultsOnly, Category = "Wallrun")
	UCurveFloat* CameraTiltCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UTimelineComponent* CameraTiltTimeline;

	// Track used to update camera tilting
	FOnTimelineFloat CameraTiltTrack;

	/** Max angle to tilt the camera when doing wallrun */
	UPROPERTY(EditDefaultsOnly, Category = "Wallrun")
	float MaxTiltAngle = 30;

	/** Minimal wallrun horizontal speed, defaults to half of max walk speed */
	UPROPERTY(EditAnywhere, Category = "Wallrun")
	float MinimumWallrunSpeed;

	// -- < END WALLRUN > ----------------------------------------------------------------

	// -- < BEGIN GRAPPLING HOOK > -------------------------------------------------------
protected:
	
	/** Fires grappling hook */
	void ShootGrapplingHook();
	void CancelGrapplingHook();


	UPROPERTY(EditDefaultsOnly, Category = "Grappling Hook")
	UGraplingHookComponent* GrapplingHook;

	UPROPERTY(EditAnywhere, Category = "Grappling Hook")
	float MaxHookReachDistance = 100000.f;

	/** 
	Where, relative to the player, to spawn the grappling hook
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Grappling Hook")
	USceneComponent* GrapplingHookSpawnPoint;

	// -- < END GRAPPLING HOOK  > ----------------------------------------------------------------


public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AParkourShooterProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

	/// <summary>
	/// This method is used to restore properties like friction and gravity that are changed by actions like grappling hook
	/// or wallrunning.
	/// </summary>
	void RestoreMovementProperties();

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

