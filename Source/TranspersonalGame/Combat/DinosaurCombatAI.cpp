#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // We manage our own interval
    ApplySpeciesDefaults();
}

// ============================================================
// BeginPlay
// ============================================================

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    ApplySpeciesDefaults();
    CurrentState = ECombat_DinoState::Patrolling;
}

// ============================================================
// TickComponent — throttled at AITickInterval (default 0.1s = 10Hz)
// ============================================================

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AITickAccumulator += DeltaTime;
    if (AITickAccumulator >= AITickInterval)
    {
        UpdateAI(AITickAccumulator);
        AITickAccumulator = 0.f;
    }
}

// ============================================================
// UpdateAI — main AI loop at 10Hz
// ============================================================

void UDinosaurCombatAI::UpdateAI(float DeltaTime)
{
    DecayThreatEntries(DeltaTime);
    SelectPrimaryTarget();
    UpdateState();
}

// ============================================================
// UpdateState — state machine transitions
// ============================================================

void UDinosaurCombatAI::UpdateState()
{
    // Flee if health critical
    if (GetHealthPercent() <= Stats.FleeHealthThreshold && CurrentState != ECombat_DinoState::Fleeing)
    {
        TransitionToState(ECombat_DinoState::Fleeing);
        if (Stats.bIsPackHunter && bIsAlpha)
        {
            BroadcastPackSignal(ECombat_DinoState::Fleeing, GetOwner()->GetActorLocation());
        }
        return;
    }

    if (!PrimaryTarget)
    {
        // No threat — return to patrol or idle
        if (CurrentState == ECombat_DinoState::Hunting ||
            CurrentState == ECombat_DinoState::Attacking ||
            CurrentState == ECombat_DinoState::Alerted)
        {
            TransitionToState(ECombat_DinoState::Patrolling);
        }
        return;
    }

    float DistToTarget = GetDistanceToTarget();

    // Within attack range
    if (DistToTarget <= Stats.AttackRange)
    {
        if (CanAttack())
        {
            TransitionToState(ECombat_DinoState::Attacking);
            ExecuteAttack(PrimaryTarget);
        }
        return;
    }

    // Within detection range — hunt
    if (DistToTarget <= Stats.DetectionRange)
    {
        if (CurrentState != ECombat_DinoState::Hunting && CurrentState != ECombat_DinoState::Attacking)
        {
            TransitionToState(ECombat_DinoState::Hunting);
            // Alpha broadcasts hunt signal to pack
            if (Stats.bIsPackHunter && bIsAlpha)
            {
                BroadcastPackSignal(ECombat_DinoState::Hunting, PrimaryTarget->GetActorLocation());
            }
        }
        return;
    }

    // Target outside detection — alert then return to patrol
    if (CurrentState == ECombat_DinoState::Hunting)
    {
        TransitionToState(ECombat_DinoState::Alerted);
    }
}

// ============================================================
// RegisterThreat — called by perception system or proximity trigger
// ============================================================

void UDinosaurCombatAI::RegisterThreat(AActor* ThreatActor, float ThreatLevel, bool bHasLOS)
{
    if (!ThreatActor) return;

    UWorld* World = GetWorld();
    float Now = World ? World->GetTimeSeconds() : 0.f;

    // Update existing entry
    for (FCombat_ThreatEntry& Entry : KnownThreats)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.ThreatLevel = FMath::Max(Entry.ThreatLevel, ThreatLevel);
            Entry.LastKnownLocation = ThreatActor->GetActorLocation();
            Entry.LastSeenTimestamp = Now;
            Entry.bHasLineOfSight = bHasLOS;
            return;
        }
    }

    // New threat entry
    FCombat_ThreatEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.LastKnownLocation = ThreatActor->GetActorLocation();
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.LastSeenTimestamp = Now;
    NewEntry.bHasLineOfSight = bHasLOS;
    KnownThreats.Add(NewEntry);
}

// ============================================================
// OnTakeDamage — react to being hit
// ============================================================

