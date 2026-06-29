// CrowdStampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Stampede trigger, panic propagation, and escape logic

#include "CrowdStampedeController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowdStampedeController::UCrowdStampedeController()
{
    PrimaryComponentTick.bCanEverTick = true;

    PanicPropagationSpeed   = 800.f;
    PanicDecayRate          = 0.15f;
    StampedeSpeedMultiplier = 2.8f;
    MinPanicThreshold       = 0.6f;
    PlayerDangerRadius      = 250.f;

    CurrentPhase        = ECrowd_StampedePhase::Idle;
    PanickingAgentCount = 0;
}

void UCrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();
}

void UCrowdStampedeController::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentPhase == ECrowd_StampedePhase::Idle || Agents.Num() == 0)
    {
        return;
    }

    ActiveEvent.TimeElapsed += DeltaTime;

    TickPanicPropagation(DeltaTime);
    TickPhaseTransitions();
    TickAgentMovement(DeltaTime);
}

// ─── Public API ──────────────────────────────────────────────────────────────

void UCrowdStampedeController::TriggerStampede(ECrowd_StampedeTrigger Trigger,
                                                FVector Epicentre, float Intensity)
{
    ActiveEvent.Trigger           = Trigger;
    ActiveEvent.EpicentreLocation = Epicentre;
    ActiveEvent.Intensity         = FMath::Clamp(Intensity, 0.f, 1.f);
    ActiveEvent.PropagationRadius = 2000.f * Intensity;
    ActiveEvent.TimeElapsed       = 0.f;

    CurrentPhase = ECrowd_StampedePhase::Panic;

    // Immediately panic agents near epicentre
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Location, Epicentre);
        if (Dist < 600.f)
        {
            Agent.PanicLevel   = Intensity;
            Agent.bIsPanicking = true;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] Triggered: %d agents, Intensity=%.2f, Epicentre=%s"),
           Agents.Num(), Intensity, *Epicentre.ToString());
}

void UCrowdStampedeController::RegisterAgent(FVector Location, int32 Index)
{
    FCrowd_StampedeAgent NewAgent;
    NewAgent.Location   = Location;
    NewAgent.AgentIndex = Index;
    NewAgent.PanicLevel = 0.f;
    NewAgent.bIsPanicking = false;
    Agents.Add(NewAgent);
}

void UCrowdStampedeController::ClearAllAgents()
{
    Agents.Empty();
    PanickingAgentCount = 0;
    CurrentPhase        = ECrowd_StampedePhase::Idle;
}

bool UCrowdStampedeController::IsPlayerInDanger(FVector PlayerLocation) const
{
    if (CurrentPhase != ECrowd_StampedePhase::Running)
    {
        return false;
    }

    for (const FCrowd_StampedeAgent& Agent : Agents)
    {
        if (Agent.bIsPanicking)
        {
            float Dist = FVector::Dist(Agent.Location, PlayerLocation);
            if (Dist < PlayerDangerRadius)
            {
                return true;
            }
        }
    }
    return false;
}

float UCrowdStampedeController::GetHerdPanicLevel() const
{
    if (Agents.Num() == 0) return 0.f;

    float Total = 0.f;
    for (const FCrowd_StampedeAgent& Agent : Agents)
    {
        Total += Agent.PanicLevel;
    }
    return Total / static_cast<float>(Agents.Num());
}

void UCrowdStampedeController::ForceSettle()
{
    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        Agent.PanicLevel   = 0.f;
        Agent.bIsPanicking = false;
        Agent.Velocity     = FVector::ZeroVector;
    }
    CurrentPhase        = ECrowd_StampedePhase::Settled;
    PanickingAgentCount = 0;
}

// ─── Private Tick Helpers ────────────────────────────────────────────────────

