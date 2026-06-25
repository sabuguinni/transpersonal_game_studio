// DinosaurBase.cpp
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260625_005
// Full implementation of ADinosaurBase with LOD-aware tick throttling.
// Key perf feature: distant dinos (>3000u) tick at 2s, very distant (>6000u) at 5s.

#include "DinosaurBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

// ── Constructor ──────────────────────────────────────────────────────────────

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Start at 10Hz; UpdateLODTickRate() adjusts this

    // Survival component — tracks hunger/thirst for dinosaurs too
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // Default capsule size (overridden per species in child BPs)
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.MovementSpeed;
    GetCharacterMovement()->JumpZVelocity = 0.0f; // Dinos don't jump by default
    GetCharacterMovement()->AirControl = 0.0f;

    // Disable controller rotation — movement component handles it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Set initial health from stats
    CurrentHealth = DinoStats.MaxHealth;
}

// ── BeginPlay ────────────────────────────────────────────────────────────────

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Sync health with stats
    CurrentHealth = DinoStats.MaxHealth;

    // Apply movement speed from stats
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.MovementSpeed;
    }

    // Configure SurvivalComponent for dinosaur (slower drain rates than player)
    if (SurvivalComp)
    {
        SurvivalComp->HungerDrainRate = 0.5f;   // Dinos drain hunger slowly
        SurvivalComp->ThirstDrainRate = 0.3f;   // Dinos are more water-efficient
        SurvivalComp->StaminaDrainRate = 2.0f;  // Stamina drains during sprinting
    }

    // Initial LOD tick rate calculation
    UpdateLODTickRate();
}

// ── Tick ─────────────────────────────────────────────────────────────────────

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (IsDead()) return;

    // ── LOD tick accumulator ─────────────────────────────────────────────────
    // UpdateLODTickRate is expensive — only run it every PlayerCacheInterval seconds
    PlayerCacheTimer += DeltaTime;
    if (PlayerCacheTimer >= PlayerCacheInterval)
    {
        PlayerCacheTimer = 0.0f;
        UpdateLODTickRate();
    }

    // ── LOD-gated AI update ──────────────────────────────────────────────────
    LODTickAccumulator += DeltaTime;
    if (LODTickAccumulator >= CurrentLODTickInterval)
    {
        LODTickAccumulator = 0.0f;
        UpdateBehaviorAI(CurrentLODTickInterval);
    }
}

// ── TakeDamage ───────────────────────────────────────────────────────────────

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    if (IsDead()) return 0.0f;

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, DinoStats.MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        HandleDeath(DamageCauser);
    }

    return ActualDamage;
}

// ── SetBehaviorState ─────────────────────────────────────────────────────────

void ADinosaurBase::SetBehaviorState(EPerf_DinoBehaviorState NewState)
{
    if (BehaviorState == NewState) return;

    BehaviorState = NewState;
    OnDinoStateChanged.Broadcast(this, NewState);
}

// ── GetHealthPercent ─────────────────────────────────────────────────────────

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / DinoStats.MaxHealth;
}

// ── GetDistanceToPlayer ──────────────────────────────────────────────────────

float ADinosaurBase::GetDistanceToPlayer() const
{
    if (!CachedPlayerPawn) return TNumericLimits<float>::Max();
    return FVector::Dist(GetActorLocation(), CachedPlayerPawn->GetActorLocation());
}

// ── UpdateLODTickRate ─────────────────────────────────────────────────────────
// Core performance feature: throttle AI tick rate based on distance to player.
// Near  (<3000u): 0.1s interval  — full AI fidelity
// Mid   (<6000u): 2.0s interval  — reduced AI updates
// Far   (<12000u): 5.0s interval — minimal AI (just state machine)
// Culled (>12000u): AI suspended entirely

void ADinosaurBase::UpdateLODTickRate()
{
    // Refresh player pawn cache
    UWorld* World = GetWorld();
    if (!World) return;

    if (!CachedPlayerPawn || !IsValid(CachedPlayerPawn))
    {
        CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    }

    const float Dist = GetDistanceToPlayer();

    if (Dist > LODConfig.CullDistance)
    {
        // Suspend AI entirely — set a very long interval
        CurrentLODTickInterval = 30.0f;
        SetActorTickEnabled(false);
    }
    else if (Dist > LODConfig.FarLODDistance)
    {
        CurrentLODTickInterval = LODConfig.FarTickInterval;
        SetActorTickEnabled(true);
    }
    else if (Dist > LODConfig.MediumLODDistance)
    {
        CurrentLODTickInterval = LODConfig.MediumTickInterval;
        SetActorTickEnabled(true);
    }
    else
    {
        // Near — full fidelity
        CurrentLODTickInterval = 0.1f;
        SetActorTickEnabled(true);
    }
}

// ── HandleDeath ──────────────────────────────────────────────────────────────

void ADinosaurBase::HandleDeath(AActor* Killer)
{
    SetBehaviorState(EPerf_DinoBehaviorState::Dead);

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
        GetCharacterMovement()->StopMovementImmediately();
    }

    // Disable tick — dead dinos don't need updates
    SetActorTickEnabled(false);

    // Broadcast death event
    OnDinoDied.Broadcast(this, Killer);
}

// ── UpdateBehaviorAI ─────────────────────────────────────────────────────────
// Minimal stub — full AI implemented by Agent #12 (Combat & Enemy AI).
// This base class just handles state transitions based on survival stats.

void ADinosaurBase::UpdateBehaviorAI(float DeltaTime)
{
    if (!SurvivalComp) return;

    const float Hunger = SurvivalComp->GetStat(EPerf_SurvivalStat::Hunger);
    const float Fear   = SurvivalComp->GetStat(EPerf_SurvivalStat::Fear);

    // Simple state machine — overridden by child classes / BehaviorTree
    switch (BehaviorState)
    {
    case EPerf_DinoBehaviorState::Idle:
        if (Hunger < 30.0f)
        {
            SetBehaviorState(EPerf_DinoBehaviorState::Foraging);
        }
        break;

    case EPerf_DinoBehaviorState::Foraging:
        if (Hunger > 70.0f)
        {
            SetBehaviorState(EPerf_DinoBehaviorState::Patrolling);
        }
        break;

    case EPerf_DinoBehaviorState::Fleeing:
        if (Fear < 20.0f)
        {
            SetBehaviorState(EPerf_DinoBehaviorState::Idle);
        }
        break;

    default:
        break;
    }
}
