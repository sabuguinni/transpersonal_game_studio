// DinosaurBase.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260630_004
// Dinosaur base class with collision, movement, and survival stats

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root capsule for collision — prevents player walking through dinosaurs
    GetCapsuleComponent()->SetCapsuleHalfHeight(150.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Body mesh component
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(GetCapsuleComponent());
    BodyMesh->SetCollisionProfileName(TEXT("BlockAll"));
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Head mesh component
    HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
    HeadMesh->SetupAttachment(BodyMesh);
    HeadMesh->SetCollisionProfileName(TEXT("BlockAll"));

    // Default survival stats
    Health = 100.0f;
    MaxHealth = 100.0f;
    Hunger = 80.0f;
    MaxHunger = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;

    // Default species traits
    Species = EDinosaurSpecies::Raptor;
    DietType = EDinosaurDiet::Carnivore;
    TerritoryRadius = 2000.0f;
    DetectionRadius = 1500.0f;
    AttackRange = 200.0f;
    AttackDamage = 25.0f;
    MoveSpeed = 600.0f;
    bIsAggressive = false;
    bIsInPack = false;
    PackSize = 1;

    // Movement component defaults
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

    CurrentBehaviorState = EDinosaurBehavior::Idle;
    HomeLocation = FVector::ZeroVector;
    bHomeLocationSet = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record spawn location as home territory center
    HomeLocation = GetActorLocation();
    bHomeLocationSet = true;

    // Start hunger drain timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::DrainHunger,
        5.0f,   // Every 5 seconds
        true    // Looping
    );

    // Start behavior update timer
    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &ADinosaurBase::UpdateBehavior,
        2.0f,   // Every 2 seconds
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina regeneration when not sprinting
    if (Stamina < MaxStamina)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + (DeltaTime * 10.0f));
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Health = FMath::Max(0.0f, Health - ActualDamage);

    if (Health <= 0.0f)
    {
        OnDeath();
    }
    else
    {
        // Become aggressive when attacked
        if (!bIsAggressive && DamageCauser)
        {
            bIsAggressive = true;
            CurrentBehaviorState = EDinosaurBehavior::Attacking;
        }
    }

    return ActualDamage;
}

void ADinosaurBase::OnDeath()
{
    // Disable collision on death
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (BodyMesh)
    {
        BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Ragdoll placeholder — full physics ragdoll will be implemented by Core Systems #03
    if (BodyMesh)
    {
        BodyMesh->SetSimulatePhysics(true);
    }

    // Clear timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    // Destroy after 30 seconds (allow player to loot)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::DrainHunger()
{
    Hunger = FMath::Max(0.0f, Hunger - 2.0f);

    // Hungry dinosaurs become more aggressive hunters
    if (Hunger < 20.0f && DietType == EDinosaurDiet::Carnivore)
    {
        bIsAggressive = true;
        DetectionRadius = DetectionRadius * 1.5f;  // Wider search radius when starving
    }
}

void ADinosaurBase::UpdateBehavior()
{
    // Simple state machine — full Behavior Tree integration by NPC Behavior #11
    switch (CurrentBehaviorState)
    {
        case EDinosaurBehavior::Idle:
            // Randomly patrol territory
            if (FMath::RandRange(0, 3) == 0)
            {
                CurrentBehaviorState = EDinosaurBehavior::Patrolling;
            }
            break;

        case EDinosaurBehavior::Patrolling:
            // Return to idle occasionally
            if (FMath::RandRange(0, 5) == 0)
            {
                CurrentBehaviorState = EDinosaurBehavior::Idle;
            }
            break;

        case EDinosaurBehavior::Fleeing:
            // After fleeing, return to patrol
            if (FMath::RandRange(0, 4) == 0)
            {
                bIsAggressive = false;
                CurrentBehaviorState = EDinosaurBehavior::Patrolling;
            }
            break;

        case EDinosaurBehavior::Attacking:
            // Drain stamina during attack
            Stamina = FMath::Max(0.0f, Stamina - 5.0f);
            if (Stamina <= 0.0f)
            {
                CurrentBehaviorState = EDinosaurBehavior::Fleeing;
            }
            break;

        default:
            break;
    }
}

bool ADinosaurBase::IsWithinTerritory(const FVector& Location) const
{
    if (!bHomeLocationSet) return true;
    return FVector::Dist(Location, HomeLocation) <= TerritoryRadius;
}

EDinosaurBehavior ADinosaurBase::GetCurrentBehavior() const
{
    return CurrentBehaviorState;
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    CurrentBehaviorState = NewState;
}
