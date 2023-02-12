// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParkourShooterCharacter.h"
#include "ParkourShooterProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "VaultComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AParkourShooterCharacter

AParkourShooterCharacter::AParkourShooterCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Wallrun Initialization
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AParkourShooterCharacter::OnWallHit);
	CameraTiltTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraTiltTimeline"));
	MinimumWallrunSpeed = GetCharacterMovement()->GetMaxSpeed() / 2;

	// Vaulting
	VaultComponent = CreateDefaultSubobject<UVaultComponent>(TEXT("VaultingComponent"));

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AParkourShooterCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));


	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	// Wallrun Setup
	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	CameraTiltTrack.BindDynamic(this, &AParkourShooterCharacter::UpdateCameraTilt);

	if (CameraTiltCurve != nullptr)
	{
		CameraTiltTimeline->AddInterpFloat(CameraTiltCurve, CameraTiltTrack);
	}

}

bool AParkourShooterCharacter::IsVaulting() const
{
	return VaultComponent->IsVaulting();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AParkourShooterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AParkourShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AParkourShooterCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AParkourShooterCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AParkourShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AParkourShooterCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AParkourShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AParkourShooterCharacter::LookUpAtRate);
}

void AParkourShooterCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AParkourShooterProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AParkourShooterProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AParkourShooterCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AParkourShooterCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AParkourShooterCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AParkourShooterCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AParkourShooterCharacter::MoveForward(float Value)
{
	ForwardAxis = Value;
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AParkourShooterCharacter::MoveRight(float Value)
{
	RightAxis = Value;
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AParkourShooterCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AParkourShooterCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AParkourShooterCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AParkourShooterCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AParkourShooterCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AParkourShooterCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

void AParkourShooterCharacter::BeginWallrun()
{
	// Save previous value for gravity scale and air control
	OldAirControl = GetCharacterMovement()->AirControl;
	OldGravityScale = GetCharacterMovement()->GravityScale;

	GetCharacterMovement()->GravityScale = 0;
	GetCharacterMovement()->AirControl = 1;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector::UpVector);
	
	// Update State
	bIsWallRunning = true;

	// Start tilting camera
	BeginCameraTilt();
	// Start update wallrun times
	GetWorldTimerManager().SetTimer(WallrunTimerHandle, this, &AParkourShooterCharacter::UpdateWallrun, 0.1, true, 0);

	UE_LOG(LogTemp, Warning, TEXT("Starting wallrunning!"));
}

void AParkourShooterCharacter::UpdateWallrun()
{
	// Constantly get current side and direction

	if (!AreRequiredKeysDown(CurrentSide) || !IsFastEnoughToWallrun())
	{
		EndWallrun(WallrunEndReason::Fall); 
		return;
	}

	// Now we have to check if we're still wallrunning: We have to check if there's a wall to attach to
	FVector Direction;
	switch (CurrentSide)
	{
	case WallrunSide::Right:
		Direction = FVector::CrossProduct(WallrunDirection, FVector(0,0,-1));
		break;
	case WallrunSide::Left:
		Direction = FVector::CrossProduct(WallrunDirection, FVector(0,0,1));
		break;
	default:
		break;
	}

	FHitResult Hit;
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.AddIgnoredActor(this);
	bool HitSomething = GetWorld()->LineTraceSingleByChannel(
		Hit, 
		GetActorLocation(), 
		GetActorLocation() + 200 * Direction, 
		ECollisionChannel::ECC_Visibility,
		Params
	);


	if (!HitSomething || !CanRunInWall(Hit.ImpactNormal))
	{
		EndWallrun(WallrunEndReason::Fall);
		return;
	}

	// Now that we're still wall runninig, we have to update our direction
	FVector NewDirection;
	WallrunSide NewSide;

	FindWallrunDirectionAndSide(Hit.ImpactNormal, NewDirection, NewSide);

	if (NewSide != CurrentSide)
	{
		EndWallrun(WallrunEndReason::Fall);
		return;
	}

	WallrunDirection = NewDirection;
	FVector NewVelocity = WallrunDirection * GetCharacterMovement()->GetMaxSpeed();
	NewVelocity.Z = 0;
	GetCharacterMovement()->Velocity = NewVelocity;
}

void AParkourShooterCharacter::EndWallrun(WallrunEndReason Reason)
{
	GetWorldTimerManager().ClearTimer(WallrunTimerHandle);

	// Reset jumps accordinly to the reason you fell off the wall
	switch (Reason)
	{
	case WallrunEndReason::Fall:
		ResetJumps(JumpCurrentCount - 1);
		break;
	case WallrunEndReason::JumpOff:
		ResetJumps(1);
		break;
	default:
		break;
	}

	// Roll back changes we did starting the wallrun
	GetCharacterMovement()->GravityScale = OldGravityScale;
	GetCharacterMovement()->AirControl = OldAirControl;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector::ZeroVector);
	bIsWallRunning = false;
	EndCameraTilt();

	UE_LOG(LogTemp, Warning, TEXT("Ending Wallrun"));
}

