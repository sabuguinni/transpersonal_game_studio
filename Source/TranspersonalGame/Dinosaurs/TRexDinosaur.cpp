// TRexDinosaur.cpp
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Tyrannosaurus Rex implementation

#include "Dinosaurs/TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATRexDinosaur::ATRexDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // TRex stats — apex predator
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    AttackDamage = 80.0f;
    AttackRange = 200.0f;
    DetectionRange = 3000.0f;
    bIsAggressive = true;
    DinoSpeciesName = TEXT("Tyrannosaurus Rex");

    // Locomotion — powerful but not fastest
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 600.0f;
        GetCharacterMovement()->MaxAcceleration = 800.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 600.0f;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
    }

    // Roar ability config
    RoarRadius = 1500.0f;
    RoarFearAmount = 40.0f;
    RoarCooldownDuration = 15.0f;
    RoarCooldownRemaining = 0.0f;

    // Stomp ability config
    StompRadius = 300.0f;
    StompDamage = 50.0f;
    StompCooldownDuration = 8.0f;
    StompCooldownRemaining = 0.0f;

    // Territory
    TerritoryCenter = FVector::ZeroVector;
    TerritoryRadius = 5000.0f;
    bIsPatrolling = false;
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // Record spawn location as territory center
    TerritoryCenter = GetActorLocation();
    bIsPatrolling = true;

    UE_LOG(LogTemp, Log, TEXT("TRexDinosaur BeginPlay — territory center: %s"), *TerritoryCenter.ToString());
}

void ATRexDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick down cooldowns
    if (RoarCooldownRemaining > 0.0f)
    {
        RoarCooldownRemaining = FMath::Max(0.0f, RoarCooldownRemaining - DeltaTime);
    }
    if (StompCooldownRemaining > 0.0f)
    {
        StompCooldownRemaining = FMath::Max(0.0f, StompCooldownRemaining - DeltaTime);
    }
}

void ATRexDinosaur::PerformRoar()
{
    if (RoarCooldownRemaining > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("TRex Roar on cooldown: %.1fs remaining"), RoarCooldownRemaining);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Log, TEXT("TRex ROAR — radius: %.0f, fear: %.0f"), RoarRadius, RoarFearAmount);

    // Find all actors within roar radius
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), OverlappingActors);

    int32 FrightenedCount = 0;
    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= RoarRadius)
        {
            // Apply fear damage (fear type) to prey actors
            UGameplayStatics::ApplyDamage(Actor, RoarFearAmount * 0.1f, GetController(), this, nullptr);
            FrightenedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TRex Roar frightened %d actors"), FrightenedCount);

    // Start cooldown
    RoarCooldownRemaining = RoarCooldownDuration;
    GetWorldTimerManager().SetTimer(RoarCooldownTimer, this, &ATRexDinosaur::ResetRoarCooldown, RoarCooldownDuration, false);

#if WITH_EDITOR
    // Debug visualization
    DrawDebugSphere(World, GetActorLocation(), RoarRadius, 16, FColor::Orange, false, 3.0f);
#endif
}

void ATRexDinosaur::PerformStomp()
{
    if (StompCooldownRemaining > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("TRex Stomp on cooldown: %.1fs remaining"), StompCooldownRemaining);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Log, TEXT("TRex STOMP — radius: %.0f, damage: %.0f"), StompRadius, StompDamage);

    // AoE damage around feet
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), OverlappingActors);

    int32 HitCount = 0;
    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= StompRadius)
        {
            UGameplayStatics::ApplyDamage(Actor, StompDamage, GetController(), this, nullptr);
            HitCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TRex Stomp hit %d actors"), HitCount);

    // Start cooldown
    StompCooldownRemaining = StompCooldownDuration;
    GetWorldTimerManager().SetTimer(StompCooldownTimer, this, &ATRexDinosaur::ResetStompCooldown, StompCooldownDuration, false);

#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), StompRadius, 12, FColor::Red, false, 2.0f);
#endif
}

void ATRexDinosaur::OnPlayerDetected_Implementation(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    UE_LOG(LogTemp, Warning, TEXT("TRex detected player: %s — initiating aggressive chase!"), *PlayerActor->GetName());

    // Trigger roar when player first detected (if off cooldown)
    if (RoarCooldownRemaining <= 0.0f)
    {
        PerformRoar();
    }

    // Force behavior state to Chasing — handled by base class AI
    bIsPatrolling = false;
}

void ATRexDinosaur::ResetRoarCooldown()
{
    RoarCooldownRemaining = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("TRex Roar cooldown reset — ready"));
}

void ATRexDinosaur::ResetStompCooldown()
{
    StompCooldownRemaining = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("TRex Stomp cooldown reset — ready"));
}
