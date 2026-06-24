#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz tick for performance
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    InitializeSpeciesStats();
    SetState(ECombat_DinoState::Patrolling);
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    DecayThreatList(DeltaTime);
    ScanForThreats();
    UpdateCombatState(DeltaTime);
}

void UDinosaurCombatAI::InitializeSpeciesStats()
{
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        Stats.MaxHealth = 2000.0f;
        Stats.CurrentHealth = 2000.0f;
        Stats.AttackDamage = 250.0f;
        Stats.AttackRange = 400.0f;
        Stats.DetectionRadius = 3000.0f;
        Stats.ChaseSpeed = 700.0f;
        Stats.PatrolSpeed = 250.0f;
        Stats.AttackCooldown = 3.0f;
        Stats.bIsPack = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Stats.MaxHealth = 400.0f;
        Stats.CurrentHealth = 400.0f;
        Stats.AttackDamage = 80.0f;
        Stats.AttackRange = 200.0f;
        Stats.DetectionRadius = 2500.0f;
        Stats.ChaseSpeed = 1100.0f;
        Stats.PatrolSpeed = 400.0f;
        Stats.AttackCooldown = 1.0f;
        Stats.bIsPack = true;
        Stats.PackSize = 3;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.MaxHealth = 1500.0f;
        Stats.CurrentHealth = 1500.0f;
        Stats.AttackDamage = 180.0f;
        Stats.AttackRange = 350.0f;
        Stats.DetectionRadius = 1500.0f;
        Stats.ChaseSpeed = 600.0f;
        Stats.PatrolSpeed = 200.0f;
        Stats.AttackCooldown = 2.5f;
        Stats.bIsPack = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Stats.MaxHealth = 3000.0f;
        Stats.CurrentHealth = 3000.0f;
        Stats.AttackDamage = 100.0f;
        Stats.AttackRange = 500.0f;
        Stats.DetectionRadius = 1000.0f;
        Stats.ChaseSpeed = 400.0f;
        Stats.PatrolSpeed = 150.0f;
        Stats.AttackCooldown = 4.0f;
        Stats.bIsPack = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Ankylosaurus:
        Stats.MaxHealth = 1800.0f;
        Stats.CurrentHealth = 1800.0f;
        Stats.AttackDamage = 200.0f;
        Stats.AttackRange = 300.0f;
        Stats.DetectionRadius = 1200.0f;
        Stats.ChaseSpeed = 450.0f;
        Stats.PatrolSpeed = 180.0f;
        Stats.AttackCooldown = 3.0f;
        Stats.bIsPack = false;
        Stats.PackSize = 1;
        break;

    default:
        break;
    }
}

void UDinosaurCombatAI::RegisterThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor) return;

    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.ThreatLevel = FMath::Max(Entry.ThreatLevel, ThreatLevel);
            Entry.LastSeenTime = GetWorld()->GetTimeSeconds();
            Entry.LastKnownLocation = ThreatActor->GetActorLocation();
            return;
        }
    }

    FCombat_ThreatEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewEntry.LastKnownLocation = ThreatActor->GetActorLocation();
    ThreatList.Add(NewEntry);
}

void UDinosaurCombatAI::ClearThreat(AActor* ThreatActor)
{
    ThreatList.RemoveAll([ThreatActor](const FCombat_ThreatEntry& Entry) {
        return Entry.ThreatActor == ThreatActor;
    });
}

void UDinosaurCombatAI::ApplyDamage(float DamageAmount)
{
    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    // When damaged, escalate to chase/attack
    if (Stats.CurrentHealth > 0.0f && CurrentState == ECombat_DinoState::Patrolling)
    {
        SetState(ECombat_DinoState::Alerted);
    }

    // Flee when critically wounded (below 20% health)
    if (Stats.CurrentHealth < Stats.MaxHealth * 0.2f)
    {
        SetState(ECombat_DinoState::Fleeing);
    }
}

bool UDinosaurCombatAI::IsHostile() const
{
    return CurrentState == ECombat_DinoState::Chasing ||
           CurrentState == ECombat_DinoState::Attacking ||
           CurrentState == ECombat_DinoState::Alerted;
}

