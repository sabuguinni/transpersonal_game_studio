#include "Dinosaurs/TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATRexDinosaur::ATRexDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- T-Rex species stats (override DinosaurBase defaults) ---
    // These match the GDD: apex predator, high health, devastating melee
    MaxHealth           = 1200.f;
    CurrentHealth       = 1200.f;
    AttackDamage        = 150.f;   // Bite: one-shots unarmoured primitive human
    AttackRange         = 220.f;   // Short reach — forces close-quarters
    DetectionRadius     = 3500.f;  // Wide sight radius
    TerritoryRadius     = 3000.f;  // Large patrol zone
    MaxWalkSpeed        = 350.f;   // Lumbering walk
    MaxSprintSpeed      = 900.f;   // Fast sprint — player cannot outrun on foot
    HungerDecayRate     = 0.8f;    // Apex predator needs frequent meals
    XPReward            = 500;     // High XP for surviving a TRex encounter

    // Diet: pure carnivore — hunts player on detection
    Diet = EEng_DinosaurDiet::Carnivore;

    // Movement: heavy, poor turning
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed        = MaxWalkSpeed;
        MoveComp->RotationRate        = FRotator(0.f, 120.f, 0.f); // Slow turn
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->GravityScale        = 1.2f; // Feels heavy
        MoveComp->MaxStepHeight       = 80.f; // Can step over boulders
        MoveComp->JumpZVelocity       = 0.f;  // TRex cannot jump
    }
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // Start in Idle/Patrolling state — becomes aggressive on player detection
    SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
    unreal::log("TRex spawned and patrolling");
}

void ATRexDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle charge movement
    if (bIsCharging)
    {
        const float Now = GetWorld()->GetTimeSeconds();
        if (Now >= ChargeEndTime)
        {
            bIsCharging = false;
            // Restore normal sprint speed
            if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
            {
                MoveComp->MaxWalkSpeed = MaxSprintSpeed;
            }
        }
    }
}

void ATRexDinosaur::OnDetectPlayer(APawn* Player)
{
    // Call base class (sets behavior state to Hunting)
    Super::OnDetectPlayer(Player);

    // TRex-specific: roar on first detection
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastRoarTime >= RoarCooldown)
    {
        PerformRoar();
    }
}

void ATRexDinosaur::PerformRoar()
{
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastRoarTime < RoarCooldown)
    {
        return; // Still on cooldown
    }
    LastRoarTime = Now;

    // Apply fear to player if within roar radius
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        const float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
        if (DistToPlayer <= RoarFearRadius)
        {
            // Apply fear via damage event — SurvivalComponent listens for EEng_DamageType::Fear
            // Using point damage so SurvivalComponent can intercept it
            UGameplayStatics::ApplyPointDamage(
                PlayerPawn,
                RoarFearAmount,
                GetActorLocation(),
                FHitResult(),
                GetController(),
                this,
                nullptr
            );
        }
    }

    // Debug: draw roar sphere in editor
#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), RoarFearRadius, 16, FColor::Orange, false, 3.f);
#endif
}

void ATRexDinosaur::PerformStomp()
{
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastStompTime < StompCooldown)
    {
        return;
    }
    LastStompTime = Now;

    UWorld* World = GetWorld();
    if (!World) return;

    // AoE damage in stomp radius
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        World,
        StompDamage,
        GetActorLocation(),
        StompRadius,
        nullptr,
        IgnoredActors,
        this,
        GetController(),
        true // Full damage at center, falloff at edge
    );

#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), StompRadius, 12, FColor::Red, false, 2.f);
#endif
}

void ATRexDinosaur::StartChargeAttack(FVector TargetLocation)
{
    if (bIsCharging) return;

    bIsCharging = true;
    ChargeTargetLocation = TargetLocation;
    ChargeEndTime = GetWorld()->GetTimeSeconds() + ChargeDuration;

    // Boost movement speed for charge duration
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = MaxSprintSpeed * ChargeSpeedMultiplier;
    }
}
