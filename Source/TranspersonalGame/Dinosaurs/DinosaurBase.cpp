// DinosaurBase.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260620_003
// Base implementation for all dinosaur types in TranspersonalGame.
// Provides movement, survival stats, AI state machine foundation,
// and sensory system hooks for Behavior Tree integration.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz tick — performance budget

    // ── Capsule ──────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // ── Movement ─────────────────────────────────────────────
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f;
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = false;
    GetCharacterMovement()->NavAgentProps.bCanJump = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // ── Default Species Stats ─────────────────────────────────
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    MaxStamina = 100.0f;
    CurrentStamina = MaxStamina;
    AttackDamage = 50.0f;
    AttackRange = 150.0f;
    DetectionRadius = 1500.0f;
    HearingRadius = 800.0f;
    WalkSpeed = 250.0f;
    RunSpeed = 600.0f;
    TurnSpeed = 180.0f;

    // ── AI State ─────────────────────────────────────────────
    CurrentBehaviorState = EDinoAIState::Idle;
    bIsAlerted = false;
    bIsAttacking = false;
    bIsFleeing = false;
    ThreatTarget = nullptr;
    LastKnownThreatLocation = FVector::ZeroVector;
    AlertCooldown = 5.0f;
    AttackCooldown = 2.0f;
    bCanAttack = true;

    // ── Species Identity ──────────────────────────────────────
    SpeciesName = TEXT("Unknown Dinosaur");
    DietType = EDinoDiet::Carnivore;
    SizeClass = EDinoSize::Medium;
    bIsPackAnimal = false;
    PackRadius = 500.0f;

    // ── AI Controller ─────────────────────────────────────────
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific movement speeds
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    // Start idle behavior loop
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        0.5f,   // 2Hz behavior update
        true
    );

    // Start stamina recovery
    GetWorldTimerManager().SetTimer(
        StaminaRecoveryTimer,
        this,
        &ADinosaurBase::RecoverStamina,
        1.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Rotate toward movement direction smoothly
    if (!GetVelocity().IsNearlyZero(1.0f))
    {
        FRotator CurrentRot = GetActorRotation();
        FRotator TargetRot = GetVelocity().Rotation();
        FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, TurnSpeed / 180.0f);
        SetActorRotation(FRotator(0.0f, NewRot.Yaw, 0.0f));
    }
}

// ── Health & Damage ──────────────────────────────────────────

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    // React to damage — alert and potentially flee
    if (ActualDamage > 0.0f)
    {
        OnDamageReceived(ActualDamage, DamageCauser);

        if (DamageCauser)
        {
            ThreatTarget = DamageCauser;
            LastKnownThreatLocation = DamageCauser->GetActorLocation();
            bIsAlerted = true;
        }
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::OnDamageReceived_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Base: switch to alert state when damaged
    if (DietType == EDinoDiet::Herbivore && DamageAmount > 20.0f)
    {
        SetBehaviorState(EDinoAIState::Fleeing);
    }
    else if (DietType == EDinoDiet::Carnivore)
    {
        SetBehaviorState(EDinoAIState::Attacking);
    }
}

