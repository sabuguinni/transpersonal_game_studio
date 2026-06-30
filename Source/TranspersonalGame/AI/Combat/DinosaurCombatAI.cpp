// DinosaurCombatAI.cpp
// Combat & Enemy AI Agent #12 — Transpersonal Game Studio
// Tactical combat state machine for dinosaur enemies.

#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    InitSpeciesPreset(Species);
    CurrentState = ECombat_DinoState::Idle;
    PreviousState = ECombat_DinoState::Idle;
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive()) return;

    AttackCooldown = FMath::Max(0.f, AttackCooldown - DeltaTime);
    StateTimer += DeltaTime;

    UpdateThreatList(DeltaTime);
    SelectPrimaryTarget();
    UpdateStateMachine(DeltaTime);
}

// ============================================================
// Species Presets
// ============================================================
void UDinosaurCombatAI::InitSpeciesPreset(ECombat_DinoSpecies InSpecies)
{
    Species = InSpecies;
    switch (InSpecies)
    {
    case ECombat_DinoSpecies::TRex:
        Stats.MaxHealth = 500.f;
        Stats.CurrentHealth = 500.f;
        Stats.AttackDamage = 80.f;
        Stats.DetectionRadius = 2000.f;
        Stats.AttackRadius = 300.f;
        Stats.ChargeSpeed = 900.f;
        Stats.PatrolSpeed = 250.f;
        Stats.FleeHealthThreshold = 0.1f;
        Stats.bIsPackHunter = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Stats.MaxHealth = 120.f;
        Stats.CurrentHealth = 120.f;
        Stats.AttackDamage = 30.f;
        Stats.DetectionRadius = 1800.f;
        Stats.AttackRadius = 150.f;
        Stats.ChargeSpeed = 1200.f;
        Stats.PatrolSpeed = 400.f;
        Stats.FleeHealthThreshold = 0.25f;
        Stats.bIsPackHunter = true;
        Stats.PackSize = 3;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.MaxHealth = 350.f;
        Stats.CurrentHealth = 350.f;
        Stats.AttackDamage = 50.f;
        Stats.DetectionRadius = 1000.f;
        Stats.AttackRadius = 250.f;
        Stats.ChargeSpeed = 700.f;
        Stats.PatrolSpeed = 180.f;
        Stats.FleeHealthThreshold = 0.15f;
        Stats.bIsPackHunter = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Stats.MaxHealth = 800.f;
        Stats.CurrentHealth = 800.f;
        Stats.AttackDamage = 40.f;
        Stats.DetectionRadius = 800.f;
        Stats.AttackRadius = 400.f;
        Stats.ChargeSpeed = 500.f;
        Stats.PatrolSpeed = 150.f;
        Stats.FleeHealthThreshold = 0.05f;
        Stats.bIsPackHunter = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        Stats.MaxHealth = 80.f;
        Stats.CurrentHealth = 80.f;
        Stats.AttackDamage = 20.f;
        Stats.DetectionRadius = 2500.f;
        Stats.AttackRadius = 120.f;
        Stats.ChargeSpeed = 1500.f;
        Stats.PatrolSpeed = 600.f;
        Stats.FleeHealthThreshold = 0.3f;
        Stats.bIsPackHunter = true;
        Stats.PackSize = 5;
        break;
    }
}

// ============================================================
// Threat System
// ============================================================
void UDinosaurCombatAI::RegisterThreat(AActor* ThreatActor, float Score)
{
    if (!ThreatActor) return;

    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.ThreatScore = FMath::Max(Entry.ThreatScore, Score);
            Entry.LastSeenTime = GetWorld()->GetTimeSeconds();
            Entry.LastKnownLocation = ThreatActor->GetActorLocation();
            AggressionLevel = FMath::Min(1.f, AggressionLevel + Score * 0.1f);
            return;
        }
    }

    FCombat_ThreatEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.ThreatScore = Score;
    NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewEntry.LastKnownLocation = ThreatActor->GetActorLocation();
    ThreatList.Add(NewEntry);
    AggressionLevel = FMath::Min(1.f, AggressionLevel + Score * 0.1f);
}