void UCrowdStampedeController::TickPanicPropagation(float DeltaTime)
{
    int32 NewPanicking = 0;

    for (int32 i = 0; i < Agents.Num(); ++i)
    {
        FCrowd_StampedeAgent& AgentA = Agents[i];

        if (!AgentA.bIsPanicking)
        {
            // Check if a nearby panicking agent infects this one
            for (int32 j = 0; j < Agents.Num(); ++j)
            {
                if (i == j) continue;
                const FCrowd_StampedeAgent& AgentB = Agents[j];
                if (!AgentB.bIsPanicking) continue;

                float Dist = FVector::Dist(AgentA.Location, AgentB.Location);
                float InfectionRange = PanicPropagationSpeed * DeltaTime * 0.5f + 300.f;

                if (Dist < InfectionRange)
                {
                    float InfectionStrength = FMath::Clamp(1.f - (Dist / InfectionRange), 0.f, 1.f);
                    AgentA.PanicLevel += InfectionStrength * AgentB.PanicLevel * DeltaTime * 2.f;
                    AgentA.PanicLevel  = FMath::Clamp(AgentA.PanicLevel, 0.f, 1.f);

                    if (AgentA.PanicLevel >= MinPanicThreshold)
                    {
                        AgentA.bIsPanicking = true;
                    }
                }
            }
        }
        else
        {
            // Decay panic if trigger is old
            if (ActiveEvent.TimeElapsed > 5.f)
            {
                AgentA.PanicLevel -= PanicDecayRate * DeltaTime;
                AgentA.PanicLevel  = FMath::Max(AgentA.PanicLevel, 0.f);
                if (AgentA.PanicLevel < 0.1f)
                {
                    AgentA.bIsPanicking = false;
                }
            }
            ++NewPanicking;
        }
    }

    PanickingAgentCount = NewPanicking;
}

void UCrowdStampedeController::TickPhaseTransitions()
{
    float HerdPanic = GetHerdPanicLevel();

    switch (CurrentPhase)
    {
    case ECrowd_StampedePhase::Panic:
        if (HerdPanic > 0.75f)
        {
            CurrentPhase = ECrowd_StampedePhase::Running;
            UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] Phase -> Running (panic=%.2f)"), HerdPanic);
        }
        break;

    case ECrowd_StampedePhase::Running:
        if (HerdPanic < 0.3f)
        {
            CurrentPhase = ECrowd_StampedePhase::Dispersing;
            UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] Phase -> Dispersing"));
        }
        break;

    case ECrowd_StampedePhase::Dispersing:
        if (HerdPanic < 0.05f)
        {
            CurrentPhase = ECrowd_StampedePhase::Settled;
            UE_LOG(LogTemp, Log, TEXT("[CrowdStampede] Phase -> Settled"));
        }
        break;

    default:
        break;
    }
}

void UCrowdStampedeController::TickAgentMovement(float DeltaTime)
{
    if (CurrentPhase != ECrowd_StampedePhase::Running &&
        CurrentPhase != ECrowd_StampedePhase::Dispersing)
    {
        return;
    }

    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        if (!Agent.bIsPanicking) continue;

        FVector FleeDir = ComputeFleeDirection(Agent);
        float   Speed   = 600.f * StampedeSpeedMultiplier * Agent.PanicLevel;

        Agent.Velocity  = FMath::VInterpTo(Agent.Velocity, FleeDir * Speed, DeltaTime, 3.f);
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

FVector UCrowdStampedeController::ComputeFleeDirection(const FCrowd_StampedeAgent& Agent) const
{
    FVector AwayFromEpicentre = (Agent.Location - ActiveEvent.EpicentreLocation).GetSafeNormal();

    // Add slight lateral spread so herd fans out rather than piling up
    FVector Lateral = FVector::CrossProduct(AwayFromEpicentre, FVector::UpVector);
    float   Spread  = FMath::Sin(static_cast<float>(Agent.AgentIndex) * 0.7f) * 0.4f;

    return (AwayFromEpicentre + Lateral * Spread).GetSafeNormal();
}
