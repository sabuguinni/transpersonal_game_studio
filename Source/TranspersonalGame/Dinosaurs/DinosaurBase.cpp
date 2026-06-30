// DinosaurBase.cpp
// Engine Architect #02 — PROD_CYCLE_AUTO_20260630_009
// Base class for all dinosaur species. Provides survival stats, threat response,
// locomotion state machine, and damage handling. All species inherit from this.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Constructor
// ============================================================
ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz update for AI responsiveness

    // --- Capsule sizing (overridden per species in child classes) ---
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleHalfHeight(88.0f);
        Capsule->SetCapsuleRadius(34.0f);
    }

    // --- Movement defaults ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed         = 400.0f;
        MoveComp->MaxAcceleration      = 1024.0f;
        MoveComp->BrakingDecelerationWalking = 512.0f;
        MoveComp->RotationRate         = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->GravityScale         = 1.0f;
        MoveComp->JumpZVelocity        = 420.0f;
    }

    // --- Default survival stats ---
    MaxHealth       = 500.0f;
    CurrentHealth   = 500.0f;
    MaxHunger       = 100.0f;
    CurrentHunger   = 80.0f;
    HungerDrainRate = 1.0f;   // per second
    MaxStamina      = 100.0f;
    CurrentStamina  = 100.0f;
    StaminaDrainRate = 5.0f;  // per second when sprinting
    StaminaRegenRate = 3.0f;  // per second when idle

    // --- Species defaults (overridden by child classes) ---
    SpeciesName     = FName("Unknown");
    bIsCarnivore    = true;
    bIsPredator     = true;
    ThreatRadius    = 1500.0f;
    AttackRange     = 200.0f;
    AttackDamage    = 50.0f;
    AttackCooldown  = 2.0f;
    bIsAttacking    = false;
    bIsFleeing      = false;
    bIsAlerted      = false;
    bIsDead         = false;

    // --- Locomotion state ---
    CurrentLocomotionState = EEng_DinoLocomotionState::Idle;
    SprintSpeedMultiplier  = 2.0f;
    WalkSpeedBase          = 400.0f;

    // --- Mesh setup (child classes assign the actual mesh asset) ---
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -Capsule->GetScaledCapsuleHalfHeight()));
        MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}

// ============================================================
// BeginPlay
// ============================================================
void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth  = MaxHealth;
    CurrentHunger  = MaxHunger * 0.8f;
    CurrentStamina = MaxStamina;

    // Start hunger drain timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::DrainHunger,
        1.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s spawned — Health: %.0f, Hunger: %.0f"),
        *SpeciesName.ToString(), CurrentHealth, CurrentHunger);
}

// ============================================================
// Tick — Locomotion state machine + stamina management
// ============================================================
void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead) return;

    UpdateLocomotionState(DeltaTime);
    UpdateStamina(DeltaTime);
    ScanForThreats();
}

// ============================================================
// UpdateLocomotionState — Drives animation state machine
// ============================================================
void ADinosaurBase::UpdateLocomotionState(float DeltaTime)
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    float Speed = GetVelocity().Size();

    EEng_DinoLocomotionState NewState = EEng_DinoLocomotionState::Idle;

    if (bIsDead)
    {
        NewState = EEng_DinoLocomotionState::Dead;
    }
    else if (bIsAttacking)
    {
        NewState = EEng_DinoLocomotionState::Attacking;
    }
    else if (Speed > WalkSpeedBase * SprintSpeedMultiplier * 0.8f)
    {
        NewState = EEng_DinoLocomotionState::Sprinting;
    }
    else if (Speed > 50.0f)
    {
        NewState = EEng_DinoLocomotionState::Walking;
    }
    else if (bIsAlerted)
    {
        NewState = EEng_DinoLocomotionState::Alert;
    }
    else
    {
        NewState = EEng_DinoLocomotionState::Idle;
    }

    if (NewState != CurrentLocomotionState)
    {
        CurrentLocomotionState = NewState;
        OnLocomotionStateChanged.Broadcast(NewState);
    }
}

