// DinosaurBase.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260620_005
// Base class for all dinosaur types in the prehistoric survival game.
// Inherits from APawn. All dinosaur species (TRex, Raptor, Brachiosaurus, etc.)
// must inherit from this class and override the virtual methods below.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule root
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCapsuleHalfHeight(100.0f);
    CapsuleComponent->SetCapsuleRadius(50.0f);
    CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
    SetRootComponent(CapsuleComponent);

    // Skeletal mesh
    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(CapsuleComponent);
    MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -100.0f));

    // Floating movement (replaced by CharacterMovement in subclasses if needed)
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = 600.0f;

    // Default survival stats
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxStamina = 100.0f;
    CurrentStamina = 100.0f;
    HungerLevel = 0.0f;
    ThirstLevel = 0.0f;

    // Default behaviour state
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    bIsAggressive = false;
    bIsPredator = false;
    DetectionRadius = 1500.0f;
    AttackRadius = 200.0f;
    WanderRadius = 3000.0f;

    // Movement speeds
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    bIsRunning = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;

    // Start metabolism timer (hunger/thirst increase over time)
    GetWorldTimerManager().SetTimer(
        MetabolismTimerHandle,
        this,
        &ADinosaurBase::TickMetabolism,
        5.0f,   // every 5 seconds
        true    // looping
    );

    OnDinosaurSpawned();
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina recovery when not running
    if (!bIsRunning && CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + 10.0f * DeltaTime);
    }

    // Drain stamina when running
    if (bIsRunning && CurrentStamina > 0.0f)
    {
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - 20.0f * DeltaTime);
        if (CurrentStamina <= 0.0f)
        {
            SetRunning(false);
        }
    }
}

void ADinosaurBase::TickMetabolism()
{
    // Hunger and thirst increase over time — drives foraging/drinking behaviour
    HungerLevel = FMath::Min(100.0f, HungerLevel + 2.0f);
    ThirstLevel = FMath::Min(100.0f, ThirstLevel + 1.5f);

    // Starvation damage
    if (HungerLevel >= 100.0f)
    {
        TakeDamage(5.0f, FDamageEvent(), nullptr, nullptr);
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    if (CurrentHealth <= 0.0f)
    {
        OnDinosaurDeath();
        Destroy();
    }
    else if (bIsPredator && ActualDamage > 0.0f)
    {
        // Predators become aggressive when hit
        SetBehaviorState(EDinosaurBehaviorState::Attacking);
    }

    return ActualDamage;
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    EDinosaurBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    OnBehaviorStateChanged(OldState, NewState);

    // Adjust movement speed based on state
    switch (NewState)
    {
    case EDinosaurBehaviorState::Fleeing:
    case EDinosaurBehaviorState::Attacking:
    case EDinosaurBehaviorState::Chasing:
        SetRunning(true);
        break;
    case EDinosaurBehaviorState::Idle:
    case EDinosaurBehaviorState::Wandering:
    case EDinosaurBehaviorState::Grazing:
    case EDinosaurBehaviorState::Drinking:
        SetRunning(false);
        break;
    default:
        break;
    }
}

void ADinosaurBase::SetRunning(bool bRun)
{
    bIsRunning = bRun;
    if (MovementComponent)
    {
        MovementComponent->MaxSpeed = bRun ? RunSpeed : WalkSpeed;
    }
}

bool ADinosaurBase::CanAttack() const
{
    return bIsPredator
        && CurrentStamina > 10.0f
        && CurrentHealth > 0.0f
        && CurrentBehaviorState != EDinosaurBehaviorState::Fleeing;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || !CanAttack()) return;

    // Drain stamina on attack
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - 15.0f);

    // Apply damage to target
    float AttackDamage = GetAttackDamage();
    FDamageEvent DmgEvent;
    Target->TakeDamage(AttackDamage, DmgEvent, GetController(), this);

    OnAttackPerformed(Target);
}

float ADinosaurBase::GetAttackDamage() const
{
    // Base damage — subclasses override for species-specific values
    return 25.0f;
}

void ADinosaurBase::OnDinosaurSpawned()
{
    // Override in subclasses for spawn logic (sound, animation, etc.)
}

void ADinosaurBase::OnDinosaurDeath()
{
    // Override in subclasses for death logic (ragdoll, loot drop, etc.)
    GetWorldTimerManager().ClearTimer(MetabolismTimerHandle);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehaviorState OldState, EDinosaurBehaviorState NewState)
{
    // Override in subclasses to react to state changes
}

void ADinosaurBase::OnAttackPerformed(AActor* Target)
{
    // Override in subclasses for attack VFX/SFX
}
