#include "AI/NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ============================================================
// UNPCBehaviorComponent — Implementation
// Agent #11 — NPC Behavior Agent
// ============================================================

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Store home location at spawn
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Start in idle state
    CurrentState = ENPC_BehaviorState::Idle;
    AlertLevel = ENPC_AlertLevel::Calm;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    TimeSinceLastStateChange += DeltaTime;

    ForgetOldMemories();
    EvaluateThreats();
    UpdateAlertLevel(DeltaTime);
    UpdateStateMachine(DeltaTime);
}

// ── State Machine ──────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    TimeSinceLastStateChange = 0.0f;
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::UpdateStateMachine(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        // After 3-6 seconds idle, start patrolling
        if (StateTimer > FMath::RandRange(3.0f, 6.0f))
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        // If alarmed, investigate
        if (AlertLevel >= ENPC_AlertLevel::Suspicious && HasMemoryOf(ENPC_MemoryType::ThreatLocation))
        {
            SetBehaviorState(ENPC_BehaviorState::Investigate);
        }
        break;

    case ENPC_BehaviorState::Patrol:
        // Select patrol point if needed
        if (!bHasPatrolTarget)
        {
            SelectNewPatrolPoint();
        }
        // Wait at waypoint
        PatrolWaitTimer += DeltaTime;
        if (PatrolWaitTimer >= BehaviorConfig.PatrolWaitTime)
        {
            bHasPatrolTarget = false;
            PatrolWaitTimer = 0.0f;
        }
        // Escalate if threat detected
        if (AlertLevel >= ENPC_AlertLevel::Alarmed)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigate);
        }
        break;

    case ENPC_BehaviorState::Investigate:
        // Move toward last known threat location
        if (AlertLevel >= ENPC_AlertLevel::Hostile && CurrentTarget)
        {
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
        // Calm down after 15 seconds with no new stimuli
        if (StateTimer > 15.0f && AlertLevel <= ENPC_AlertLevel::Curious)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        break;

    case ENPC_BehaviorState::Chase:
        if (!CurrentTarget)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigate);
            break;
        }
        {
            float DistToTarget = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());
            // Attack if close enough
            if (DistToTarget <= BehaviorConfig.AttackRange)
            {
                SetBehaviorState(ENPC_BehaviorState::Attack);
            }
            // Give up chase if target too far
            if (DistToTarget > BehaviorConfig.ChaseRange * 1.5f)
            {
                CurrentTarget = nullptr;
                SetBehaviorState(ENPC_BehaviorState::Investigate);
            }
        }
        break;

    case ENPC_BehaviorState::Attack:
        if (!CurrentTarget)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
            break;
        }
        {
            float DistToTarget = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistToTarget > BehaviorConfig.AttackRange * 1.5f)
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
        }
        break;

    case ENPC_BehaviorState::Flee:
        // Flee for 10 seconds then reassess
        if (StateTimer > 10.0f)
        {
            AlertLevel = ENPC_AlertLevel::Suspicious;
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        break;

    case ENPC_BehaviorState::Roar:
        // Roar lasts 2.5 seconds, then transition to chase
        if (StateTimer > 2.5f)
        {
            if (CurrentTarget)
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
        }
        break;

    case ENPC_BehaviorState::Feeding:
        // Feed for 8-15 seconds, very alert while feeding
        if (StateTimer > FMath::RandRange(8.0f, 15.0f))
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        if (AlertLevel >= ENPC_AlertLevel::Alarmed)
        {
            SetBehaviorState(ENPC_BehaviorState::Flee);
        }
        break;

    case ENPC_BehaviorState::Resting:
        // Rest for 20-40 seconds
        if (StateTimer > FMath::RandRange(20.0f, 40.0f))
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        if (AlertLevel >= ENPC_AlertLevel::Suspicious)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        break;

    case ENPC_BehaviorState::Social:
        // Social interaction lasts 5-10 seconds
        if (StateTimer > FMath::RandRange(5.0f, 10.0f))
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        break;
    }
}

