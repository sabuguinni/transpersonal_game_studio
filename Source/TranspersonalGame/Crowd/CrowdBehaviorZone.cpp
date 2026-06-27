#include "CrowdBehaviorZone.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

ACrowd_BehaviorZone::ACrowd_BehaviorZone()
    : BehaviorUpdateTimer(0.0f)
{
    PrimaryActorTick.bCanEverTick = true;

    ZoneBoundary = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneBoundary"));
    ZoneBoundary->SetSphereRadius(500.0f);
    ZoneBoundary->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ZoneBoundary->SetVisibility(false);
    RootComponent = ZoneBoundary;
}

void ACrowd_BehaviorZone::BeginPlay()
{
    Super::BeginPlay();

    // Sync sphere radius with zone config
    if (ZoneBoundary)
    {
        ZoneBoundary->SetSphereRadius(ZoneConfig.ZoneRadius);
    }
}

void ACrowd_BehaviorZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ZoneConfig.bIsActive)
    {
        return;
    }

    BehaviorUpdateTimer += DeltaTime;
    if (BehaviorUpdateTimer >= BehaviorUpdateInterval)
    {
        BehaviorUpdateTimer = 0.0f;
        UpdateAgentBehaviors();
    }

#if WITH_EDITOR
    // Debug visualization in editor
    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        ZoneConfig.ZoneRadius,
        32,
        FColor::Green,
        false,
        BehaviorUpdateInterval
    );
#endif
}

void ACrowd_BehaviorZone::TriggerDangerResponse(float DangerIntensity)
{
    ZoneConfig.DangerLevel = FMath::Clamp(DangerIntensity, 0.0f, 1.0f);
    ApplyFleeBehavior(DangerIntensity);

    UE_LOG(LogTemp, Warning, TEXT("CrowdBehaviorZone [%s]: Danger response triggered! Intensity=%.2f, Agents fleeing=%d"),
        *GetName(), DangerIntensity, ActiveAgents.Num());
}

void ACrowd_BehaviorZone::RegisterAgent(const FCrowd_AgentData& AgentData)
{
    if (IsAtCapacity())
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdBehaviorZone [%s]: At capacity (%d/%d), cannot register agent %d"),
            *GetName(), ActiveAgents.Num(), ZoneConfig.MaxAgentCapacity, AgentData.AgentID);
        return;
    }

    // Check for duplicate
    for (const FCrowd_AgentData& Existing : ActiveAgents)
    {
        if (Existing.AgentID == AgentData.AgentID)
        {
            return;
        }
    }

    ActiveAgents.Add(AgentData);
}

void ACrowd_BehaviorZone::UnregisterAgent(int32 AgentID)
{
    ActiveAgents.RemoveAll([AgentID](const FCrowd_AgentData& Agent)
    {
        return Agent.AgentID == AgentID;
    });
}

float ACrowd_BehaviorZone::GetOccupancyRatio() const
{
    if (ZoneConfig.MaxAgentCapacity <= 0)
    {
        return 0.0f;
    }
    return static_cast<float>(ActiveAgents.Num()) / static_cast<float>(ZoneConfig.MaxAgentCapacity);
}

bool ACrowd_BehaviorZone::IsAtCapacity() const
{
    return ActiveAgents.Num() >= ZoneConfig.MaxAgentCapacity;
}

FVector ACrowd_BehaviorZone::GetZoneCenter() const
{
    return GetActorLocation();
}

void ACrowd_BehaviorZone::UpdateAgentBehaviors()
{
    if (ActiveAgents.Num() == 0)
    {
        return;
    }

    // Decay danger level over time
    ZoneConfig.DangerLevel = FMath::Max(0.0f, ZoneConfig.DangerLevel - 0.1f);

    // If danger is still high, keep fleeing
    if (ZoneConfig.DangerLevel > 0.3f)
    {
        ApplyFleeBehavior(ZoneConfig.DangerLevel);
        return;
    }

    // Apply zone-specific behaviors
    switch (ZoneConfig.ZoneType)
    {
    case ECrowd_ZoneType::WateringHole:
        ApplyWateringHoleBehavior();
        break;

    case ECrowd_ZoneType::NestingGround:
        ApplyNestingBehavior();
        break;

    case ECrowd_ZoneType::MigrationPath:
        // Agents in migration path continue migrating
        for (FCrowd_AgentData& Agent : ActiveAgents)
        {
            Agent.State = ECrowd_AgentState::Migrating;
        }
        break;

    case ECrowd_ZoneType::FeedingArea:
        // Agents in feeding area graze
        for (FCrowd_AgentData& Agent : ActiveAgents)
        {
            Agent.HungerLevel = FMath::Max(0.0f, Agent.HungerLevel - 0.05f);
            if (Agent.HungerLevel < 0.8f)
            {
                Agent.State = ECrowd_AgentState::Grazing;
            }
        }
        break;

    default:
        break;
    }
}

void ACrowd_BehaviorZone::ApplyWateringHoleBehavior()
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        // Agents drink to reduce thirst
        Agent.ThirstLevel = FMath::Max(0.0f, Agent.ThirstLevel - 0.08f);

        if (Agent.ThirstLevel < 0.2f)
        {
            // Satisfied — become alert/idle
            Agent.State = ECrowd_AgentState::Alert;
        }
        else
        {
            Agent.State = ECrowd_AgentState::Drinking;
        }

        // Fear increases slightly near water (predator ambush risk)
        Agent.FearLevel = FMath::Min(1.0f, Agent.FearLevel + 0.02f);
    }
}

void ACrowd_BehaviorZone::ApplyNestingBehavior()
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        Agent.State = ECrowd_AgentState::Nesting;

        // Nesting increases fear sensitivity (protecting young)
        Agent.FearLevel = FMath::Min(1.0f, Agent.FearLevel + 0.05f);
    }
}

void ACrowd_BehaviorZone::ApplyFleeBehavior(float Intensity)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        Agent.State = ECrowd_AgentState::Fleeing;
        Agent.FearLevel = FMath::Min(1.0f, Agent.FearLevel + Intensity * 0.5f);
    }
}
