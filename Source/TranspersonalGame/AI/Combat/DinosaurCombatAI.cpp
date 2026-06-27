#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ============================================================
// UDinosaurCombatAI — Implementation
// Combat & Enemy AI Agent #12
// Tactical AI: state machine, pack hunting, flanking, retreat
// ============================================================

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance

    // Default to T-Rex traits
    SpeciesTraits = GetTRexTraits();
    CurrentHealth = SpeciesTraits.MaxHealth;
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    HomeLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    CurrentHealth = SpeciesTraits.MaxHealth;
    CurrentState = ECombat_DinoState::Patrolling;
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentState == ECombat_DinoState::Dead) return;

    StateTimer += DeltaTime;
    ThreatEvalTimer += DeltaTime;

    // Evaluate threats every 0.5s
    if (ThreatEvalTimer >= 0.5f)
    {
        ThreatEvalTimer = 0.f;
        PruneStaleThreatEntries();
        EvaluateThreatList();
    }

    // State machine dispatch
    switch (CurrentState)
    {
        case ECombat_DinoState::Idle:
        case ECombat_DinoState::Patrolling:
            TickIdle(DeltaTime);
            break;
        case ECombat_DinoState::Stalking:
        case ECombat_DinoState::Investigating:
            TickStalking(DeltaTime);
            break;
        case ECombat_DinoState::Flanking:
            TickFlanking(DeltaTime);
            break;
        case ECombat_DinoState::Charging:
            TickCharging(DeltaTime);
            break;
        case ECombat_DinoState::Attacking:
            TickAttacking(DeltaTime);
            break;
        case ECombat_DinoState::Retreating:
        case ECombat_DinoState::Fleeing:
            TickRetreating(DeltaTime);
            break;
        default:
            break;
    }
}

// ── State Tick Implementations ─────────────────────────────

void UDinosaurCombatAI::TickIdle(float DeltaTime)
{
    if (!PrimaryTarget) return;

    float Dist = GetDistanceToTarget();

    // Transition to stalking when target enters detection radius
    if (Dist < SpeciesTraits.DetectionRadius)
    {
        if (SpeciesTraits.bIsPackHunter)
        {
            BroadcastPackAlert(PrimaryTarget);
            SetCombatState(ECombat_DinoState::Flanking);
        }
        else
        {
            SetCombatState(ECombat_DinoState::Stalking);
        }
    }
}

void UDinosaurCombatAI::TickStalking(float DeltaTime)
{
    if (!PrimaryTarget)
    {
        SetCombatState(ECombat_DinoState::Patrolling);
        return;
    }

    float Dist = GetDistanceToTarget();

    // Stalk for configured duration then charge
    if (StateTimer >= SpeciesTraits.StalkDuration)
    {
        SetCombatState(ECombat_DinoState::Charging);
        return;
    }

    // If target gets too close during stalk, attack immediately
    if (Dist < SpeciesTraits.AttackRadius)
    {
        SetCombatState(ECombat_DinoState::Attacking);
    }
}

void UDinosaurCombatAI::TickFlanking(float DeltaTime)
{
    if (!PrimaryTarget)
    {
        SetCombatState(ECombat_DinoState::Patrolling);
        return;
    }

    float Dist = GetDistanceToTarget();

    // Once in attack range after flanking, strike
    if (Dist < SpeciesTraits.AttackRadius)
    {
        SetCombatState(ECombat_DinoState::Attacking);
        return;
    }

    // Flanking timeout — fall back to charge
    if (StateTimer >= 8.f)
    {
        SetCombatState(ECombat_DinoState::Charging);
    }
}

void UDinosaurCombatAI::TickCharging(float DeltaTime)
{
    if (!PrimaryTarget)
    {
        SetCombatState(ECombat_DinoState::Patrolling);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist < SpeciesTraits.AttackRadius)
    {
        SetCombatState(ECombat_DinoState::Attacking);
    }
}

void UDinosaurCombatAI::TickAttacking(float DeltaTime)
{
    if (!PrimaryTarget)
    {
        SetCombatState(ECombat_DinoState::Patrolling);
        return;
    }

    // Attempt attack every 1.5s while in attack state
    if (StateTimer >= 1.5f)
    {
        AttemptAttack();
        StateTimer = 0.f;
    }

    // Check retreat condition
    if (GetHealthPercent() < SpeciesTraits.RetreatHealthThreshold)
    {
        SetCombatState(ECombat_DinoState::Retreating);
    }
}

