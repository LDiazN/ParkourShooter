// Fill out your copyright notice in the Description page of Project Settings.

#include "GrapleHook.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AGrapleHook::AGrapleHook()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetEnableGravity(false);
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	RootComponent = MeshComponent;
}

// Called when the game starts or when spawned
void AGrapleHook::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrapleHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ProjectileMovementComponent->Velocity = HookVelocity;
}

