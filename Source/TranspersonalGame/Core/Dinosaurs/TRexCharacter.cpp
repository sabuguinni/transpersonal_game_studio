// TRexCharacter.cpp
// T-Rex species implementation — territorial apex predator
// Core Systems Programmer — Agent #3

#include "TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // T-Rex physical dimensions — based on Tyrannosaurus rex paleontology
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 200.0f);

    // Movement — T-Rex was a pursuit predator, not a sprinter
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 500.0f;          // ~18 km/h walk
        MoveComp->MaxWalkSpeedCrouched = 200.0f;
        MoveComp->JumpZVelocity = 0.0f;           // T-Rex cannot jump
        MoveComp->GravityScale = 1.2f;
        MoveComp->Mass = 8000.0f;                 // ~8 tonnes
        MoveComp->bCanWalkOffLedges = true;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
    }

    // Species identity
    SpeciesName = FName("Tyrannosaurus_Rex");
    TerritorialRadius = 5000.0f;   // 50m radius territory
    DetectionRadius = 3000.0f;     // 30m detection range
    AttackRange = 250.0f;          // 2.5m bite range
    BaseDamage = 85.0f;            // Massive bite force
    MaxHealth = 1500.0f;
    CurrentHealth = 1500.0f;
    bIsPredator = true;
    bIsPackAnimal = false;
    HungerDrainRate = 0.5f;        // Slow metabolism — large body
    CurrentHunger = 100.0f;
    MaxHunger = 100.0f;

    // AI state
    CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;
    bIsAlerted = false;
    bIsTerritoryOwner = true;
    AlertCooldown = 10.0f;
    AttackCooldown = 3.0f;
    LastAttackTime = -999.0f;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Record home territory center
    TerritoryCenter = GetActorLocation();

    // Start hunger drain timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ATRexCharacter::TickHunger,
        5.0f,   // every 5 seconds
        true
    );

    // Start behavior update timer
    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &ATRexCharacter::UpdateBehavior,
        1.0f,   // every 1 second
        true
    );

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] spawned at territory center (%.0f, %.0f, %.0f)"),
        *GetActorLabel(),
        TerritoryCenter.X, TerritoryCenter.Y, TerritoryCenter.Z);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Roar cooldown
    if (bIsAlerted)
    {
        AlertCooldown -= DeltaTime;
        if (AlertCooldown <= 0.0f)
        {
            bIsAlerted = false;
            AlertCooldown = 10.0f;
        }
    }
}

void ATRexCharacter::TickHunger()
{
    CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDrainRate);

    if (CurrentHunger <= 20.0f && CurrentBehaviorState == ECore_DinosaurBehavior::Patrolling)
    {
        // Hungry — switch to active hunting
        CurrentBehaviorState = ECore_DinosaurBehavior::Hunting;
        UE_LOG(LogTemp, Log, TEXT("TRex [%s] is HUNGRY — switching to hunting behavior"), *GetActorLabel());
    }
}