AActor* UDinosaurCombatAI::GetHighestThreat() const
{
    AActor* HighestThreatActor = nullptr;
    float HighestLevel = 0.0f;

    for (const FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor && Entry.ThreatLevel > HighestLevel)
        {
            HighestLevel = Entry.ThreatLevel;
            HighestThreatActor = Entry.ThreatActor;
        }
    }

    return HighestThreatActor;
}

void UDinosaurCombatAI::SetState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateEntryTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UDinosaurCombatAI::ExecuteAttack(AActor* Target)
{
    if (!Target) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < Stats.AttackCooldown) return;

    LastAttackTime = CurrentTime;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(Target, Stats.AttackDamage, nullptr, GetOwner(), nullptr);
}

void UDinosaurCombatAI::UpdateCombatState(float DeltaTime)
{
    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:
        // Transition to patrol after 3 seconds
        if (GetWorld()->GetTimeSeconds() - StateEntryTime > 3.0f)
        {
            SetState(ECombat_DinoState::Patrolling);
        }
        break;

    case ECombat_DinoState::Patrolling:
        UpdatePatrol(DeltaTime);
        break;

    case ECombat_DinoState::Alerted:
    {
        AActor* Threat = GetHighestThreat();
        if (Threat)
        {
            SetState(ECombat_DinoState::Chasing);
        }
        else if (GetWorld()->GetTimeSeconds() - StateEntryTime > 5.0f)
        {
            SetState(ECombat_DinoState::Patrolling);
        }
        break;
    }

    case ECombat_DinoState::Chasing:
        UpdateChase(DeltaTime);
        break;

    case ECombat_DinoState::Attacking:
        UpdateAttack(DeltaTime);
        break;

    case ECombat_DinoState::Fleeing:
        // Flee logic — move away from threats
        if (ThreatList.Num() == 0)
        {
            SetState(ECombat_DinoState::Idle);
        }
        break;

    default:
        break;
    }
}

void UDinosaurCombatAI::UpdatePatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector CurrentLocation = Owner->GetActorLocation();
    FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];

    float DistToTarget = FVector::Dist(CurrentLocation, TargetPoint);
    if (DistToTarget < 200.0f)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }
}

void UDinosaurCombatAI::UpdateChase(float DeltaTime)
{
    AActor* Target = GetHighestThreat();
    if (!Target)
    {
        SetState(ECombat_DinoState::Patrolling);
        return;
    }

    if (IsInAttackRange(Target))
    {
        SetState(ECombat_DinoState::Attacking);
        return;
    }

    // Update last known location
    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor == Target)
        {
            Entry.LastKnownLocation = Target->GetActorLocation();
            break;
        }
    }
}

void UDinosaurCombatAI::UpdateAttack(float DeltaTime)
{
    AActor* Target = GetHighestThreat();
    if (!Target)
    {
        SetState(ECombat_DinoState::Patrolling);
        return;
    }

    if (!IsInAttackRange(Target))
    {
        SetState(ECombat_DinoState::Chasing);
        return;
    }

    ExecuteAttack(Target);
}

void UDinosaurCombatAI::ScanForThreats()
{
    AActor* Owner = GetOwner();
    if (!Owner || !GetWorld()) return;

    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor == Owner) continue;

        float Distance = FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= Stats.DetectionRadius)
        {
            float ThreatLevel = 1.0f - (Distance / Stats.DetectionRadius);
            RegisterThreat(Actor, ThreatLevel * 100.0f);

            if (CurrentState == ECombat_DinoState::Patrolling || CurrentState == ECombat_DinoState::Idle)
            {
                SetState(ECombat_DinoState::Alerted);
            }
        }
    }
}

void UDinosaurCombatAI::DecayThreatList(float DeltaTime)
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    const float ThreatMemoryDuration = 30.0f;

    ThreatList.RemoveAll([CurrentTime, ThreatMemoryDuration](const FCombat_ThreatEntry& Entry) {
        return (CurrentTime - Entry.LastSeenTime) > ThreatMemoryDuration;
    });
}

bool UDinosaurCombatAI::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetOwner()) return false;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= Stats.AttackRange;
}