void UDinosaurCombatAI::TickRetreating(float DeltaTime)
{
    // Move back toward home territory
    // Actual movement handled by AIController/NavMesh
    // Transition to Idle once far enough from threat
    if (PrimaryTarget)
    {
        float Dist = GetDistanceToTarget();
        if (Dist > SpeciesTraits.DetectionRadius * 1.5f)
        {
            PrimaryTarget = nullptr;
            SetCombatState(ECombat_DinoState::Idle);
        }
    }
    else
    {
        SetCombatState(ECombat_DinoState::Idle);
    }
}

// ── Threat System ──────────────────────────────────────────

void UDinosaurCombatAI::RegisterThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor) return;

    // Update existing entry or add new
    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.ThreatLevel = FMath::Max(Entry.ThreatLevel, ThreatLevel);
            Entry.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
            Entry.LastKnownLocation = ThreatActor->GetActorLocation();
            return;
        }
    }

    FCombat_ThreatEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    NewEntry.LastKnownLocation = ThreatActor->GetActorLocation();
    ThreatList.Add(NewEntry);
}

void UDinosaurCombatAI::EvaluateThreatList()
{
    if (ThreatList.IsEmpty()) return;

    // Select highest threat as primary target
    FCombat_ThreatEntry* HighestThreat = nullptr;
    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (!HighestThreat || Entry.ThreatLevel > HighestThreat->ThreatLevel)
        {
            HighestThreat = &Entry;
        }
    }

    if (HighestThreat && HighestThreat->ThreatActor)
    {
        PrimaryTarget = HighestThreat->ThreatActor;

        // Transition out of idle states when threat is registered
        if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrolling)
        {
            if (SpeciesTraits.bIsPackHunter && PackMembers.Num() > 0)
            {
                SelectFlankRole();
                SetCombatState(ECombat_DinoState::Flanking);
            }
            else
            {
                SetCombatState(ECombat_DinoState::Stalking);
            }
        }
    }
}

void UDinosaurCombatAI::PruneStaleThreatEntries()
{
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    const float StaleThreshold = 30.f; // Forget threats after 30s without sight

    ThreatList.RemoveAll([Now, StaleThreshold](const FCombat_ThreatEntry& Entry)
    {
        return (Now - Entry.LastSeenTime) > StaleThreshold || !IsValid(Entry.ThreatActor);
    });
}

// ── Combat Actions ─────────────────────────────────────────

void UDinosaurCombatAI::TakeDamage_Combat(float DamageAmount, AActor* DamageSource)
{
    CurrentHealth = FMath::Max(0.f, CurrentHealth - DamageAmount);

    // Register attacker as high-priority threat
    if (DamageSource)
    {
        RegisterThreat(DamageSource, 100.f);
    }

    if (CurrentHealth <= 0.f)
    {
        SetCombatState(ECombat_DinoState::Dead);
        return;
    }

    // Panic response — immediately charge attacker
    if (CurrentState != ECombat_DinoState::Attacking && CurrentState != ECombat_DinoState::Charging)
    {
        SetCombatState(ECombat_DinoState::Charging);
    }

    // Retreat if health critically low
    if (GetHealthPercent() < SpeciesTraits.RetreatHealthThreshold)
    {
        SetCombatState(ECombat_DinoState::Retreating);
    }
}

void UDinosaurCombatAI::AttemptAttack()
{
    if (!PrimaryTarget || !GetOwner()) return;

    float Dist = GetDistanceToTarget();
    if (Dist > SpeciesTraits.AttackRadius) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        PrimaryTarget,
        SpeciesTraits.BaseDamage,
        nullptr,
        GetOwner(),
        nullptr
    );
}

// ── Pack Coordination ──────────────────────────────────────

void UDinosaurCombatAI::JoinPack(AActor* PackLeader)
{
    if (!PackLeader) return;
    if (!PackMembers.Contains(PackLeader))
    {
        PackMembers.Add(PackLeader);
    }
}

void UDinosaurCombatAI::BroadcastPackAlert(AActor* Threat)
{
    for (AActor* Member : PackMembers)
    {
        if (!IsValid(Member)) continue;
        UDinosaurCombatAI* MemberAI = Member->FindComponentByClass<UDinosaurCombatAI>();
        if (MemberAI)
        {
            MemberAI->RegisterThreat(Threat, 80.f);
        }
    }
}

