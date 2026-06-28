// CrowdStampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Full implementation of stampede trigger, panic propagation, player danger

#include "CrowdStampedeController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowdStampedeController::ACrowdStampedeController()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentState = ECrowd_StampedeState::Idle;
    LastTrigger = ECrowd_StampedeTrigger::None;
    StampedeTimer = 0.0f;
    StateTimer = 0.0f;
    StampedeDirection = FVector(1.0f, 0.0f, 0.0f);
    ThreatOrigin = FVector::ZeroVector;
}

void ACrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();

    // Cache player actor reference
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerChar)
    {
        CachedPlayerActor = PlayerChar;
    }
}

void ACrowdStampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateStampedeState(DeltaTime);

    if (CurrentState == ECrowd_StampedeState::Stampeding ||
        CurrentState == ECrowd_StampedeState::Panic)
    {
        UpdateAgentMovement(DeltaTime);

        if (StampedeConfig.bPlayerDangerIfInPath)
        {
            CheckPlayerCollision();
        }
    }
}

void ACrowdStampedeController::TriggerStampede(ECrowd_StampedeTrigger Trigger, FVector ThreatLocation)
{
    if (RegisteredAgents.Num() < StampedeConfig.MinAgentsForStampede)
    {
        // Not enough agents — only trigger alert
        TransitionToState(ECrowd_StampedeState::Alert);
        return;
    }

    LastTrigger = Trigger;
    ThreatOrigin = ThreatLocation;

    // Calculate average herd centre
    FVector HerdCentre = FVector::ZeroVector;
    for (const FCrowd_StampedeAgent& Agent : RegisteredAgents)
    {
        if (Agent.AgentActor)
        {
            HerdCentre += Agent.AgentActor->GetActorLocation();
        }
    }
    if (RegisteredAgents.Num() > 0)
    {
        HerdCentre /= RegisteredAgents.Num();
    }

    // Flee direction = away from threat
    StampedeDirection = (HerdCentre - ThreatLocation).GetSafeNormal();

    // Set all agents to panic
    for (FCrowd_StampedeAgent& Agent : RegisteredAgents)
    {
        if (Agent.AgentActor)
        {
            Agent.PanicLevel = 1.0f;
            Agent.FleeDirection = CalculateFleeDirection(ThreatLocation, Agent.AgentActor->GetActorLocation());
            Agent.Speed = StampedeConfig.MaxStampedeSpeed;
        }
    }

    TransitionToState(ECrowd_StampedeState::Stampeding);
    StampedeTimer = StampedeConfig.StampedeDuration;

    UE_LOG(LogTemp, Warning, TEXT("CrowdStampedeController: Stampede triggered! Trigger=%d, Agents=%d, Direction=%s"),
        (int32)Trigger, RegisteredAgents.Num(), *StampedeDirection.ToString());
}

void ACrowdStampedeController::RegisterAgent(AActor* Agent, bool bIsLeader)
{
    if (!Agent) return;

    // Check not already registered
    for (const FCrowd_StampedeAgent& Existing : RegisteredAgents)
    {
        if (Existing.AgentActor == Agent) return;
    }

    FCrowd_StampedeAgent NewAgent;
    NewAgent.AgentActor = Agent;
    NewAgent.bIsLeader = bIsLeader;
    NewAgent.PanicLevel = 0.0f;
    NewAgent.Speed = 400.0f;
    NewAgent.FleeDirection = FVector::ZeroVector;

    RegisteredAgents.Add(NewAgent);
}

void ACrowdStampedeController::UnregisterAgent(AActor* Agent)
{
    RegisteredAgents.RemoveAll([Agent](const FCrowd_StampedeAgent& A)
    {
        return A.AgentActor == Agent;
    });
}

void ACrowdStampedeController::PropagateAlertToNearbyAgents(FVector EpicentreLocation, float Radius)
{
    for (FCrowd_StampedeAgent& Agent : RegisteredAgents)
    {
        if (!Agent.AgentActor) continue;

        float Dist = FVector::Dist(Agent.AgentActor->GetActorLocation(), EpicentreLocation);
        if (Dist <= Radius)
        {
            float PropagatedPanic = FMath::Clamp(1.0f - (Dist / Radius), 0.0f, 1.0f);
            Agent.PanicLevel = FMath::Max(Agent.PanicLevel, PropagatedPanic * StampedeConfig.PanicPropagationSpeed);
        }
    }
}

