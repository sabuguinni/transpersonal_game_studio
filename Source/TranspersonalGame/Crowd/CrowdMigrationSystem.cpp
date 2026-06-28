#include "CrowdMigrationSystem.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ACrowdMigrationSystem::ACrowdMigrationSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    HerdSize = 20;
    MigrationRadius = 2000.f;
    FleeRadius = 800.f;
    MigrationDestination = FVector(5000.f, 5000.f, 0.f);
    ElapsedTime = 0.f;
    bMigrationActive = false;
}

void ACrowdMigrationSystem::BeginPlay()
{
    Super::BeginPlay();
    FVector Origin = GetActorLocation();
    InitializeHerd(HerdSize, Origin, MigrationDestination);
}

void ACrowdMigrationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ElapsedTime += DeltaTime;

    if (bMigrationActive)
    {
        UpdateAgentPositions(DeltaTime);
    }
}

void ACrowdMigrationSystem::InitializeHerd(int32 InHerdSize, FVector Origin, FVector Destination)
{
    Agents.Empty();
    HerdSize = InHerdSize;
    MigrationDestination = Destination;

    for (int32 i = 0; i < HerdSize; i++)
    {
        FCrowd_MigrationAgent Agent;
        float OffsetX = FMath::RandRange(-MigrationRadius * 0.3f, MigrationRadius * 0.3f);
        float OffsetY = FMath::RandRange(-MigrationRadius * 0.3f, MigrationRadius * 0.3f);
        Agent.CurrentLocation = Origin + FVector(OffsetX, OffsetY, 0.f);
        Agent.TargetLocation = Destination + FVector(
            FMath::RandRange(-200.f, 200.f),
            FMath::RandRange(-200.f, 200.f),
            0.f
        );
        Agent.MoveSpeed = FMath::RandRange(250.f, 400.f);
        Agent.State = ECrowd_MigrationState::Migrating;
        Agent.HerdID = 0;
        Agent.bIsLeader = (i == 0);
        Agents.Add(Agent);
    }

    bMigrationActive = true;
    UE_LOG(LogTemp, Log, TEXT("CrowdMigrationSystem: Herd of %d initialized"), HerdSize);
}

void ACrowdMigrationSystem::TriggerFlee(FVector ThreatLocation)
{
    for (FCrowd_MigrationAgent& Agent : Agents)
    {
        FVector FleeDir = (Agent.CurrentLocation - ThreatLocation).GetSafeNormal();
        Agent.TargetLocation = Agent.CurrentLocation + FleeDir * FleeRadius;
        Agent.State = ECrowd_MigrationState::Fleeing;
        Agent.MoveSpeed = FMath::RandRange(500.f, 700.f);
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdMigrationSystem: Flee triggered from threat at %s"), *ThreatLocation.ToString());
}

void ACrowdMigrationSystem::UpdateAgentPositions(float DeltaTime)
{
    for (FCrowd_MigrationAgent& Agent : Agents)
    {
        if (Agent.State == ECrowd_MigrationState::Idle)
            continue;

        FVector Direction = (Agent.TargetLocation - Agent.CurrentLocation).GetSafeNormal();
        float Distance = FVector::Dist(Agent.CurrentLocation, Agent.TargetLocation);

        if (Distance > 50.f)
        {
            Agent.CurrentLocation += Direction * Agent.MoveSpeed * DeltaTime;
        }
        else
        {
            // Reached target — transition to idle or dispersing
            if (Agent.State == ECrowd_MigrationState::Fleeing)
            {
                Agent.State = ECrowd_MigrationState::Dispersing;
            }
            else
            {
                Agent.State = ECrowd_MigrationState::Idle;
            }
        }
    }
}

int32 ACrowdMigrationSystem::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_MigrationAgent& Agent : Agents)
    {
        if (Agent.State != ECrowd_MigrationState::Idle)
        {
            Count++;
        }
    }
    return Count;
}