void UDinosaurCombatAI::UpdateThreatList(float DeltaTime)
{
    float Now = GetWorld()->GetTimeSeconds();
    AggressionLevel = FMath::Max(0.f, AggressionLevel - DeltaTime * 0.05f);

    ThreatList.RemoveAll([&](const FCombat_ThreatEntry& Entry) {
        return !IsValid(Entry.ThreatActor) || (Now - Entry.LastSeenTime) > ThreatExpireTime;
    });

    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        Entry.ThreatScore = FMath::Max(0.f, Entry.ThreatScore - ThreatDecayRate * DeltaTime);
    }
}

void UDinosaurCombatAI::SelectPrimaryTarget()
{
    AActor* BestTarget = nullptr;
    float BestScore = 0.f;

    for (const FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatScore > BestScore && IsValid(Entry.ThreatActor))
        {
            BestScore = Entry.ThreatScore;
            BestTarget = Entry.ThreatActor;
        }
    }

    PrimaryTarget = BestTarget;
}

float UDinosaurCombatAI::ComputeThreatScore(AActor* Actor) const
{
    if (!Actor || !GetOwner()) return 0.f;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
    float DistScore = FMath::Clamp(1.f - (Dist / Stats.DetectionRadius), 0.f, 1.f);
    return DistScore * 100.f;
}

// ============================================================
// State Machine
// ============================================================
void UDinosaurCombatAI::TransitionToState(ECombat_DinoState NewState)
{
    if (NewState == CurrentState) return;
    PreviousState = CurrentState;
    CurrentState = NewState;
    StateTimer = 0.f;
}

void UDinosaurCombatAI::UpdateStateMachine(float DeltaTime)
{
    // Global transitions
    if (!IsAlive()) return;

    float HealthPct = GetHealthPercent();

    if (HealthPct <= Stats.FleeHealthThreshold && CurrentState != ECombat_DinoState::Fleeing)
    {
        TransitionToState(ECombat_DinoState::Fleeing);
    }

    if (PrimaryTarget && CurrentState == ECombat_DinoState::Idle)
    {
        TransitionToState(Stats.bIsPackHunter ? ECombat_DinoState::PackHunt : ECombat_DinoState::Stalking);
    }

    if (!PrimaryTarget && (CurrentState == ECombat_DinoState::Stalking || CurrentState == ECombat_DinoState::Charging))
    {
        TransitionToState(ECombat_DinoState::Idle);
    }

    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:       ExecuteStateIdle(DeltaTime);       break;
    case ECombat_DinoState::Stalking:   ExecuteStateStalking(DeltaTime);   break;
    case ECombat_DinoState::Charging:   ExecuteStateCharging(DeltaTime);   break;
    case ECombat_DinoState::Attacking:  ExecuteStateAttacking(DeltaTime);  break;
    case ECombat_DinoState::Retreating: ExecuteStateRetreating(DeltaTime); break;
    case ECombat_DinoState::Fleeing:    ExecuteStateFleeing(DeltaTime);    break;
    case ECombat_DinoState::PackHunt:   ExecuteStatePackHunt(DeltaTime);   break;
    }
}

void UDinosaurCombatAI::ExecuteStateIdle(float DeltaTime)
{
    // Scan for threats via overlap check
    if (!GetOwner()) return;
    TArray<AActor*> OverlappingActors;
    GetOwner()->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
    for (AActor* A : OverlappingActors)
    {
        RegisterThreat(A, ComputeThreatScore(A));
    }
}

void UDinosaurCombatAI::ExecuteStateStalking(float DeltaTime)
{
    if (!PrimaryTarget || !GetOwner()) return;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());

    if (Dist < Stats.AttackRadius * 2.f)
    {
        TransitionToState(ECombat_DinoState::Charging);
    }
    else if (StateTimer > 5.f && AggressionLevel > 0.6f)
    {
        TransitionToState(ECombat_DinoState::Charging);
    }
}

