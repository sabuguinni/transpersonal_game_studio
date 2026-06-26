#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ── Constructor ──
UCombat_DinosaurCombatAI::UCombat_DinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI — performance conscious

    // Default traits for T-Rex
    SpeciesTraits = BuildTraitsForSpecies(ECombat_DinoSpecies::TRex);
}

void UCombat_DinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    SetSpecies(DinoSpecies);
    CurrentState = ECombat_DinoState::Patrol;
}

void UCombat_DinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    if (AttackCooldown > 0.0f)
    {
        AttackCooldown -= DeltaTime;
    }

    UpdateThreatDecay(DeltaTime);
    UpdateStateMachine(DeltaTime);
}

// ── State Machine ──
void UCombat_DinosaurCombatAI::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.0f;
}

void UCombat_DinosaurCombatAI::UpdateStateMachine(float DeltaTime)
{
    AActor* HighestThreat = GetHighestThreat();

    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:
        // Transition to patrol after brief idle
        if (StateTimer > 3.0f)
        {
            SetCombatState(ECombat_DinoState::Patrol);
        }
        break;

    case ECombat_DinoState::Patrol:
        // If threat detected, alert
        if (HighestThreat && GetThreatLevel(HighestThreat) > 10.0f)
        {
            CurrentTarget = HighestThreat;
            SetCombatState(ECombat_DinoState::Alerted);
        }
        break;

    case ECombat_DinoState::Alerted:
        if (!HighestThreat)
        {
            SetCombatState(ECombat_DinoState::Patrol);
            break;
        }
        CurrentTarget = HighestThreat;
        // Pack hunters stalk first; apex predators charge directly
        if (SpeciesTraits.bIsPackHunter)
        {
            if (StateTimer > 2.0f) SetCombatState(ECombat_DinoState::Stalking);
        }
        else
        {
            if (StateTimer > 1.0f) SetCombatState(ECombat_DinoState::Charging);
        }
        break;

    case ECombat_DinoState::Stalking:
        if (!CurrentTarget)
        {
            SetCombatState(ECombat_DinoState::Patrol);
            break;
        }
        // Signal pack before committing to attack
        if (bIsAlpha && StateTimer > 3.0f)
        {
            SignalPackAttack(CurrentTarget);
            SetCombatState(ECombat_DinoState::Charging);
        }
        else if (!bIsAlpha && StateTimer > 5.0f)
        {
            SetCombatState(ECombat_DinoState::Charging);
        }
        break;

    case ECombat_DinoState::Charging:
        if (!CurrentTarget)
        {
            SetCombatState(ECombat_DinoState::Patrol);
            break;
        }
        {
            float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistToTarget <= SpeciesTraits.AttackRange)
            {
                SetCombatState(ECombat_DinoState::Attacking);
            }
            // Flee if health critically low
            if (CurrentHealth / MaxHealth < SpeciesTraits.FleeHealthThreshold)
            {
                SetCombatState(ECombat_DinoState::Fleeing);
            }
        }
        break;

    case ECombat_DinoState::Attacking:
        if (AttackCooldown <= 0.0f && CurrentTarget)
        {
            ExecuteAttack(CurrentTarget);
            AttackCooldown = 1.5f; // 1.5s between attacks
        }
        // Return to charging if target moved away
        if (CurrentTarget)
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (Dist > SpeciesTraits.AttackRange * 1.5f)
            {
                SetCombatState(ECombat_DinoState::Charging);
            }
        }
        break;

    case ECombat_DinoState::Fleeing:
        // Flee for 10 seconds then reassess
        if (StateTimer > 10.0f)
        {
            SetCombatState(ECombat_DinoState::Patrol);
        }
        break;

    case ECombat_DinoState::Feeding:
        if (StateTimer > 8.0f)
        {
            Hunger = FMath::Max(0.0f, Hunger - 30.0f);
            SetCombatState(ECombat_DinoState::Idle);
        }
        break;

    default:
        break;
    }
}

// ── Threat System ──
void UCombat_DinosaurCombatAI::RegisterThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor) return;

    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.ThreatLevel = FMath::Max(Entry.ThreatLevel, ThreatLevel);
            Entry.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            Entry.LastKnownLocation = ThreatActor->GetActorLocation();
            return;
        }
    }

    // New threat
    FCombat_ThreatEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewEntry.LastKnownLocation = ThreatActor->GetActorLocation();
    ThreatList.Add(NewEntry);
}

AActor* UCombat_DinosaurCombatAI::GetHighestThreat() const
{
    AActor* Best = nullptr;
    float BestLevel = 0.0f;

    for (const FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor && Entry.ThreatLevel > BestLevel)
        {
            BestLevel = Entry.ThreatLevel;
            Best = Entry.ThreatActor;
        }
    }
    return Best;
}

float UCombat_DinosaurCombatAI::GetThreatLevel(AActor* Actor) const
{
    for (const FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor == Actor)
        {
            return Entry.ThreatLevel;
        }
    }
    return 0.0f;
}