void UDinosaurCombatAI::OnTakeDamage(float DamageAmount, AActor* DamageSource)
{
    Stats.CurrentHealth = FMath::Max(0.f, Stats.CurrentHealth - DamageAmount);

    // Register attacker as high-priority threat
    if (DamageSource)
    {
        RegisterThreat(DamageSource, 100.f, true);
    }

    // Immediate state reaction
    if (GetHealthPercent() <= Stats.FleeHealthThreshold)
    {
        TransitionToState(ECombat_DinoState::Fleeing);
    }
    else if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrolling)
    {
        TransitionToState(ECombat_DinoState::Alerted);
    }
}

// ============================================================
// CanAttack — cooldown check
// ============================================================

bool UDinosaurCombatAI::CanAttack() const
{
    UWorld* World = GetWorld();
    if (!World) return false;
    return (World->GetTimeSeconds() - LastAttackTime) >= Stats.AttackCooldown;
}

// ============================================================
// SelectAttackType — species-specific attack selection
// ============================================================

ECombat_AttackType UDinosaurCombatAI::SelectAttackType() const
{
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        return ECombat_AttackType::Bite;
    case ECombat_DinoSpecies::Velociraptor:
        return ECombat_AttackType::Swipe;
    case ECombat_DinoSpecies::Triceratops:
        return ECombat_AttackType::Charge;
    case ECombat_DinoSpecies::Brachiosaurus:
        return ECombat_AttackType::Stomp;
    case ECombat_DinoSpecies::Pterodactyl:
        return ECombat_AttackType::DiveStrike;
    default:
        return ECombat_AttackType::Bite;
    }
}

// ============================================================
// ExecuteAttack — deal damage to target
// ============================================================

void UDinosaurCombatAI::ExecuteAttack(AActor* Target)
{
    if (!Target || !CanAttack()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    LastAttackTime = World->GetTimeSeconds();
    LastAttackType = SelectAttackType();

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        Stats.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );

    TransitionToState(ECombat_DinoState::Attacking);
}

// ============================================================
// Pack coordination
// ============================================================

void UDinosaurCombatAI::BroadcastPackSignal(ECombat_DinoState SignalState, FVector TargetLocation)
{
    if (!Stats.bIsPackHunter) return;

    UWorld* World = GetWorld();
    FCombat_PackSignal Signal;
    Signal.TargetLocation = TargetLocation;
    Signal.SignalState = SignalState;
    Signal.SignalTimestamp = World ? World->GetTimeSeconds() : 0.f;
    Signal.bIsAlphaSignal = bIsAlpha;

    for (UDinosaurCombatAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->ReceivePackSignal(Signal);
        }
    }
}

void UDinosaurCombatAI::ReceivePackSignal(const FCombat_PackSignal& Signal)
{
    LastPackSignal = Signal;

    // Non-alpha follows alpha's signal
    if (Signal.bIsAlphaSignal && !bIsAlpha)
    {
        if (Signal.SignalState == ECombat_DinoState::Hunting)
        {
            TransitionToState(ECombat_DinoState::Hunting);
        }
        else if (Signal.SignalState == ECombat_DinoState::Fleeing)
        {
            TransitionToState(ECombat_DinoState::Fleeing);
        }
    }
}

void UDinosaurCombatAI::AddPackMember(UDinosaurCombatAI* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
    }
}

// ============================================================
// Utility
// ============================================================

float UDinosaurCombatAI::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.f) return 0.f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

FVector UDinosaurCombatAI::GetLastKnownTargetLocation() const
{
    if (!PrimaryTarget) return FVector::ZeroVector;
    for (const FCombat_ThreatEntry& Entry : KnownThreats)
    {
        if (Entry.ThreatActor == PrimaryTarget)
        {
            return Entry.LastKnownLocation;
        }
    }
    return FVector::ZeroVector;
}

void UDinosaurCombatAI::SelectPrimaryTarget()
{
    PrimaryTarget = nullptr;
    float HighestThreat = 0.f;

    for (const FCombat_ThreatEntry& Entry : KnownThreats)
    {
        if (Entry.ThreatActor && Entry.ThreatLevel > HighestThreat)
        {
            HighestThreat = Entry.ThreatLevel;
            PrimaryTarget = Entry.ThreatActor;
        }
    }
}

