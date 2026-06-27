#include "DinosaurBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UDinosaurBehaviorComponent::UDinosaurBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz — sufficient for AI

    // Default species traits (T-Rex baseline)
    SpeciesTraits.Species             = ENPC_DinoSpecies::TRex;
    SpeciesTraits.DetectionRadius     = 3000.0f;
    SpeciesTraits.AttackRadius        = 300.0f;
    SpeciesTraits.ChaseRadius         = 5000.0f;
    SpeciesTraits.PatrolRadius        = 3000.0f;
    SpeciesTraits.WalkSpeed           = 300.0f;
    SpeciesTraits.RunSpeed            = 700.0f;
    SpeciesTraits.SprintSpeed         = 1200.0f;
    SpeciesTraits.bIsPredator         = true;
    SpeciesTraits.bIsPackHunter       = false;
    SpeciesTraits.MaxPackSize         = 1;
    SpeciesTraits.DefaultThreatResponse = ENPC_ThreatResponse::Attack;
    SpeciesTraits.MemoryDuration      = 30.0f;
    SpeciesTraits.RoarCooldown        = 15.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwner())
    {
        PatrolOrigin = GetOwner()->GetActorLocation();
    }

    CurrentState  = ENPC_DinoAIState::Idle;
    PreviousState = ENPC_DinoAIState::Idle;
    StateEnteredTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentState == ENPC_DinoAIState::Dead)
    {
        return;
    }

    TickThreatMemory(DeltaTime);
    EvaluateThreatResponse();
}

// ─────────────────────────────────────────────────────────────────────────────
// State Machine
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::SetAIState(ENPC_DinoAIState NewState)
{
    if (NewState == CurrentState)
    {
        return;
    }

    OnStateExited(CurrentState);
    PreviousState = CurrentState;
    CurrentState  = NewState;
    StateEnteredTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    OnStateEntered(NewState);
}

void UDinosaurBehaviorComponent::OnStateEntered(ENPC_DinoAIState NewState)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (!OwnerChar || !OwnerChar->GetCharacterMovement())
    {
        return;
    }

    UCharacterMovementComponent* Movement = OwnerChar->GetCharacterMovement();

    switch (NewState)
    {
    case ENPC_DinoAIState::Idle:
    case ENPC_DinoAIState::Resting:
    case ENPC_DinoAIState::Feeding:
        Movement->MaxWalkSpeed = SpeciesTraits.WalkSpeed * 0.3f;
        break;

    case ENPC_DinoAIState::Patrol:
    case ENPC_DinoAIState::Investigate:
        Movement->MaxWalkSpeed = SpeciesTraits.WalkSpeed;
        break;

    case ENPC_DinoAIState::Alert:
        Movement->MaxWalkSpeed = SpeciesTraits.RunSpeed * 0.5f;
        break;

    case ENPC_DinoAIState::Chase:
        Movement->MaxWalkSpeed = SpeciesTraits.RunSpeed;
        break;

    case ENPC_DinoAIState::Attack:
        Movement->MaxWalkSpeed = SpeciesTraits.SprintSpeed;
        break;

    case ENPC_DinoAIState::Flee:
        Movement->MaxWalkSpeed = SpeciesTraits.SprintSpeed;
        break;

    case ENPC_DinoAIState::Dead:
        Movement->MaxWalkSpeed = 0.0f;
        Movement->DisableMovement();
        break;

    default:
        break;
    }
}

void UDinosaurBehaviorComponent::OnStateExited(ENPC_DinoAIState OldState)
{
    if (OldState == ENPC_DinoAIState::Attack)
    {
        bIsRoaring = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Threat Memory
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
    {
        return;
    }

    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Update existing entry
    for (FNPC_DinoMemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.ThreatLevel       = FMath::Max(Entry.ThreatLevel, ThreatLevel);
            Entry.LastKnownLocation = ThreatActor->GetActorLocation();
            Entry.TimeStamp         = Now;
            Entry.bIsActive         = true;
            return;
        }
    }

    // Add new entry
    FNPC_DinoMemoryEntry NewEntry;
    NewEntry.ThreatActor       = ThreatActor;
    NewEntry.LastKnownLocation = ThreatActor->GetActorLocation();
    NewEntry.ThreatLevel       = ThreatLevel;
    NewEntry.TimeStamp         = Now;
    NewEntry.bIsActive         = true;
    ThreatMemory.Add(NewEntry);
}

