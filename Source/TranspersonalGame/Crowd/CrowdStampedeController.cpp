// CrowdStampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements stampede trigger, wave propagation, and LOD-aware agent panic spreading

#include "CrowdStampedeController.h"
#include "CrowdSimulationManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UCrowdStampedeController::UCrowdStampedeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    StampedeRadius = 2500.0f;
    PanicPropagationSpeed = 800.0f;
    StampedeDecayTime = 12.0f;
    MaxStampedeAgents = 200;
    bStampedeActive = false;
    CurrentPanicLevel = 0.0f;
    StampedeTriggerThreshold = 0.65f;
}

void UCrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();
}

void UCrowdStampedeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bStampedeActive)
    {
        TickStampedeWave(DeltaTime);
    }
}

void UCrowdStampedeController::TriggerStampede(FVector Origin, ECrowd_StampedeCause Cause)
{
    if (bStampedeActive)
    {
        // Amplify existing stampede if new trigger is stronger
        CurrentPanicLevel = FMath::Min(1.0f, CurrentPanicLevel + 0.3f);
        StampedeOrigins.Add(Origin);
        return;
    }

    bStampedeActive = true;
    CurrentPanicLevel = GetInitialPanicLevel(Cause);
    StampedeOrigins.Add(Origin);
    StampedeCause = Cause;
    StampedeStartTime = GetWorld()->GetTimeSeconds();

    // Notify all nearby agents
    PropagateStampedePanic(Origin, CurrentPanicLevel);

    // Schedule decay
    FTimerHandle DecayHandle;
    GetWorld()->GetTimerManager().SetTimer(DecayHandle, this, &UCrowdStampedeController::DecayStampede, StampedeDecayTime, false);

    UE_LOG(LogTemp, Log, TEXT("CrowdStampede: Triggered at (%.0f, %.0f, %.0f) Cause=%d PanicLevel=%.2f"),
        Origin.X, Origin.Y, Origin.Z, (int32)Cause, CurrentPanicLevel);
}

void UCrowdStampedeController::PropagateStampedePanic(FVector Origin, float PanicLevel)
{
    if (!GetOwner()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Find all crowd agents within radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(StampedeRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    World->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Pawn, Sphere, Params);

    int32 AffectedCount = 0;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (AffectedCount >= MaxStampedeAgents) break;

        AActor* HitActor = Overlap.GetActor();
        if (!HitActor) continue;

        float Distance = FVector::Dist(Origin, HitActor->GetActorLocation());
        float DistanceFactor = 1.0f - FMath::Clamp(Distance / StampedeRadius, 0.0f, 1.0f);
        float AgentPanic = PanicLevel * DistanceFactor;

        if (AgentPanic >= StampedeTriggerThreshold)
        {
            // Apply panic to agent — broadcast event
            OnAgentPanicked.Broadcast(HitActor, AgentPanic, Origin);
            AffectedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdStampede: Propagated panic to %d agents within %.0f units"), AffectedCount, StampedeRadius);
}

void UCrowdStampedeController::TickStampedeWave(float DeltaTime)
{
    if (!bStampedeActive) return;

    float Elapsed = GetWorld()->GetTimeSeconds() - StampedeStartTime;
    float DecayFactor = 1.0f - FMath::Clamp(Elapsed / StampedeDecayTime, 0.0f, 1.0f);
    CurrentPanicLevel = FMath::Max(0.0f, CurrentPanicLevel * DecayFactor);

    // Expand wave front
    for (const FVector& Origin : StampedeOrigins)
    {
        float WaveRadius = PanicPropagationSpeed * Elapsed;
        float WaveThickness = 300.0f;

        // Debug visualization in editor
#if WITH_EDITOR
        DrawDebugCircle(GetWorld(), Origin, WaveRadius, 32,
            FColor::Orange, false, -1.0f, 0, 5.0f, FVector(1,0,0), FVector(0,1,0));
#endif
    }

    if (CurrentPanicLevel < 0.05f)
    {
        EndStampede();
    }
}

void UCrowdStampedeController::DecayStampede()
{
    EndStampede();
}

void UCrowdStampedeController::EndStampede()
{
    bStampedeActive = false;
    CurrentPanicLevel = 0.0f;
    StampedeOrigins.Empty();
    OnStampedeEnded.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("CrowdStampede: Stampede ended — all agents returning to normal behavior"));
}

float UCrowdStampedeController::GetInitialPanicLevel(ECrowd_StampedeCause Cause) const
{
    switch (Cause)
    {
        case ECrowd_StampedeCause::PredatorSighted:     return 0.75f;
        case ECrowd_StampedeCause::LoudNoise:           return 0.55f;
        case ECrowd_StampedeCause::Earthquake:          return 0.90f;
        case ECrowd_StampedeCause::FireOrSmoke:         return 0.80f;
        case ECrowd_StampedeCause::PlayerProximity:     return 0.45f;
        case ECrowd_StampedeCause::PackLeaderDeath:     return 0.85f;
        default:                                         return 0.60f;
    }
}

bool UCrowdStampedeController::IsStampedeActive() const
{
    return bStampedeActive;
}

float UCrowdStampedeController::GetCurrentPanicLevel() const
{
    return CurrentPanicLevel;
}
