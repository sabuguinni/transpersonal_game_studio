// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base implementation for all dinosaur pawns in the prehistoric survival game.
// Inherits from ACharacter to leverage UCharacterMovementComponent for locomotion.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule defaults (overridden per species in subclasses) ---
    GetCapsuleComponent()->InitCapsuleSize(88.0f, 192.0f);

    // --- Skeletal mesh offset so feet touch ground ---
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -192.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // --- Movement defaults ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 400.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MoveComp->JumpZVelocity = 0.0f;      // Most dinos cannot jump
        MoveComp->AirControl = 0.0f;
        MoveComp->GravityScale = 1.0f;
        MoveComp->NavAgentProps.bCanCrouch = false;
        MoveComp->NavAgentProps.bCanJump = false;
        MoveComp->NavAgentProps.bCanWalk = true;
        MoveComp->NavAgentProps.bCanSwim = false;
    }

    // --- Species defaults ---
    DinosaurSpecies = EEng_DinosaurSpecies::Unknown;
    BehaviorState   = EEng_DinosaurBehavior::Idle;
    MaxHealth       = 500.0f;
    CurrentHealth   = 500.0f;
    AttackDamage    = 50.0f;
    AttackRange     = 200.0f;
    DetectionRadius = 2000.0f;
    bIsHerbivore    = false;
    bIsPack         = false;
    PatrolRadius    = 1500.0f;

    // --- Auto-possess: AI controller ---
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    HomeLocation  = GetActorLocation();

    // Start idle patrol loop
    GetWorldTimerManager().SetTimer(
        BehaviorTickHandle,
        this,
        &ADinosaurBase::UpdateBehavior,
        2.0f,   // tick every 2 seconds
        true    // looping
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// ---------------------------------------------------------------------------
// Health / Damage
// ---------------------------------------------------------------------------

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (!bIsHerbivore && BehaviorState == EEng_DinosaurBehavior::Idle)
    {
        // Carnivores become aggressive when hit
        BehaviorState = EEng_DinosaurBehavior::Aggressive;
    }

    return ActualDamage;
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

// ---------------------------------------------------------------------------
// Behavior state machine (called every 2 s from timer)
// ---------------------------------------------------------------------------

void ADinosaurBase::UpdateBehavior()
{
    if (!IsAlive()) return;

    switch (BehaviorState)
    {
        case EEng_DinosaurBehavior::Idle:
            // Randomly decide to patrol
            if (FMath::RandBool())
            {
                BehaviorState = EEng_DinosaurBehavior::Patrolling;
            }
            break;

        case EEng_DinosaurBehavior::Patrolling:
            // Wander within patrol radius; revert to Idle occasionally
            if (FMath::RandRange(0, 4) == 0)
            {
                BehaviorState = EEng_DinosaurBehavior::Idle;
            }
            break;

        case EEng_DinosaurBehavior::Fleeing:
            // Herbivores flee; after distance gained, return to Idle
            BehaviorState = EEng_DinosaurBehavior::Idle;
            break;

        case EEng_DinosaurBehavior::Aggressive:
        case EEng_DinosaurBehavior::Hunting:
            // Handled by AI Controller / Behavior Tree
            break;

        default:
            break;
    }
}

void ADinosaurBase::OnDeath()
{
    BehaviorState = EEng_DinosaurBehavior::Dead;
    GetWorldTimerManager().ClearTimer(BehaviorTickHandle);

    // Disable collision so corpse doesn't block navigation
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy after 30 seconds to keep actor count manageable
    SetLifeSpan(30.0f);
}

// ---------------------------------------------------------------------------
// Species helpers
// ---------------------------------------------------------------------------

FString ADinosaurBase::GetSpeciesName() const
{
    switch (DinosaurSpecies)
    {
        case EEng_DinosaurSpecies::TyrannosaurusRex:  return TEXT("Tyrannosaurus Rex");
        case EEng_DinosaurSpecies::Velociraptor:       return TEXT("Velociraptor");
        case EEng_DinosaurSpecies::Triceratops:        return TEXT("Triceratops");
        case EEng_DinosaurSpecies::Brachiosaurus:      return TEXT("Brachiosaurus");
        case EEng_DinosaurSpecies::Ankylosaurus:       return TEXT("Ankylosaurus");
        case EEng_DinosaurSpecies::Parasaurolophus:    return TEXT("Parasaurolophus");
        case EEng_DinosaurSpecies::Pachycephalosaurus: return TEXT("Pachycephalosaurus");
        case EEng_DinosaurSpecies::Protoceratops:      return TEXT("Protoceratops");
        default:                                        return TEXT("Unknown Dinosaur");
    }
}