void UCombat_DinosaurCombatAI::UpdateThreatDecay(float DeltaTime)
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    for (int32 i = ThreatList.Num() - 1; i >= 0; --i)
    {
        float TimeSinceSeen = CurrentTime - ThreatList[i].LastSeenTime;
        // Threats decay after 30 seconds of no contact
        if (TimeSinceSeen > 30.0f)
        {
            ThreatList[i].ThreatLevel -= DeltaTime * 2.0f;
            if (ThreatList[i].ThreatLevel <= 0.0f)
            {
                ThreatList.RemoveAt(i);
            }
        }
    }
}

// ── Combat Actions ──
void UCombat_DinosaurCombatAI::InitiateCharge(AActor* Target)
{
    if (!Target) return;
    CurrentTarget = Target;
    SetCombatState(ECombat_DinoState::Charging);
}

void UCombat_DinosaurCombatAI::ExecuteAttack(AActor* Target)
{
    if (!Target || !GetWorld()) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        SpeciesTraits.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );

    // Debug visualization in editor
#if WITH_EDITOR
    DrawDebugSphere(
        GetWorld(),
        Target->GetActorLocation(),
        80.0f,
        12,
        FColor::Red,
        false,
        1.0f
    );
#endif
}

void UCombat_DinosaurCombatAI::BeginStalking(AActor* Target)
{
    if (!Target) return;
    CurrentTarget = Target;
    SetCombatState(ECombat_DinoState::Stalking);
}

void UCombat_DinosaurCombatAI::AttemptFlee()
{
    CurrentTarget = nullptr;
    SetCombatState(ECombat_DinoState::Fleeing);
}

// ── Pack Coordination ──
void UCombat_DinosaurCombatAI::CoordinateWithPackMember(UCombat_DinosaurCombatAI* PackMember)
{
    if (!PackMember) return;
    if (!PackMembers.Contains(PackMember))
    {
        PackMembers.Add(PackMember);
    }
}

void UCombat_DinosaurCombatAI::SignalPackAttack(AActor* Target)
{
    if (!Target) return;
    for (UCombat_DinosaurCombatAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->RegisterThreat(Target, 80.0f);
            Member->InitiateCharge(Target);
        }
    }
}

// ── Species Config ──
void UCombat_DinosaurCombatAI::SetSpecies(ECombat_DinoSpecies Species)
{
    DinoSpecies = Species;
    SpeciesTraits = BuildTraitsForSpecies(Species);
}

FCombat_SpeciesTraits UCombat_DinosaurCombatAI::BuildTraitsForSpecies(ECombat_DinoSpecies Species) const
{
    FCombat_SpeciesTraits Traits;

    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        Traits.SightRadius = 3000.0f;
        Traits.HearingRadius = 2500.0f;
        Traits.AttackRange = 350.0f;
        Traits.AttackDamage = 120.0f;
        Traits.MoveSpeed = 700.0f;
        Traits.ChargeSpeed = 1400.0f;
        Traits.bIsPackHunter = false;
        Traits.FleeHealthThreshold = 0.1f; // T-Rex rarely flees
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Traits.SightRadius = 2000.0f;
        Traits.HearingRadius = 1800.0f;
        Traits.AttackRange = 150.0f;
        Traits.AttackDamage = 35.0f;
        Traits.MoveSpeed = 900.0f;
        Traits.ChargeSpeed = 1600.0f;
        Traits.bIsPackHunter = true;
        Traits.FleeHealthThreshold = 0.3f;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Traits.SightRadius = 1500.0f;
        Traits.HearingRadius = 1200.0f;
        Traits.AttackRange = 280.0f;
        Traits.AttackDamage = 80.0f;
        Traits.MoveSpeed = 500.0f;
        Traits.ChargeSpeed = 1000.0f;
        Traits.bIsPackHunter = false;
        Traits.FleeHealthThreshold = 0.15f;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Traits.SightRadius = 2500.0f;
        Traits.HearingRadius = 2000.0f;
        Traits.AttackRange = 400.0f;
        Traits.AttackDamage = 60.0f; // Stomp damage
        Traits.MoveSpeed = 300.0f;
        Traits.ChargeSpeed = 600.0f;
        Traits.bIsPackHunter = false;
        Traits.FleeHealthThreshold = 0.25f;
        break;

    case ECombat_DinoSpecies::Ankylosaurus:
        Traits.SightRadius = 1200.0f;
        Traits.HearingRadius = 1000.0f;
        Traits.AttackRange = 220.0f;
        Traits.AttackDamage = 90.0f; // Tail club
        Traits.MoveSpeed = 350.0f;
        Traits.ChargeSpeed = 700.0f;
        Traits.bIsPackHunter = false;
        Traits.FleeHealthThreshold = 0.05f; // Heavily armoured — almost never flees
        break;

    default:
        break;
    }

    return Traits;
}