void AParkourShooterCharacter::BeginCameraTilt()
{
	CameraTiltTimeline->Play();
}

void AParkourShooterCharacter::UpdateCameraTilt(float NewTilt)
{
	AController* MyController = GetController();
	if (MyController == nullptr)
		return;

	float TiltSide;
	switch (CurrentSide)
	{
	case WallrunSide::Right:
		TiltSide = -1;
		break;
	case WallrunSide::Left:
		TiltSide = 1;
		break;
	default:
		break;
	}

	FRotator NewRotation = MyController->GetControlRotation();
	NewRotation.Roll = TiltSide * NewTilt * MaxTiltAngle;

	MyController->SetControlRotation(NewRotation);
}

void AParkourShooterCharacter::EndCameraTilt()
{
	CameraTiltTimeline->Reverse();
}

void AParkourShooterCharacter::FindWallrunDirectionAndSide(const FVector& SurfaceNormal, FVector& OutDirection, WallrunSide& OutSide) const
{
	// Pick side, depends on the relationship between surface normal and forward vector
	WallrunSide NewSide;

	if (FVector::DotProduct(SurfaceNormal, GetActorRightVector()) > 0)
		NewSide = WallrunSide::Left;
	else
		NewSide = WallrunSide::Right;

	// Depending on side, we build a different resulting direction
	FVector NewDirection;
	switch (NewSide)
	{
	case WallrunSide::Right:
		NewDirection = FVector::CrossProduct(SurfaceNormal, FVector(0, 0, -1));
		break;
	case WallrunSide::Left:
		NewDirection = FVector::CrossProduct(SurfaceNormal, FVector(0, 0, 1));
		break;
	default:
		break;
	}



	OutSide = NewSide;
	OutDirection = NewDirection;
}

void AParkourShooterCharacter::ResetJumps(int NewJumps)
{
	JumpCurrentCount = FMath::Clamp(NewJumps, 0, JumpMaxCount);
}

bool AParkourShooterCharacter::ConsumeJump()
{
	if (IsOnWall())
	{
		return true;
	}
	else if (JumpCurrentCount < JumpMaxCount)
	{
		// JumpCurrentCount++;
		return true;
	}

	return false;
}

bool AParkourShooterCharacter::CanRunInWall(FVector SurfaceNormal) const
{

	// Check if this is the ceilling
	if (SurfaceNormal.Z < -0.05)
		return false;

	// Now we have to check surface angle. We can do it by taking the dot product between the normal 
	// vector and its own XY projection
	FVector Projection = FVector(SurfaceNormal.X, SurfaceNormal.Y, 0);
	Projection.Normalize();
	float FloorAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, Projection)));
	FloorAngle = 180 - FloorAngle - 90;
	
	return  !(GetCharacterMovement()->GetWalkableFloorAngle() >= FloorAngle || !IsFacingAwayEnoughFromWall(SurfaceNormal));
}

bool AParkourShooterCharacter::IsFacingAwayEnoughFromWall(const FVector& SurfaceNormal) const
{
	float AngleToWall = FVector2D::DotProduct(FVector2D(SurfaceNormal), FVector2D(GetActorForwardVector()));
	AngleToWall = FMath::RadiansToDegrees(FMath::Acos(AngleToWall)) - 90.f;

	return AngleToWall <= ToleranceDegreesToStartWallrun;
}

