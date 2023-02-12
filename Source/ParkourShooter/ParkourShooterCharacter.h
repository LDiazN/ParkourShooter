// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Components/TimelineComponent.h"
#include "ParkourShooterCharacter.generated.h"

class UInputComponent;
class UVaultComponent;

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

	UFUNCTION()
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

	// Use this variables to save and restore state we're changing when 
	// starting a wall run
	float OldGravityScale;
	float OldAirControl;

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

