// DinosaurBase.cpp
// Engine Architect #02 — PROD_CYCLE_AUTO_20260624_001
// Base implementation for all dinosaur types in the prehistoric survival game.
// All species (TRex, Raptor, Brachiosaurus, Stegosaurus, Triceratops) inherit from this.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // ── Movement ─────────────────────────────────────────────────────────────
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->JumpZVelocity = 0.0f;   // most dinos don't jump
    GetCharacterMovement()->AirControl = 0.0f;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->bCanWalkOffLedges = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ── AI Perception stimulus source ────────────────────────────────────────
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(
        TEXT("PerceptionStimuliSource"));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
    PerceptionStimuliSource->bAutoRegister = true;

    // ── Default survival stats ────────────────────────────────────────────────
    MaxHealth           = 500.0f;
    CurrentHealth       = 500.0f;
    AttackDamage        = 50.0f;
    AttackRange         = 200.0f;
    DetectionRange      = 2000.0f;
    ChaseRange          = 3000.0f;
    WalkSpeed           = 300.0f;
    RunSpeed            = 700.0f;
    bIsAggressive       = false;
    bIsHunting          = false;
    bIsFleeing          = false;
    bIsDead             = false;
    CurrentBehaviorState = EDinosaurBehavior::Idle;
    DinosaurSpecies     = EDinosaurSpecies::Raptor;
    HungerLevel         = 50.0f;
    ThirstLevel         = 50.0f;
    TerritoryRadius     = 1500.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Store home territory center
    TerritoryCenter = GetActorLocation();

    // Apply species-specific stats
    ApplySpeciesStats();

    // Start hunger/thirst decay timer
    GetWorldTimerManager().SetTimer(
        SurvivalTimerHandle,
        this,
        &ADinosaurBase::UpdateSurvivalStats,
        10.0f,   // every 10 seconds
        true     // looping
    );

    // Start behavior update
    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f,    // every 2 seconds
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// ── Damage ────────────────────────────────────────────────────────────────────

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (!bIsAggressive && DamageCauser)
    {
        // Become aggressive when hit
        bIsAggressive = true;
        CurrentBehaviorState = EDinosaurBehavior::Attacking;
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;
    CurrentBehaviorState = EDinosaurBehavior::Dead;

    // Stop all timers
    GetWorldTimerManager().ClearTimer(SurvivalTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    // Disable movement
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Notify blueprint
    OnDinosaurDied();

    // Auto-destroy after 30 seconds (corpse cleanup)
    SetLifeSpan(30.0f);
}

// ── Species stats ─────────────────────────────────────────────────────────────

void ADinosaurBase::ApplySpeciesStats()
{
    switch (DinosaurSpecies)
    {
    case EDinosaurSpecies::TyrannosaurusRex:
        MaxHealth       = 2000.0f;
        CurrentHealth   = 2000.0f;
        AttackDamage    = 150.0f;
        AttackRange     = 350.0f;
        DetectionRange  = 3000.0f;
        ChaseRange      = 5000.0f;
        WalkSpeed       = 250.0f;
        RunSpeed        = 600.0f;
        bIsAggressive   = true;
        TerritoryRadius = 5000.0f;
        GetCapsuleComponent()->InitCapsuleSize(100.0f, 250.0f);
        break;

    case EDinosaurSpecies::Raptor:
        MaxHealth       = 300.0f;
        CurrentHealth   = 300.0f;
        AttackDamage    = 60.0f;
        AttackRange     = 150.0f;
        DetectionRange  = 2500.0f;
        ChaseRange      = 4000.0f;
        WalkSpeed       = 350.0f;
        RunSpeed        = 900.0f;
        bIsAggressive   = true;
        TerritoryRadius = 2000.0f;
        break;

    case EDinosaurSpecies::Brachiosaurus:
        MaxHealth       = 3000.0f;
        CurrentHealth   = 3000.0f;
        AttackDamage    = 80.0f;   // stomp
        AttackRange     = 400.0f;
        DetectionRange  = 1500.0f;
        ChaseRange      = 1000.0f; // won't chase far
        WalkSpeed       = 200.0f;
        RunSpeed        = 400.0f;
        bIsAggressive   = false;
        TerritoryRadius = 3000.0f;
        GetCapsuleComponent()->InitCapsuleSize(150.0f, 400.0f);
        break;

    case EDinosaurSpecies::Stegosaurus:
        MaxHealth       = 800.0f;
        CurrentHealth   = 800.0f;
        AttackDamage    = 90.0f;   // tail spike
        AttackRange     = 250.0f;
        DetectionRange  = 1200.0f;
        ChaseRange      = 800.0f;
        WalkSpeed       = 220.0f;
        RunSpeed        = 450.0f;
        bIsAggressive   = false;
        TerritoryRadius = 2000.0f;
        break;

    case EDinosaurSpecies::Triceratops:
        MaxHealth       = 1200.0f;
        CurrentHealth   = 1200.0f;
        AttackDamage    = 110.0f;  // horn charge
        AttackRange     = 300.0f;
        DetectionRange  = 1800.0f;
        ChaseRange      = 2500.0f;
        WalkSpeed       = 280.0f;
        RunSpeed        = 550.0f;
        bIsAggressive   = false;   // defensive only
        TerritoryRadius = 2500.0f;
        GetCapsuleComponent()->InitCapsuleSize(90.0f, 180.0f);
        break;

    default:
        break;
    }

    // Apply speeds to movement component
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// ── Behavior state machine ────────────────────────────────────────────────────

void ADinosaurBase::UpdateBehaviorState()
{
    if (bIsDead) return;

    // Hunger-driven hunting
    if (HungerLevel < 20.0f && !bIsHunting)
    {
        bIsHunting = true;
        CurrentBehaviorState = EDinosaurBehavior::Hunting;
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        return;
    }

    // Thirst-driven migration to water
    if (ThirstLevel < 15.0f)
    {
        CurrentBehaviorState = EDinosaurBehavior::Migrating;
        return;
    }

    // Return to territory if too far
    const float DistFromHome = FVector::Dist(GetActorLocation(), TerritoryCenter);
    if (DistFromHome > TerritoryRadius * 1.5f)
    {
        CurrentBehaviorState = EDinosaurBehavior::Patrolling;
        return;
    }

    // Default idle/patrol
    if (CurrentBehaviorState != EDinosaurBehavior::Attacking &&
        CurrentBehaviorState != EDinosaurBehavior::Hunting)
    {
        CurrentBehaviorState = EDinosaurBehavior::Idle;
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void ADinosaurBase::UpdateSurvivalStats()
{
    if (bIsDead) return;

    // Decay hunger and thirst over time
    HungerLevel = FMath::Clamp(HungerLevel - 2.0f, 0.0f, 100.0f);
    ThirstLevel = FMath::Clamp(ThirstLevel - 1.5f, 0.0f, 100.0f);

    // Starvation damage
    if (HungerLevel <= 0.0f)
    {
        CurrentHealth = FMath::Clamp(CurrentHealth - 10.0f, 0.0f, MaxHealth);
        if (CurrentHealth <= 0.0f) Die();
    }
}

// ── Blueprint-callable helpers ────────────────────────────────────────────────

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    if (bIsDead) return;
    CurrentBehaviorState = NewState;

    switch (NewState)
    {
    case EDinosaurBehavior::Running:
    case EDinosaurBehavior::Attacking:
    case EDinosaurBehavior::Hunting:
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        break;
    default:
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        break;
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

void ADinosaurBase::OnDinosaurDied_Implementation()
{
    // Default: play death animation via ragdoll enable
    // Blueprint can override for species-specific death effects
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}