bool ACrowdStampedeController::IsPlayerInStampedePath(AActor* PlayerActor) const
{
    if (!PlayerActor || CurrentState != ECrowd_StampedeState::Stampeding) return false;

    FVector PlayerLoc = PlayerActor->GetActorLocation();

    for (const FCrowd_StampedeAgent& Agent : RegisteredAgents)
    {
        if (!Agent.AgentActor) continue;

        FVector AgentLoc = Agent.AgentActor->GetActorLocation();
        float DistToPlayer = FVector::Dist(AgentLoc, PlayerLoc);

        if (DistToPlayer < 300.0f)
        {
            // Check if agent is moving toward player
            FVector ToPlayer = (PlayerLoc - AgentLoc).GetSafeNormal();
            float Dot = FVector::DotProduct(Agent.FleeDirection, ToPlayer);
            if (Dot > 0.5f)
            {
                return true;
            }
        }
    }

    return false;
}

void ACrowdStampedeController::ForceSettle()
{
    for (FCrowd_StampedeAgent& Agent : RegisteredAgents)
    {
        Agent.PanicLevel = 0.0f;
        Agent.FleeDirection = FVector::ZeroVector;
        Agent.Speed = 400.0f;
    }
    TransitionToState(ECrowd_StampedeState::Settled);
    StampedeTimer = 0.0f;
}

int32 ACrowdStampedeController::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_StampedeAgent& Agent : RegisteredAgents)
    {
        if (Agent.AgentActor) Count++;
    }
    return Count;
}

float ACrowdStampedeController::GetAveragePanicLevel() const
{
    if (RegisteredAgents.Num() == 0) return 0.0f;

    float Total = 0.0f;
    for (const FCrowd_StampedeAgent& Agent : RegisteredAgents)
    {
        Total += Agent.PanicLevel;
    }
    return Total / RegisteredAgents.Num();
}

// ---- Private ----

void ACrowdStampedeController::UpdateStampedeState(float DeltaTime)
{
    StateTimer += DeltaTime;

    switch (CurrentState)
    {
    case ECrowd_StampedeState::Stampeding:
        StampedeTimer -= DeltaTime;
        if (StampedeTimer <= 0.0f)
        {
            TransitionToState(ECrowd_StampedeState::Dispersing);
        }
        break;

    case ECrowd_StampedeState::Dispersing:
        // Gradually reduce panic
        for (FCrowd_StampedeAgent& Agent : RegisteredAgents)
        {
            Agent.PanicLevel = FMath::Max(0.0f, Agent.PanicLevel - DeltaTime * 0.3f);
            Agent.Speed = FMath::Lerp(Agent.Speed, 400.0f, DeltaTime * 0.5f);
        }
        if (GetAveragePanicLevel() < 0.05f)
        {
            TransitionToState(ECrowd_StampedeState::Settled);
        }
        break;

    case ECrowd_StampedeState::Alert:
        if (StateTimer > 5.0f)
        {
            TransitionToState(ECrowd_StampedeState::Idle);
        }
        break;

    default:
        break;
    }
}

void ACrowdStampedeController::UpdateAgentMovement(float DeltaTime)
{
    for (FCrowd_StampedeAgent& Agent : RegisteredAgents)
    {
        if (!Agent.AgentActor || Agent.FleeDirection.IsNearlyZero()) continue;

        // Add slight random deviation for natural look
        FVector Deviation = FVector(
            FMath::RandRange(-0.15f, 0.15f),
            FMath::RandRange(-0.15f, 0.15f),
            0.0f
        );
        FVector MoveDir = (Agent.FleeDirection + Deviation).GetSafeNormal();

        FVector NewLocation = Agent.AgentActor->GetActorLocation() + MoveDir * Agent.Speed * DeltaTime;
        Agent.AgentActor->SetActorLocation(NewLocation, true);

        // Face movement direction
        FRotator NewRot = MoveDir.Rotation();
        Agent.AgentActor->SetActorRotation(NewRot);
    }
}

void ACrowdStampedeController::CheckPlayerCollision()
{
    if (!CachedPlayerActor.IsValid()) return;

    if (IsPlayerInStampedePath(CachedPlayerActor.Get()))
    {
        // Apply damage to player
        ACharacter* PlayerChar = Cast<ACharacter>(CachedPlayerActor.Get());
        if (PlayerChar)
        {
            UGameplayStatics::ApplyDamage(
                PlayerChar,
                StampedeConfig.PlayerDamagePerSecond * GetWorld()->GetDeltaSeconds(),
                nullptr,
                this,
                nullptr
            );
        }
    }
}

FVector ACrowdStampedeController::CalculateFleeDirection(FVector ThreatLocation, FVector AgentLocation) const
{
    FVector BaseDir = (AgentLocation - ThreatLocation).GetSafeNormal();
    // Flatten to XY plane
    BaseDir.Z = 0.0f;
    return BaseDir.GetSafeNormal();
}

void ACrowdStampedeController::TransitionToState(ECrowd_StampedeState NewState)
{
    if (CurrentState == NewState) return;

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: State %d -> %d"), (int32)CurrentState, (int32)NewState);
    CurrentState = NewState;
    StateTimer = 0.0f;
}
