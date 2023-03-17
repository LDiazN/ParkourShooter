// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapleHook.generated.h"

class UProjectileMovementComponent;

UCLASS()
class PARKOURSHOOTER_API AGrapleHook : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrapleHook();
	void SetVelocity(FVector NewVelocity) { HookVelocity = NewVelocity; }
	FVector GetVelocity() const { return HookVelocity; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly)
	UProjectileMovementComponent* ProjectileMovementComponent;

	FVector HookVelocity;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