void UDinosaurCombatAI::ExecuteStateCharging(float DeltaTime)
{
    if (!PrimaryTarget || !GetOwner()) return;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());

    if (Dist <= Stats.AttackRadius)
    {
        TransitionToState(ECombat_DinoState::Attacking);
    }
    else if (!PrimaryTarget)
    {
        TransitionToState(ECombat_DinoState::Retreating);
    }
}

void UDinosaurCombatAI::ExecuteStateAttacking(float DeltaTime)
{
    if (!PrimaryTarget || !GetOwner()) return;

    if (AttackCooldown <= 0.f)
    {
        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());
        if (Dist <= Stats.AttackRadius)
        {
            UGameplayStatics::ApplyDamage(PrimaryTarget, Stats.AttackDamage, nullptr, GetOwner(), nullptr);
            AttackCooldown = 1.5f;
        }
        else
        {
            TransitionToState(ECombat_DinoState::Charging);
        }
    }

    if (StateTimer > 8.f)
    {
        TransitionToState(ECombat_DinoState::Retreating);
    }
}

void UDinosaurCombatAI::ExecuteStateRetreating(float DeltaTime)
{
    if (StateTimer > 3.f)
    {
        if (PrimaryTarget)
            TransitionToState(ECombat_DinoState::Stalking);
        else
            TransitionToState(ECombat_DinoState::Idle);
    }
}

void UDinosaurCombatAI::ExecuteStateFleeing(float DeltaTime)
{
    // Stay in flee state until health recovers above threshold (handled externally via healing)
    if (GetHealthPercent() > Stats.FleeHealthThreshold + 0.1f && StateTimer > 10.f)
    {
        TransitionToState(ECombat_DinoState::Idle);
    }
}

void UDinosaurCombatAI::ExecuteStatePackHunt(float DeltaTime)
{
    if (!PrimaryTarget || !GetOwner()) return;

    if (bIsPackLeader)
    {
        NotifyPackOfTarget(PrimaryTarget);
    }

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());
    if (Dist <= Stats.AttackRadius)
    {
        TransitionToState(ECombat_DinoState::Attacking);
    }
    else if (Dist <= Stats.AttackRadius * 3.f)
    {
        TransitionToState(ECombat_DinoState::Charging);
    }
}

void UDinosaurCombatAI::NotifyPackOfTarget(AActor* Target)
{
    for (UDinosaurCombatAI* Member : PackMembers)
    {
        if (IsValid(Member) && Member != this)
        {
            Member->RegisterThreat(Target, 80.f);
            if (Member->CurrentState == ECombat_DinoState::Idle)
            {
                Member->TransitionToState(ECombat_DinoState::PackHunt);
            }
        }
    }
}

// ============================================================
// Damage & Pack
// ============================================================
void UDinosaurCombatAI::TakeCombatDamage(float Damage, AActor* DamageSource)
{
    Stats.CurrentHealth = FMath::Max(0.f, Stats.CurrentHealth - Damage);

    if (DamageSource)
    {
        RegisterThreat(DamageSource, 90.f);
    }

    if (!IsAlive())
    {
        TransitionToState(ECombat_DinoState::Fleeing);
    }
    else if (GetHealthPercent() <= Stats.FleeHealthThreshold)
    {
        TransitionToState(ECombat_DinoState::Fleeing);
    }
    else if (CurrentState == ECombat_DinoState::Idle)
    {
        TransitionToState(ECombat_DinoState::Charging);
    }
}

void UDinosaurCombatAI::JoinPack(UDinosaurCombatAI* PackLeader)
{
    if (!PackLeader || PackLeader == this) return;
    if (!PackLeader->PackMembers.Contains(this))
    {
        PackLeader->PackMembers.Add(this);
    }
    bIsPackLeader = false;
}

float UDinosaurCombatAI::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.f) return 0.f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}
