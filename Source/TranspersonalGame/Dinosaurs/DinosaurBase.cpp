// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur pawns. Inherits ACharacter for movement + capsule collision.
// All species-specific dinosaurs (TRex, Raptor, Brachiosaurus, etc.) inherit from this.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxStamina = 200.0f;
    CurrentStamina = 200.0f;
    AttackDamage = 50.0f;
    DetectionRadius = 1500.0f;
    AttackRange = 200.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    bIsAggressive = false;
    bIsAlerted = false;
    bIsDead = false;
    CurrentBehaviorState = EDinoState::Idle;
    DinosaurSpecies = EDinoSpecies::Generic;

    // Configure capsule
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Configure movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->AirControl = 0.2f;
        MoveComp->GravityScale = 1.0f;
    }

    // Mesh defaults — species subclasses override with actual skeletal mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific stats on begin play
    ApplySpeciesStats();

    // Start idle behaviour loop
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

    // Passive stamina regeneration
    if (CurrentStamina < MaxStamina && !bIsDead)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (20.0f * DeltaTime));
    }
}

// ─── Combat ──────────────────────────────────────────────────────────────────

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        // Alert the dinosaur when hit
        bIsAlerted = true;
        SetBehaviorState(EDinoState::Alert);
    }

    return ActualDamage;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || bIsDead) return;
    if (CurrentStamina < 20.0f) return;  // Not enough stamina to attack

    const float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - 20.0f);
    }
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;
    CurrentBehaviorState = EDinoState::Dead;

    // Stop behavior timer
    GetWorldTimerManager().ClearTimer(BehaviorTickHandle);

    // Enable ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Destroy after 30 seconds (corpse persistence)
    SetLifeSpan(30.0f);
}

// ─── Behaviour ───────────────────────────────────────────────────────────────

void ADinosaurBase::SetBehaviorState(EDinoState NewState)
{
    if (CurrentBehaviorState == NewState) return;
    CurrentBehaviorState = NewState;

    // Adjust movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    switch (NewState)
    {
        case EDinoState::Idle:
        case EDinoState::Grazing:
            MoveComp->MaxWalkSpeed = WalkSpeed * 0.3f;
            break;
        case EDinoState::Patrolling:
        case EDinoState::Wandering:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
        case EDinoState::Alert:
        case EDinoState::Fleeing:
            MoveComp->MaxWalkSpeed = RunSpeed;
            break;
        case EDinoState::Chasing:
        case EDinoState::Attacking:
            MoveComp->MaxWalkSpeed = RunSpeed;
            bIsAggressive = true;
            break;
        case EDinoState::Dead:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
    }
}

void ADinosaurBase::UpdateBehavior()
{
    if (bIsDead) return;

    // Simple state machine — subclasses override for species-specific AI
    switch (CurrentBehaviorState)
    {
        case EDinoState::Idle:
            // 30% chance to start wandering
            if (FMath::RandRange(0, 9) < 3)
            {
                SetBehaviorState(EDinoState::Wandering);
            }
            break;

        case EDinoState::Wandering:
            // 20% chance to return to idle / graze
            if (FMath::RandRange(0, 9) < 2)
            {
                SetBehaviorState(bIsAggressive ? EDinoState::Patrolling : EDinoState::Grazing);
            }
            break;

        case EDinoState::Alert:
            // Alert fades after a few ticks if no threat detected
            if (!bIsAlerted)
            {
                SetBehaviorState(EDinoState::Idle);
            }
            bIsAlerted = false;  // Reset flag — will be re-set if threat persists
            break;

        default:
            break;
    }
}

// ─── Species stats ────────────────────────────────────────────────────────────

void ADinosaurBase::ApplySpeciesStats()
{
    switch (DinosaurSpecies)
    {
        case EDinoSpecies::TRex:
            MaxHealth = 2000.0f;
            CurrentHealth = 2000.0f;
            AttackDamage = 250.0f;
            DetectionRadius = 3000.0f;
            AttackRange = 350.0f;
            WalkSpeed = 250.0f;
            RunSpeed = 800.0f;
            bIsAggressive = true;
            GetCapsuleComponent()->InitCapsuleSize(120.0f, 280.0f);
            break;

        case EDinoSpecies::Raptor:
            MaxHealth = 300.0f;
            CurrentHealth = 300.0f;
            AttackDamage = 80.0f;
            DetectionRadius = 2000.0f;
            AttackRange = 150.0f;
            WalkSpeed = 400.0f;
            RunSpeed = 1100.0f;
            bIsAggressive = true;
            GetCapsuleComponent()->InitCapsuleSize(40.0f, 80.0f);
            break;

        case EDinoSpecies::Brachiosaurus:
            MaxHealth = 5000.0f;
            CurrentHealth = 5000.0f;
            AttackDamage = 150.0f;  // Stomp damage
            DetectionRadius = 800.0f;
            AttackRange = 400.0f;
            WalkSpeed = 200.0f;
            RunSpeed = 400.0f;
            bIsAggressive = false;
            GetCapsuleComponent()->InitCapsuleSize(200.0f, 500.0f);
            break;

        case EDinoSpecies::Spinosaurus:
            MaxHealth = 2500.0f;
            CurrentHealth = 2500.0f;
            AttackDamage = 200.0f;
            DetectionRadius = 2500.0f;
            AttackRange = 300.0f;
            WalkSpeed = 300.0f;
            RunSpeed = 750.0f;
            bIsAggressive = true;
            GetCapsuleComponent()->InitCapsuleSize(100.0f, 250.0f);
            break;

        case EDinoSpecies::Triceratops:
            MaxHealth = 1500.0f;
            CurrentHealth = 1500.0f;
            AttackDamage = 120.0f;
            DetectionRadius = 1200.0f;
            AttackRange = 250.0f;
            WalkSpeed = 280.0f;
            RunSpeed = 600.0f;
            bIsAggressive = false;  // Defensive, not predatory
            GetCapsuleComponent()->InitCapsuleSize(90.0f, 180.0f);
            break;

        case EDinoSpecies::Stegosaurus:
            MaxHealth = 1200.0f;
            CurrentHealth = 1200.0f;
            AttackDamage = 100.0f;
            DetectionRadius = 1000.0f;
            AttackRange = 200.0f;
            WalkSpeed = 220.0f;
            RunSpeed = 450.0f;
            bIsAggressive = false;
            GetCapsuleComponent()->InitCapsuleSize(80.0f, 160.0f);
            break;

        case EDinoSpecies::Generic:
        default:
            // Keep constructor defaults
            break;
    }

    // Apply walk speed to movement component
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }
}