void AParkourShooterCharacter::OnWallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// We do nothing when:
	// - We're just running around in the ground
	// - We're already on the wall, nothing to do
	// - The surface can't even be runable, like a ceilling
	if (IsOnWall() || !GetCharacterMovement()->IsFalling() || !CanRunInWall(Hit.Normal) || !IsFastEnoughToWallrun())
		return;

	// Now that we know we hit a valid wall, we can start a new wallrun: We have to find out direction and side
	FVector NewDirection;
	WallrunSide NewSide;

	FindWallrunDirectionAndSide(Hit.Normal, NewDirection, NewSide);

	// Now we have to check if all required keys are down.
	if (!AreRequiredKeysDown(NewSide))
		return;

	WallrunDirection = NewDirection;
	CurrentSide = NewSide;

	BeginWallrun();
}

void AParkourShooterCharacter::Jump()
{
	FVector VaultPosition;
	if (!VaultComponent->CanVault(VaultPosition))
	{
		Super::Jump();

		// If couldn't jump, just end
		if (!ConsumeJump())
			return;

		LaunchCharacter(FindLaunchVelocity(), false, true);

		if (IsOnWall())
		{
			EndWallrun(WallrunEndReason::JumpOff);
		}

		return;
	}

	VaultComponent->BeginVault(VaultPosition);
}

void AParkourShooterCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	ResetJumps(0);
}

FVector AParkourShooterCharacter::FindLaunchVelocity() const
{

	FVector LaunchDirection(0,0,0);
	if (IsOnWall())
	{
		switch (CurrentSide)
		{
		case WallrunSide::Right:
			LaunchDirection = FVector::CrossProduct(WallrunDirection, FVector::UpVector);
			break;
		case WallrunSide::Left:
			LaunchDirection = FVector::CrossProduct(WallrunDirection, FVector::DownVector);
			break;
		default:
			break;
		}
	}
	else if (GetCharacterMovement()->IsFalling() && JumpCurrentCount < JumpMaxCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Direction is (%f, %f)"), RightAxis, ForwardAxis);
		LaunchDirection = GetActorRightVector() * RightAxis + GetActorForwardVector() * ForwardAxis;
	}
	return GetCharacterMovement()->JumpZVelocity * LaunchDirection + FVector::UpVector * GetCharacterMovement()->JumpZVelocity;
}

bool AParkourShooterCharacter::AreRequiredKeysDown(WallrunSide Side) const
{
	/// Minimum presure required to trigger run
	const float MIN_REQUIRED_PRESSURE = 0.1f;

	// To walrrun yo need to direct your character towards the wall

	switch (Side)
	{
	case WallrunSide::Right:
		return ForwardAxis > MIN_REQUIRED_PRESSURE && RightAxis > MIN_REQUIRED_PRESSURE;
	case WallrunSide::Left:
		return ForwardAxis > MIN_REQUIRED_PRESSURE && RightAxis < -MIN_REQUIRED_PRESSURE;
	default:
		break;
	}

	return false;
}

FVector2D AParkourShooterCharacter::GetHorizontalVelocity() const
{
	return FVector2D(GetCharacterMovement()->Velocity);
}

void AParkourShooterCharacter::SetHorizontalVelocity(FVector2D NewVelocity)
{
	FVector Velocity = GetCharacterMovement()->Velocity;
	Velocity.X = NewVelocity.X;
	Velocity.Y = NewVelocity.Y;
	GetCharacterMovement()->Velocity = Velocity;
}

void AParkourShooterCharacter::ClampHorizontalVelocity()
{
	if (GetCharacterMovement()->IsFalling())
	{
		FVector2D HorizontalVelocity = GetHorizontalVelocity();
		float MaxSpeed = GetCharacterMovement()->GetMaxSpeed();
		if (HorizontalVelocity.SizeSquared() > MaxSpeed * MaxSpeed)
		{
			HorizontalVelocity.Normalize();
			HorizontalVelocity *= MaxSpeed;
		}

		SetHorizontalVelocity(HorizontalVelocity);
	}
}

void AParkourShooterCharacter::Tick(float DeltaSeconds)
{
	ClampHorizontalVelocity();
}

bool AParkourShooterCharacter::IsFastEnoughToWallrun() const
{
	float SquaredSpeed = FVector2D(GetCharacterMovement()->Velocity).SizeSquared();
	UE_LOG(LogTemp, Warning, TEXT("Current Speed: %f, Minimum Speed: %f"), FMath::Sqrt(SquaredSpeed), MinimumWallrunSpeed);
	return SquaredSpeed >= MinimumWallrunSpeed * MinimumWallrunSpeed;
}