void UNPCBehaviorComponent::UpdateAlertLevel(float DeltaTime)
{
    AlertDecayTimer += DeltaTime;

    // Alert decays over time when no new threats
    if (AlertDecayTimer > 5.0f && AlertLevel > ENPC_AlertLevel::Calm)
    {
        AlertDecayTimer = 0.0f;
        uint8 CurrentLevel = static_cast<uint8>(AlertLevel);
        if (CurrentLevel > 0)
        {
            AlertLevel = static_cast<ENPC_AlertLevel>(CurrentLevel - 1);
        }
    }
}

void UNPCBehaviorComponent::SelectNewPatrolPoint()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Random point within patrol radius of home
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = FMath::RandRange(500.0f, BehaviorConfig.PatrolRadius);
    float RadAngle = FMath::DegreesToRadians(Angle);

    CurrentPatrolTarget = HomeLocation + FVector(
        FMath::Cos(RadAngle) * Distance,
        FMath::Sin(RadAngle) * Distance,
        0.0f
    );
    bHasPatrolTarget = true;
}

void UNPCBehaviorComponent::EvaluateThreats()
{
    // Look for player pawn
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    if (CanSeeTarget(PlayerPawn))
    {
        OnTargetDetected(PlayerPawn, ENPC_MemoryType::PlayerSighting);
        AlertLevel = ENPC_AlertLevel::Hostile;
        AlertDecayTimer = 0.0f;
        CurrentTarget = PlayerPawn;
    }
}

bool UNPCBehaviorComponent::IsWithinPatrolBounds() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return true;
    return FVector::Dist(Owner->GetActorLocation(), HomeLocation) <= BehaviorConfig.PatrolRadius;
}

// ── Perception ─────────────────────────────────────────────────────────────

bool UNPCBehaviorComponent::CanSeeTarget(AActor* Target) const
{
    if (!Target) return false;
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    FVector OwnerLoc = Owner->GetActorLocation();
    FVector TargetLoc = Target->GetActorLocation();
    float Distance = FVector::Dist(OwnerLoc, TargetLoc);

    if (Distance > BehaviorConfig.SightRange) return false;

    // Angle check
    FVector ToTarget = (TargetLoc - OwnerLoc).GetSafeNormal();
    FVector Forward = Owner->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(Forward, ToTarget);
    float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    if (AngleDeg > BehaviorConfig.SightAngleDegrees * 0.5f) return false;

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);
    bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        HitResult, OwnerLoc + FVector(0, 0, 50), TargetLoc + FVector(0, 0, 50),
        ECC_Visibility, Params
    );

    return !bBlocked || HitResult.GetActor() == Target;
}

bool UNPCBehaviorComponent::CanHearTarget(AActor* Target, float NoiseLevel) const
{
    if (!Target) return false;
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
    float EffectiveHearingRange = BehaviorConfig.HearingRange * NoiseLevel;
    return Distance <= EffectiveHearingRange;
}

void UNPCBehaviorComponent::OnTargetDetected(AActor* Target, ENPC_MemoryType DetectionType)
{
    if (!Target) return;
    AddMemory(DetectionType, Target->GetActorLocation(), Target, 1.0f);

    // Escalate alert
    if (AlertLevel < ENPC_AlertLevel::Hostile)
    {
        uint8 CurrentLevel = static_cast<uint8>(AlertLevel);
        AlertLevel = static_cast<ENPC_AlertLevel>(FMath::Min(CurrentLevel + 1, 4));
    }
    AlertDecayTimer = 0.0f;
}

void UNPCBehaviorComponent::OnNoiseMade(FVector NoiseLocation, float NoiseRadius, float Loudness)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Distance = FVector::Dist(Owner->GetActorLocation(), NoiseLocation);
    float EffectiveRange = BehaviorConfig.HearingRange * Loudness;

    if (Distance <= EffectiveRange)
    {
        AddMemory(ENPC_MemoryType::ThreatLocation, NoiseLocation, nullptr, Loudness * (1.0f - Distance / EffectiveRange));
        if (AlertLevel < ENPC_AlertLevel::Suspicious)
        {
            AlertLevel = ENPC_AlertLevel::Suspicious;
        }
        AlertDecayTimer = 0.0f;
    }
}