void ADinosaurBase::Die()
{
    // Disable collision and AI
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    if (AController* Ctrl = GetController())
    {
        Ctrl->UnPossess();
    }

    OnDeath();

    // Destroy after ragdoll settle time
    SetLifeSpan(10.0f);
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Enable ragdoll on mesh
    if (USkeletalMeshComponent* Mesh = GetMesh())
    {
        Mesh->SetSimulatePhysics(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

// ── AI State Machine ─────────────────────────────────────────

void ADinosaurBase::SetBehaviorState(EDinoAIState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    EDinoAIState PreviousState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    // Apply movement speed for new state
    switch (NewState)
    {
        case EDinoAIState::Idle:
        case EDinoAIState::Grazing:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.5f;
            break;
        case EDinoAIState::Wandering:
        case EDinoAIState::Patrolling:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            break;
        case EDinoAIState::Alerted:
        case EDinoAIState::Investigating:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 1.5f;
            break;
        case EDinoAIState::Chasing:
        case EDinoAIState::Attacking:
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
            break;
        case EDinoAIState::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed * 1.2f;
            break;
        case EDinoAIState::Resting:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;
        default:
            break;
    }

    OnBehaviorStateChanged(PreviousState, NewState);
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (CurrentHealth <= 0.0f) return;

    // Stamina-based state transitions
    if (CurrentStamina < 20.0f && CurrentBehaviorState == EDinoAIState::Chasing)
    {
        SetBehaviorState(EDinoAIState::Alerted);
        return;
    }

    // Alert cooldown
    if (bIsAlerted && ThreatTarget == nullptr)
    {
        AlertCooldown -= 0.5f;
        if (AlertCooldown <= 0.0f)
        {
            bIsAlerted = false;
            AlertCooldown = 5.0f;
            SetBehaviorState(EDinoAIState::Wandering);
        }
    }
}

void ADinosaurBase::OnBehaviorStateChanged_Implementation(EDinoAIState PreviousState, EDinoAIState NewState)
{
    // Blueprint-overridable hook — base does nothing
}

// ── Sensory System ───────────────────────────────────────────

bool ADinosaurBase::CanSeeTarget(AActor* Target) const
{
    if (!Target) return false;

    float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
    if (DistSq > DetectionRadius * DetectionRadius) return false;

    // Field of view check (120 degree cone)
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    float DotProduct = FVector::DotProduct(GetActorForwardVector(), ToTarget);
    if (DotProduct < -0.17f) return false; // Outside ~100 degree FOV

    // Line of sight trace
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        GetActorLocation() + FVector(0, 0, 50.0f),
        Target->GetActorLocation() + FVector(0, 0, 50.0f),
        ECC_Visibility,
        Params
    );

    return !bHit || HitResult.GetActor() == Target;
}

bool ADinosaurBase::CanHearTarget(AActor* Target) const
{
    if (!Target) return false;
    float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
    return DistSq <= HearingRadius * HearingRadius;
}

void ADinosaurBase::AlertToThreat(AActor* Threat, FVector ThreatLocation)
{
    if (!Threat) return;

    ThreatTarget = Threat;
    LastKnownThreatLocation = ThreatLocation;
    bIsAlerted = true;
    AlertCooldown = 5.0f;

    if (CurrentBehaviorState == EDinoAIState::Idle ||
        CurrentBehaviorState == EDinoAIState::Wandering ||
        CurrentBehaviorState == EDinoAIState::Grazing)
    {
        SetBehaviorState(EDinoAIState::Alerted);
    }
}

// ── Combat ───────────────────────────────────────────────────

void ADinosaurBase::PerformAttack()
{
    if (!bCanAttack || CurrentBehaviorState != EDinoAIState::Attacking) return;
    if (!ThreatTarget) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), ThreatTarget->GetActorLocation());
    if (DistToTarget > AttackRange) return;

    // Apply damage
    FDamageEvent DmgEvent;
    ThreatTarget->TakeDamage(AttackDamage, DmgEvent, GetController(), this);

    // Consume stamina
    CurrentStamina = FMath::Clamp(CurrentStamina - 15.0f, 0.0f, MaxStamina);

    // Attack cooldown
    bCanAttack = false;
    FTimerHandle AttackCooldownHandle;
    GetWorldTimerManager().SetTimer(
        AttackCooldownHandle,
        [this]() { bCanAttack = true; },
        AttackCooldown,
        false
    );
}

// ── Stamina ──────────────────────────────────────────────────

void ADinosaurBase::RecoverStamina()
{
    if (CurrentBehaviorState == EDinoAIState::Resting)
    {
        CurrentStamina = FMath::Clamp(CurrentStamina + 20.0f, 0.0f, MaxStamina);
    }
    else if (CurrentBehaviorState == EDinoAIState::Idle ||
             CurrentBehaviorState == EDinoAIState::Grazing)
    {
        CurrentStamina = FMath::Clamp(CurrentStamina + 5.0f, 0.0f, MaxStamina);
    }
    else if (CurrentBehaviorState == EDinoAIState::Chasing ||
             CurrentBehaviorState == EDinoAIState::Fleeing)
    {
        CurrentStamina = FMath::Clamp(CurrentStamina - 5.0f, 0.0f, MaxStamina);
    }
}

// ── Getters ──────────────────────────────────────────────────

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

EDinoAIState ADinosaurBase::GetCurrentBehaviorState() const
{
    return CurrentBehaviorState;
}