void UDinosaurCombatAI::SelectFlankRole()
{
    // Assign flank roles based on pack position index
    // Role 0: Direct charge, Role 1: Flank left, Role 2: Flank right, Role 3+: Surround
    PackRoleIndex = PackMembers.IndexOfByKey(GetOwner());
    bIsFlankLeft = (PackRoleIndex % 2 == 1);
}

// ── Utility ────────────────────────────────────────────────

void UDinosaurCombatAI::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.f;
}

float UDinosaurCombatAI::GetHealthPercent() const
{
    if (SpeciesTraits.MaxHealth <= 0.f) return 0.f;
    return CurrentHealth / SpeciesTraits.MaxHealth;
}

bool UDinosaurCombatAI::IsHostile() const
{
    return CurrentState == ECombat_DinoState::Charging
        || CurrentState == ECombat_DinoState::Attacking
        || CurrentState == ECombat_DinoState::Flanking;
}

float UDinosaurCombatAI::GetDistanceToTarget() const
{
    if (!PrimaryTarget || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());
}

bool UDinosaurCombatAI::HasLineOfSightToTarget() const
{
    if (!PrimaryTarget || !GetOwner() || !GetWorld()) return false;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        Hit,
        GetOwner()->GetActorLocation(),
        PrimaryTarget->GetActorLocation(),
        ECC_Visibility,
        Params
    );

    return !bBlocked || Hit.GetActor() == PrimaryTarget;
}

FVector UDinosaurCombatAI::CalculateFlankPosition(AActor* Target, float SideOffset) const
{
    if (!Target || !GetOwner()) return FVector::ZeroVector;

    FVector ToTarget = (Target->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector Right = FVector::CrossProduct(ToTarget, FVector::UpVector);

    float Side = bIsFlankLeft ? -1.f : 1.f;
    return Target->GetActorLocation() + (Right * Side * SideOffset) - (ToTarget * SpeciesTraits.AttackRadius * 0.8f);
}

// ── Static Species Presets ─────────────────────────────────

FCombat_DinoSpeciesTraits UDinosaurCombatAI::GetTRexTraits()
{
    FCombat_DinoSpeciesTraits T;
    T.Species = ECombat_DinoSpecies::TRex;
    T.DetectionRadius = 3000.f;
    T.AttackRadius = 400.f;
    T.ChargeSpeed = 900.f;
    T.PatrolSpeed = 180.f;
    T.BaseDamage = 120.f;
    T.MaxHealth = 1500.f;
    T.bIsPackHunter = false;
    T.bCanFlank = false;
    T.RetreatHealthThreshold = 0.1f; // T-Rex almost never retreats
    T.StalkDuration = 3.f;
    T.TerritoryRadius = 8000.f;
    return T;
}

FCombat_DinoSpeciesTraits UDinosaurCombatAI::GetRaptorTraits()
{
    FCombat_DinoSpeciesTraits T;
    T.Species = ECombat_DinoSpecies::Raptor;
    T.DetectionRadius = 2500.f;
    T.AttackRadius = 200.f;
    T.ChargeSpeed = 1200.f;
    T.PatrolSpeed = 350.f;
    T.BaseDamage = 40.f;
    T.MaxHealth = 300.f;
    T.bIsPackHunter = true;
    T.bCanFlank = true;
    T.FlankAngleDeg = 120.f;
    T.RetreatHealthThreshold = 0.25f;
    T.StalkDuration = 6.f;
    T.TerritoryRadius = 6000.f;
    return T;
}

FCombat_DinoSpeciesTraits UDinosaurCombatAI::GetTriceratopsTraits()
{
    FCombat_DinoSpeciesTraits T;
    T.Species = ECombat_DinoSpecies::Triceratops;
    T.DetectionRadius = 1800.f;
    T.AttackRadius = 350.f;
    T.ChargeSpeed = 700.f;
    T.PatrolSpeed = 150.f;
    T.BaseDamage = 80.f;
    T.MaxHealth = 1200.f;
    T.bIsPackHunter = false;
    T.bCanFlank = false;
    T.RetreatHealthThreshold = 0.15f;
    T.StalkDuration = 4.f;
    T.TerritoryRadius = 4000.f;
    return T;
}