// ── Memory ─────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::AddMemory(ENPC_MemoryType Type, FVector Location, AActor* Source, float Confidence)
{
    FNPC_MemoryEntry Entry;
    Entry.MemoryType = Type;
    Entry.Location = Location;
    Entry.SourceActor = Source;
    Entry.Confidence = Confidence;
    Entry.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    MemoryBank.Add(Entry);

    // Trim to max entries
    while (MemoryBank.Num() > BehaviorConfig.MaxMemoryEntries)
    {
        MemoryBank.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::ForgetOldMemories()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    MemoryBank.RemoveAll([this, CurrentTime](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.Timestamp) > BehaviorConfig.MemoryDuration;
    });
}

bool UNPCBehaviorComponent::HasMemoryOf(ENPC_MemoryType Type) const
{
    for (const FNPC_MemoryEntry& Entry : MemoryBank)
    {
        if (Entry.MemoryType == Type) return true;
    }
    return false;
}

FVector UNPCBehaviorComponent::GetLastKnownThreatLocation() const
{
    float MostRecent = -1.0f;
    FVector LastLoc = FVector::ZeroVector;

    for (const FNPC_MemoryEntry& Entry : MemoryBank)
    {
        if ((Entry.MemoryType == ENPC_MemoryType::PlayerSighting ||
             Entry.MemoryType == ENPC_MemoryType::ThreatLocation) &&
            Entry.Timestamp > MostRecent)
        {
            MostRecent = Entry.Timestamp;
            LastLoc = Entry.Location;
        }
    }
    return LastLoc;
}

// ── Pack Behavior ──────────────────────────────────────────────────────────

void UNPCBehaviorComponent::AlertPackMembers(FVector ThreatLocation)
{
    if (!BehaviorConfig.bIsPackAnimal) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Find nearby actors with NPC behavior components
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    for (AActor* NearbyActor : NearbyActors)
    {
        if (NearbyActor == Owner) continue;

        float Distance = FVector::Dist(Owner->GetActorLocation(), NearbyActor->GetActorLocation());
        if (Distance > BehaviorConfig.PackCommunicationRange) continue;

        UNPCBehaviorComponent* PackComp = NearbyActor->FindComponentByClass<UNPCBehaviorComponent>();
        if (PackComp && PackComp->BehaviorConfig.bIsPackAnimal)
        {
            PackComp->ReceivePackAlert(ThreatLocation, Owner);
        }
    }
}

void UNPCBehaviorComponent::ReceivePackAlert(FVector ThreatLocation, AActor* AlertSource)
{
    AddMemory(ENPC_MemoryType::ThreatLocation, ThreatLocation, AlertSource, 0.8f);

    if (AlertLevel < ENPC_AlertLevel::Alarmed)
    {
        AlertLevel = ENPC_AlertLevel::Alarmed;
        AlertDecayTimer = 0.0f;
    }

    if (CurrentState == ENPC_BehaviorState::Idle || CurrentState == ENPC_BehaviorState::Patrol)
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
    }
}

// ── Daily Routine ──────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateDailyRoutine(float TimeOfDay)
{
    // TimeOfDay: 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    // Predators more active at dawn (0.15-0.25) and dusk (0.7-0.8)
    bool bIsDawnOrDusk = (TimeOfDay >= 0.15f && TimeOfDay <= 0.25f) ||
                         (TimeOfDay >= 0.70f && TimeOfDay <= 0.80f);
    bool bIsNight = TimeOfDay < 0.1f || TimeOfDay > 0.9f;

    if (bIsNight && CurrentState == ENPC_BehaviorState::Idle)
    {
        // Nocturnal rest during deep night
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            SetBehaviorState(ENPC_BehaviorState::Resting);
        }
    }
    else if (bIsDawnOrDusk && CurrentState == ENPC_BehaviorState::Resting)
    {
        // Wake up at dawn/dusk
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}
