// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Implements survival-based NPC behavior: patrol, alert, flee, shelter, sleep

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic

    CurrentState = ENPC_BehaviorState::Idle;
    NPCRole = ENPC_Role::Gatherer;
    PatrolRadius = 1500.f;
    ThreatDetectionRange = 2000.f;
    FearFleeThreshold = 0.6f;
    FearLevel = 0.f;
    Stamina = 1.f;
    TimeOfDay = 0.5f;
    bSleepsAtNight = true;
    PatrolTimer = 0.f;
    bHasPatrolTarget = false;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache home location at spawn
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Scouts and Guards start patrolling immediately
    if (NPCRole == ENPC_Role::Scout || NPCRole == ENPC_Role::Guard)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    DecayMemory(DeltaTime);
    UpdateBehavior(DeltaTime);

    // Stamina recovery when not fleeing
    if (CurrentState != ENPC_BehaviorState::Flee)
    {
        Stamina = FMath::Min(1.f, Stamina + DeltaTime * 0.05f);
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    switch (NewState)
    {
    case ENPC_BehaviorState::Patrol:
        bHasPatrolTarget = false;
        break;
    case ENPC_BehaviorState::Flee:
        // Burn stamina when fleeing
        Stamina = FMath::Max(0.f, Stamina - 0.2f);
        break;
    case ENPC_BehaviorState::Sleep:
        FearLevel = FMath::Max(0.f, FearLevel - 0.3f);
        break;
    default:
        break;
    }
}

void UNPCBehaviorComponent::RegisterThreat(FVector ThreatLocation, float ThreatMagnitude, bool bIsDinosaur)
{
    Memory.LastKnownThreatLocation = ThreatLocation;
    Memory.ThreatLevel = FMath::Clamp(Memory.ThreatLevel + ThreatMagnitude, 0.f, 1.f);
    Memory.TimeSinceThreat = 0.f;
    Memory.bHasSeenDinosaur = bIsDinosaur;
    Memory.bHasSeenPlayer = !bIsDinosaur;

    // Raise fear proportional to threat magnitude
    FearLevel = FMath::Clamp(FearLevel + ThreatMagnitude * 0.5f, 0.f, 1.f);

    // Immediately alert or flee based on fear
    if (FearLevel >= FearFleeThreshold && Stamina > 0.2f)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (FearLevel > 0.2f)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::DecayMemory(float DeltaTime)
{
    if (Memory.ThreatLevel > 0.f)
    {
        Memory.TimeSinceThreat += DeltaTime;
        // Threat memory decays over 60 seconds
        float DecayRate = 1.f / 60.f;
        Memory.ThreatLevel = FMath::Max(0.f, Memory.ThreatLevel - DecayRate * DeltaTime);
        FearLevel = FMath::Max(0.f, FearLevel - DecayRate * 0.5f * DeltaTime);
    }
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint() const
{
    UWorld* World = GetWorld();
    if (!World) return HomeLocation;

    // Random point within patrol radius using NavMesh
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLoc))
        {
            return NavLoc.Location;
        }
    }

    // Fallback: random offset from home
    float Angle = FMath::RandRange(0.f, 360.f);
    float Dist = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    return HomeLocation + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
        0.f
    );
}

bool UNPCBehaviorComponent::ShouldSleep() const
{
    // Night = TimeOfDay < 0.25 or > 0.75
    return bSleepsAtNight && (TimeOfDay < 0.25f || TimeOfDay > 0.75f);
}

FVector UNPCBehaviorComponent::GetFleeDirection() const
{
    if (AActor* Owner = GetOwner())
    {
        FVector ToThreat = Memory.LastKnownThreatLocation - Owner->GetActorLocation();
        if (ToThreat.SizeSquared() > 1.f)
        {
            // Flee directly away from threat
            return -ToThreat.GetSafeNormal();
        }
    }
    // Default: flee toward home
    if (AActor* Owner = GetOwner())
    {
        return (HomeLocation - Owner->GetActorLocation()).GetSafeNormal();
    }
    return FVector::ForwardVector;
}

void UNPCBehaviorComponent::UpdateBehavior(float DeltaTime)
{
    // Schedule override: sleep at night
    if (ShouldSleep() && CurrentState != ENPC_BehaviorState::Flee && CurrentState != ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Sleep);
        return;
    }

    // Scan for threats every tick (10Hz due to TickInterval)
    ScanForThreats();

    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        PatrolTimer += DeltaTime;
        // After 5-15 seconds idle, start patrolling
        if (PatrolTimer > FMath::RandRange(5.f, 15.f))
        {
            PatrolTimer = 0.f;
            if (NPCRole != ENPC_Role::Elder) // Elders stay near home
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
        }
        break;

    case ENPC_BehaviorState::Patrol:
        if (!bHasPatrolTarget)
        {
            CurrentPatrolTarget = GetNextPatrolPoint();
            bHasPatrolTarget = true;
        }
        // Check if reached patrol target (handled by AI controller via MoveToLocation)
        // Reset after 30 seconds to get a new target
        PatrolTimer += DeltaTime;
        if (PatrolTimer > 30.f)
        {
            PatrolTimer = 0.f;
            bHasPatrolTarget = false;
        }
        break;

    case ENPC_BehaviorState::Alert:
        // If fear decays below threshold, return to patrol
        if (FearLevel < 0.15f)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
        break;

    case ENPC_BehaviorState::Flee:
        // If stamina depleted or fear decays, shelter
        if (Stamina <= 0.05f || FearLevel < 0.3f)
        {
            SetBehaviorState(ENPC_BehaviorState::Shelter);
        }
        break;

    case ENPC_BehaviorState::Shelter:
        // Recover in shelter until fear is gone
        if (FearLevel < 0.1f)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        break;

    case ENPC_BehaviorState::Sleep:
        // Wake up when day returns
        if (!ShouldSleep())
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
        break;

    case ENPC_BehaviorState::Interact:
        // Interaction handled externally by dialogue system
        break;
    }
}

void UNPCBehaviorComponent::ScanForThreats()
{
    UWorld* World = GetWorld();
    AActor* Owner = GetOwner();
    if (!World || !Owner) return;

    FVector OwnerLoc = Owner->GetActorLocation();

    // Sphere overlap for nearby actors
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(ThreatDetectionRange);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    bool bHit = World->OverlapMultiByChannel(
        Overlaps,
        OwnerLoc,
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* OtherActor = Overlap.GetActor();
            if (!OtherActor) continue;

            FString Label = OtherActor->GetActorLabel();
            float DistSq = FVector::DistSquared(OwnerLoc, OtherActor->GetActorLocation());
            float Dist = FMath::Sqrt(DistSq);

            // Dinosaurs are high threats
            bool bIsDino = Label.Contains(TEXT("TRex")) || Label.Contains(TEXT("Raptor")) ||
                           Label.Contains(TEXT("Trike")) || Label.Contains(TEXT("Brachio")) ||
                           Label.Contains(TEXT("Dino")) || Label.Contains(TEXT("Ankylo"));

            if (bIsDino)
            {
                // Threat magnitude inversely proportional to distance
                float ThreatMag = FMath::Clamp(1.f - (Dist / ThreatDetectionRange), 0.f, 1.f);
                // T-Rex is extra dangerous
                if (Label.Contains(TEXT("TRex"))) ThreatMag *= 1.5f;
                ThreatMag = FMath::Clamp(ThreatMag, 0.f, 1.f);

                if (ThreatMag > 0.1f)
                {
                    RegisterThreat(OtherActor->GetActorLocation(), ThreatMag * 0.1f, true);
                }
            }
        }
    }
}