void UDinosaurBehaviorComponent::ClearThreat(AActor* ThreatActor)
{
    ThreatMemory.RemoveAll([ThreatActor](const FNPC_DinoMemoryEntry& Entry)
    {
        return Entry.ThreatActor == ThreatActor;
    });
}

bool UDinosaurBehaviorComponent::HasActiveThreat() const
{
    for (const FNPC_DinoMemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.bIsActive && Entry.ThreatLevel > 0.0f)
        {
            return true;
        }
    }
    return false;
}

AActor* UDinosaurBehaviorComponent::GetPrimaryThreat() const
{
    AActor* BestThreat = nullptr;
    float   BestLevel  = -1.0f;

    for (const FNPC_DinoMemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.bIsActive && Entry.ThreatLevel > BestLevel)
        {
            BestLevel  = Entry.ThreatLevel;
            BestThreat = Entry.ThreatActor;
        }
    }
    return BestThreat;
}

void UDinosaurBehaviorComponent::TickThreatMemory(float DeltaTime)
{
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (FNPC_DinoMemoryEntry& Entry : ThreatMemory)
    {
        float Age = Now - Entry.TimeStamp;
        if (Age > SpeciesTraits.MemoryDuration)
        {
            Entry.bIsActive   = false;
            Entry.ThreatLevel = 0.0f;
        }
        else
        {
            // Decay threat level over time
            float DecayRate = 1.0f / SpeciesTraits.MemoryDuration;
            Entry.ThreatLevel = FMath::Max(0.0f, Entry.ThreatLevel - DecayRate * DeltaTime * 10.0f);
        }
    }

    // Remove fully expired entries
    ThreatMemory.RemoveAll([](const FNPC_DinoMemoryEntry& Entry)
    {
        return !Entry.bIsActive && Entry.ThreatLevel <= 0.0f;
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// Threat Evaluation
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::EvaluateThreatResponse()
{
    if (!HasActiveThreat())
    {
        if (CurrentState == ENPC_DinoAIState::Chase ||
            CurrentState == ENPC_DinoAIState::Alert ||
            CurrentState == ENPC_DinoAIState::Investigate)
        {
            SetAIState(ENPC_DinoAIState::Patrol);
        }
        return;
    }

    AActor* PrimaryThreat = GetPrimaryThreat();
    if (!PrimaryThreat || !GetOwner())
    {
        return;
    }

    float DistToThreat = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryThreat->GetActorLocation());
    FNPC_DinoMemoryEntry* ThreatEntry = nullptr;

    for (FNPC_DinoMemoryEntry& Entry : ThreatMemory)
    {
        if (Entry.ThreatActor == PrimaryThreat)
        {
            ThreatEntry = &Entry;
            break;
        }
    }

    if (!ThreatEntry)
    {
        return;
    }

    ENPC_ThreatResponse Response = ClassifyThreat(ThreatEntry->ThreatLevel);

    switch (Response)
    {
    case ENPC_ThreatResponse::Ignore:
        if (CurrentState != ENPC_DinoAIState::Idle)
        {
            SetAIState(ENPC_DinoAIState::Idle);
        }
        break;

    case ENPC_ThreatResponse::Investigate:
        if (CurrentState == ENPC_DinoAIState::Idle || CurrentState == ENPC_DinoAIState::Patrol)
        {
            SetAIState(ENPC_DinoAIState::Investigate);
        }
        break;

    case ENPC_ThreatResponse::Flee:
        SetAIState(ENPC_DinoAIState::Flee);
        break;

    case ENPC_ThreatResponse::Attack:
        if (DistToThreat <= SpeciesTraits.AttackRadius)
        {
            SetAIState(ENPC_DinoAIState::Attack);
        }
        else if (DistToThreat <= SpeciesTraits.ChaseRadius)
        {
            SetAIState(ENPC_DinoAIState::Chase);
        }
        else
        {
            SetAIState(ENPC_DinoAIState::Alert);
        }
        break;

    case ENPC_ThreatResponse::CallPack:
        BroadcastPackAlert(PrimaryThreat, 5000.0f);
        SetAIState(ENPC_DinoAIState::Chase);
        break;

    default:
        break;
    }
}

ENPC_ThreatResponse UDinosaurBehaviorComponent::ClassifyThreat(float ThreatLevel) const
{
    if (!SpeciesTraits.bIsPredator)
    {
        // Herbivores flee or ignore
        if (ThreatLevel > 0.7f) return ENPC_ThreatResponse::Flee;
        if (ThreatLevel > 0.3f) return ENPC_ThreatResponse::Investigate;
        return ENPC_ThreatResponse::Ignore;
    }

    // Predators attack or investigate
    if (ThreatLevel > 0.5f)
    {
        if (SpeciesTraits.bIsPackHunter) return ENPC_ThreatResponse::CallPack;
        return ENPC_ThreatResponse::Attack;
    }
    if (ThreatLevel > 0.2f) return ENPC_ThreatResponse::Investigate;
    return ENPC_ThreatResponse::Ignore;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pack Behavior
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::BroadcastPackAlert(AActor* ThreatActor, float Radius)
{
    if (!ThreatActor || !GetOwner() || !GetWorld())
    {
        return;
    }

    FVector Origin = GetOwner()->GetActorLocation();
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), NearbyActors);

    for (AActor* NearbyActor : NearbyActors)
    {
        if (NearbyActor == GetOwner())
        {
            continue;
        }

        float Dist = FVector::Dist(Origin, NearbyActor->GetActorLocation());
        if (Dist > Radius)
        {
            continue;
        }

        UDinosaurBehaviorComponent* PackMember = NearbyActor->FindComponentByClass<UDinosaurBehaviorComponent>();
        if (PackMember && PackMember->SpeciesTraits.Species == SpeciesTraits.Species)
        {
            PackMember->OnPackAlertReceived(ThreatActor, ThreatActor->GetActorLocation());
        }
    }
}

void UDinosaurBehaviorComponent::OnPackAlertReceived(AActor* ThreatActor, FVector ThreatLocation)
{
    if (!ThreatActor)
    {
        return;
    }

    RegisterThreat(ThreatActor, 0.8f);
    SetAIState(ENPC_DinoAIState::Alert);
}

// ─────────────────────────────────────────────────────────────────────────────
// Patrol
// ─────────────────────────────────────────────────────────────────────────────

FVector UDinosaurBehaviorComponent::GetNextPatrolPoint()
{
    // Generate deterministic patrol points around origin in a polygon pattern
    const int32 NumPoints = 6;
    float AngleStep = 360.0f / NumPoints;
    float Angle     = FMath::DegreesToRadians(PatrolPointIndex * AngleStep);

    float Radius = SpeciesTraits.PatrolRadius * FMath::RandRange(0.5f, 1.0f);
    FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

    PatrolPointIndex = (PatrolPointIndex + 1) % NumPoints;
    return PatrolOrigin + Offset;
}

// ─────────────────────────────────────────────────────────────────────────────
// Roar
// ─────────────────────────────────────────────────────────────────────────────

bool UDinosaurBehaviorComponent::TryRoar()
{
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (Now - LastRoarTime < SpeciesTraits.RoarCooldown)
    {
        return false;
    }

    bIsRoaring   = true;
    LastRoarTime = Now;

    // Auto-clear roar flag after 3 seconds (animation driven)
    FTimerHandle RoarTimer;
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(RoarTimer, [this]()
        {
            bIsRoaring = false;
        }, 3.0f, false);
    }

    return true;
}