// ============================================================
// UpdateStamina — Drain when sprinting, regen when idle/walking
// ============================================================
void ADinosaurBase::UpdateStamina(float DeltaTime)
{
    if (CurrentLocomotionState == EEng_DinoLocomotionState::Sprinting)
    {
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - StaminaDrainRate * DeltaTime);
        if (CurrentStamina <= 0.0f)
        {
            // Force walk when exhausted
            UCharacterMovementComponent* MoveComp = GetCharacterMovement();
            if (MoveComp)
            {
                MoveComp->MaxWalkSpeed = WalkSpeedBase;
            }
        }
    }
    else if (CurrentLocomotionState == EEng_DinoLocomotionState::Idle ||
             CurrentLocomotionState == EEng_DinoLocomotionState::Walking)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
    }
}

// ============================================================
// DrainHunger — Called by timer every second
// ============================================================
void ADinosaurBase::DrainHunger()
{
    if (bIsDead) return;

    CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDrainRate);

    // Starvation damage
    if (CurrentHunger <= 0.0f)
    {
        TakeDamage(5.0f, FDamageEvent(), nullptr, nullptr);
        UE_LOG(LogTemp, Warning, TEXT("[DinosaurBase] %s is starving!"), *SpeciesName.ToString());
    }
}

// ============================================================
// ScanForThreats — Detect player within ThreatRadius
// ============================================================
void ADinosaurBase::ScanForThreats()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= ThreatRadius)
    {
        if (!bIsAlerted)
        {
            bIsAlerted = true;
            OnThreatDetected.Broadcast(PlayerPawn);
            UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s detected threat at distance %.0f"),
                *SpeciesName.ToString(), DistToPlayer);
        }

        // Attack if in range and not on cooldown
        if (DistToPlayer <= AttackRange && !bIsAttacking)
        {
            PerformAttack(PlayerPawn);
        }
    }
    else
    {
        bIsAlerted = false;
    }
}

// ============================================================
// TakeDamage — Override from AActor
// ============================================================
float ADinosaurBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (bIsDead) return 0.0f;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    OnDamageTaken.Broadcast(DamageAmount, CurrentHealth);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s took %.0f damage — Health: %.0f/%.0f"),
        *SpeciesName.ToString(), DamageAmount, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return DamageAmount;
}

// ============================================================
// PerformAttack — Execute attack with cooldown
// ============================================================
void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || bIsAttacking || bIsDead) return;

    bIsAttacking = true;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
    OnAttackPerformed.Broadcast(Target);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s attacked %s for %.0f damage"),
        *SpeciesName.ToString(), *Target->GetName(), AttackDamage);

    // Reset attack flag after cooldown
    FTimerHandle AttackCooldownHandle;
    GetWorldTimerManager().SetTimer(
        AttackCooldownHandle,
        [this]() { bIsAttacking = false; },
        AttackCooldown,
        false
    );
}

// ============================================================
// Die — Handle death state
// ============================================================
void ADinosaurBase::Die()
{
    if (bIsDead) return;

    bIsDead = true;
    bIsAttacking = false;
    bIsAlerted = false;

    GetWorldTimerManager().ClearTimer(HungerTimerHandle);

    // Disable movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }

    // Disable collision
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    OnDeath.Broadcast();
    CurrentLocomotionState = EEng_DinoLocomotionState::Dead;

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s has died."), *SpeciesName.ToString());

    // Destroy actor after 30 seconds (corpse persistence)
    SetLifeSpan(30.0f);
}

// ============================================================
// SetSprinting — Enable/disable sprint speed
// ============================================================
void ADinosaurBase::SetSprinting(bool bSprint)
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    if (bSprint && CurrentStamina > 10.0f)
    {
        MoveComp->MaxWalkSpeed = WalkSpeedBase * SprintSpeedMultiplier;
    }
    else
    {
        MoveComp->MaxWalkSpeed = WalkSpeedBase;
    }
}

// ============================================================
// GetHealthPercent — Utility for UI/AI
// ============================================================
float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

// ============================================================
// GetHungerPercent
// ============================================================
float ADinosaurBase::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}

// ============================================================
// GetStaminaPercent
// ============================================================
float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}