void ATRexCharacter::UpdateBehavior()
{
    if (!GetWorld()) return;

    AActor* NearestPrey = FindNearestPrey();

    switch (CurrentBehaviorState)
    {
        case ECore_DinosaurBehavior::Patrolling:
            ExecutePatrol();
            if (NearestPrey)
            {
                float Dist = FVector::Dist(GetActorLocation(), NearestPrey->GetActorLocation());
                if (Dist <= DetectionRadius)
                {
                    CurrentBehaviorState = ECore_DinosaurBehavior::Chasing;
                    CurrentTarget = NearestPrey;
                    OnAlerted();
                }
            }
            break;

        case ECore_DinosaurBehavior::Hunting:
            if (NearestPrey)
            {
                CurrentTarget = NearestPrey;
                CurrentBehaviorState = ECore_DinosaurBehavior::Chasing;
            }
            else
            {
                ExecutePatrol();
            }
            break;

        case ECore_DinosaurBehavior::Chasing:
            if (CurrentTarget && !CurrentTarget->IsPendingKill())
            {
                float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
                if (Dist <= AttackRange)
                {
                    CurrentBehaviorState = ECore_DinosaurBehavior::Attacking;
                }
                else if (Dist > TerritorialRadius * 1.5f)
                {
                    // Lost prey — return to territory
                    CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;
                    CurrentTarget = nullptr;
                }
            }
            else
            {
                CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;
                CurrentTarget = nullptr;
            }
            break;

        case ECore_DinosaurBehavior::Attacking:
            if (CurrentTarget && !CurrentTarget->IsPendingKill())
            {
                float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
                if (Dist <= AttackRange)
                {
                    ExecuteAttack(CurrentTarget);
                }
                else
                {
                    CurrentBehaviorState = ECore_DinosaurBehavior::Chasing;
                }
            }
            else
            {
                CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;
                CurrentTarget = nullptr;
                // Fed — restore hunger partially
                CurrentHunger = FMath::Min(MaxHunger, CurrentHunger + 40.0f);
            }
            break;

        case ECore_DinosaurBehavior::Fleeing:
            // T-Rex doesn't flee — revert to patrol after a moment
            CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;
            break;

        case ECore_DinosaurBehavior::Resting:
            // Resting — restore health slowly
            CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + 5.0f);
            if (CurrentHunger > 50.0f)
            {
                CurrentBehaviorState = ECore_DinosaurBehavior::Patrolling;
            }
            break;

        default:
            break;
    }
}

void ATRexCharacter::ExecutePatrol()
{
    // Simple territorial patrol — move within TerritorialRadius of home
    float DistFromHome = FVector::Dist(GetActorLocation(), TerritoryCenter);
    if (DistFromHome > TerritorialRadius)
    {
        // Return toward territory center
        FVector DirectionHome = (TerritoryCenter - GetActorLocation()).GetSafeNormal();
        AddMovementInput(DirectionHome, 0.6f);
    }
    else
    {
        // Random wander within territory
        if (FMath::RandBool())
        {
            FVector RandomDir = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                0.0f
            ).GetSafeNormal();
            AddMovementInput(RandomDir, 0.4f);
        }
    }
}

void ATRexCharacter::ExecuteAttack(AActor* Target)
{
    if (!Target) return;

    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastAttackTime < AttackCooldown) return;

    LastAttackTime = Now;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        Target,
        BaseDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] BITES [%s] for %.0f damage"),
        *GetActorLabel(),
        *Target->GetActorLabel(),
        BaseDamage);
}

AActor* ATRexCharacter::FindNearestPrey() const
{
    if (!GetWorld()) return nullptr;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);

    AActor* Nearest = nullptr;
    float NearestDist = DetectionRadius;

    for (AActor* Actor : AllActors)
    {
        if (Actor == this) continue;

        // Skip other T-Rex (same species)
        if (Actor->IsA(ATRexCharacter::StaticClass())) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            Nearest = Actor;
        }
    }

    return Nearest;
}

void ATRexCharacter::OnAlerted()
{
    bIsAlerted = true;
    AlertCooldown = 10.0f;

    UE_LOG(LogTemp, Warning, TEXT("TRex [%s] ALERTED — target detected!"), *GetActorLabel());

    // Territorial roar — broadcast to nearby T-Rex (none in this species, but good for future)
    // Future: trigger roar sound via Audio component
}

float ATRexCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] took %.0f damage — health: %.0f/%.0f"),
        *GetActorLabel(), ActualDamage, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (CurrentHealth < MaxHealth * 0.3f)
    {
        // Wounded — T-Rex becomes more aggressive
        BaseDamage *= 1.2f;
        GetCharacterMovement()->MaxWalkSpeed = 600.0f; // Adrenaline burst
    }

    return ActualDamage;
}

void ATRexCharacter::OnDeath()
{
    UE_LOG(LogTemp, Warning, TEXT("TRex [%s] has DIED"), *GetActorLabel());

    // Stop all timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    // Disable movement
    GetCharacterMovement()->DisableMovement();

    // Disable collision (ragdoll would be triggered here in full implementation)
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Destroy after delay (carcass lingers for scavengers)
    SetLifeSpan(60.0f);
}
