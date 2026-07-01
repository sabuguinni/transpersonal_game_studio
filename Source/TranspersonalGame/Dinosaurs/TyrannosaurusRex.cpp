// TyrannosaurusRex.cpp
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260701_010
// Full implementation of ATyrannosaurusRex

#include "Dinosaurs/TyrannosaurusRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // Apply TRex-specific defaults immediately in constructor
    ApplySpeciesDefaults();
}

void ATyrannosaurusRex::ApplySpeciesDefaults()
{
    // Override base class species
    Species = EDinosaurSpecies::TRex;

    // Physical scale — massive apex predator
    GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // Stats
    MaxHealth = 2500.0f;
    CurrentHealth = 2500.0f;
    MaxHunger = 300.0f;
    CurrentHunger = 150.0f;
    AttackDamage = 180.0f;
    AttackRange = 350.0f;
    AggressionLevel = 0.95f;  // Near-maximum aggression

    // Movement — slow but powerful
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 700.0f;
        GetCharacterMovement()->MaxAcceleration = 800.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 600.0f;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->GravityScale = 1.2f;  // Heavier gravity feel
    }

    // Perception — large detection radius for apex predator
    DetectionRadius = 2000.0f;
    AttackRadius = 350.0f;

    // Territorial — does not flee
    bIsPredator = true;
}

void ATyrannosaurusRex::BeginPlay()
{
    Super::BeginPlay();

    // TRex starts in Wandering state, transitions to Hunting when prey detected
    CurrentBehavior = EDinosaurBehavior::Wandering;

    UE_LOG(LogTemp, Log, TEXT("ATyrannosaurusRex [%s] spawned — Health: %.0f, Aggression: %.2f"),
        *GetName(), CurrentHealth, AggressionLevel);
}

void ATyrannosaurusRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Charge logic — accelerate toward target during charge
    if (bIsCharging)
    {
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;
        }
    }
    else
    {
        // Restore normal speed when not charging
        if (GetCharacterMovement() && GetCharacterMovement()->MaxWalkSpeed == ChargeSpeed)
        {
            GetCharacterMovement()->MaxWalkSpeed = 700.0f;
        }
    }
}

float ATyrannosaurusRex::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    // TRex takes 20% less damage — thick hide
    float ModifiedDamage = DamageAmount * 0.80f;
    float ActualDamage = Super::TakeDamage(ModifiedDamage, DamageEvent, EventInstigator, DamageCauser);

    // Roar when hit — intimidation response
    if (CurrentHealth > 0.0f && bRoarAvailable)
    {
        PerformRoar();
    }

    // Trigger charge if attacker is within range and TRex is healthy
    if (DamageCauser && CurrentHealth > MaxHealth * 0.3f && !bIsCharging)
    {
        float DistToAttacker = FVector::Dist(GetActorLocation(), DamageCauser->GetActorLocation());
        if (DistToAttacker < ChargeActivationDistance)
        {
            BeginChargeAttack(DamageCauser->GetActorLocation());
        }
    }

    return ActualDamage;
}

void ATyrannosaurusRex::PerformRoar()
{
    if (!bRoarAvailable) return;

    bRoarAvailable = false;

    UE_LOG(LogTemp, Log, TEXT("ATyrannosaurusRex [%s] ROARS — radius: %.0f"), *GetName(), RoarRadius);

    // Apply slow debuff to all actors within RoarRadius
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

        for (AActor* Actor : NearbyActors)
        {
            if (Actor == this) continue;

            float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Dist <= RoarRadius)
            {
                // Apply roar stun — reduce movement speed via damage event
                // In full implementation: apply GameplayEffect "Stunned" for 3s
                // For now: log affected actors
                UE_LOG(LogTemp, Log, TEXT("  Roar affects: %s (dist: %.0f)"), *Actor->GetName(), Dist);
            }
        }

#if WITH_EDITOR
        // Debug sphere in editor
        DrawDebugSphere(World, GetActorLocation(), RoarRadius, 16, FColor::Orange, false, 3.0f);
#endif
    }

    // Start cooldown timer
    World->GetTimerManager().SetTimer(RoarCooldownTimer, this,
        &ATyrannosaurusRex::ResetRoarCooldown, RoarCooldown, false);
}

void ATyrannosaurusRex::BeginChargeAttack(const FVector& TargetLocation)
{
    if (bIsCharging) return;

    bIsCharging = true;
    CurrentBehavior = EDinosaurBehavior::Attacking;

    UE_LOG(LogTemp, Log, TEXT("ATyrannosaurusRex [%s] CHARGING toward (%.0f, %.0f, %.0f)"),
        *GetName(), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);

    // Boost speed for charge
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;
    }

    // End charge after 3 seconds
    FTimerHandle ChargeTimer;
    GetWorld()->GetTimerManager().SetTimer(ChargeTimer, this,
        &ATyrannosaurusRex::EndChargeAttack, 3.0f, false);
}

void ATyrannosaurusRex::EndChargeAttack()
{
    bIsCharging = false;
    CurrentBehavior = EDinosaurBehavior::Hunting;

    // Restore normal speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 700.0f;
    }

    UE_LOG(LogTemp, Log, TEXT("ATyrannosaurusRex [%s] charge ended"), *GetName());
}

void ATyrannosaurusRex::ResetRoarCooldown()
{
    bRoarAvailable = true;
    UE_LOG(LogTemp, Log, TEXT("ATyrannosaurusRex [%s] roar ready"), *GetName());
}