void UDinosaurCombatAI::DecayThreatEntries(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    float Now = World->GetTimeSeconds();
    const float ThreatDecayRate = 5.f;  // threat units per second
    const float ThreatExpireTime = 30.f; // seconds before entry removed

    for (int32 i = KnownThreats.Num() - 1; i >= 0; --i)
    {
        FCombat_ThreatEntry& Entry = KnownThreats[i];
        Entry.ThreatLevel = FMath::Max(0.f, Entry.ThreatLevel - ThreatDecayRate * DeltaTime);

        // Remove expired or zero-threat entries
        if (Entry.ThreatLevel <= 0.f || (Now - Entry.LastSeenTimestamp) > ThreatExpireTime)
        {
            KnownThreats.RemoveAt(i);
        }
    }
}

bool UDinosaurCombatAI::HasLineOfSightToTarget() const
{
    if (!PrimaryTarget || !GetOwner()) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        GetOwner()->GetActorLocation(),
        PrimaryTarget->GetActorLocation(),
        ECC_Visibility,
        Params
    );

    return !bHit || HitResult.GetActor() == PrimaryTarget;
}

float UDinosaurCombatAI::GetDistanceToTarget() const
{
    if (!PrimaryTarget || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());
}

void UDinosaurCombatAI::TransitionToState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
}

void UDinosaurCombatAI::ApplySpeciesDefaults()
{
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        Stats.MaxHealth = 500.f;
        Stats.CurrentHealth = 500.f;
        Stats.AttackDamage = 80.f;
        Stats.AttackRange = 350.f;
        Stats.DetectionRange = 2500.f;
        Stats.ChaseSpeed = 550.f;
        Stats.PatrolSpeed = 180.f;
        Stats.AttackCooldown = 2.5f;
        Stats.bIsPackHunter = false;
        Stats.FleeHealthThreshold = 0.1f;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Stats.MaxHealth = 80.f;
        Stats.CurrentHealth = 80.f;
        Stats.AttackDamage = 20.f;
        Stats.AttackRange = 150.f;
        Stats.DetectionRange = 1800.f;
        Stats.ChaseSpeed = 750.f;
        Stats.PatrolSpeed = 280.f;
        Stats.AttackCooldown = 0.8f;
        Stats.bIsPackHunter = true;
        Stats.FleeHealthThreshold = 0.25f;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.MaxHealth = 300.f;
        Stats.CurrentHealth = 300.f;
        Stats.AttackDamage = 50.f;
        Stats.AttackRange = 280.f;
        Stats.DetectionRange = 1200.f;
        Stats.ChaseSpeed = 480.f;
        Stats.PatrolSpeed = 160.f;
        Stats.AttackCooldown = 3.0f;
        Stats.bIsPackHunter = false;
        Stats.FleeHealthThreshold = 0.15f;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Stats.MaxHealth = 800.f;
        Stats.CurrentHealth = 800.f;
        Stats.AttackDamage = 40.f;
        Stats.AttackRange = 400.f;
        Stats.DetectionRange = 800.f;
        Stats.ChaseSpeed = 300.f;
        Stats.PatrolSpeed = 120.f;
        Stats.AttackCooldown = 4.0f;
        Stats.bIsPackHunter = false;
        Stats.FleeHealthThreshold = 0.05f;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        Stats.MaxHealth = 60.f;
        Stats.CurrentHealth = 60.f;
        Stats.AttackDamage = 15.f;
        Stats.AttackRange = 200.f;
        Stats.DetectionRange = 3000.f;
        Stats.ChaseSpeed = 900.f;
        Stats.PatrolSpeed = 400.f;
        Stats.AttackCooldown = 1.2f;
        Stats.bIsPackHunter = false;
        Stats.FleeHealthThreshold = 0.3f;
        break;

    default:
        break;
    }
}
