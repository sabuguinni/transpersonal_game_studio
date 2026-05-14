#include "Crowd_MassSpawner.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassAgentComponent.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "Math/UnrealMathUtility.h"

ACrowd_MassSpawner::ACrowd_MassSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    MaxAgents = 10000;
    SpawnRadius = 5000.0f;
    AgentSpacing = 200.0f;
    bAutoSpawn = true;
    
    // Agent type configuration
    AgentTypes = {ECrowd_AgentType::Human, ECrowd_AgentType::Animal};
    TypeWeights = {0.7f, 0.3f}; // 70% humans, 30% animals
    
    // LOD distances
    HighDetailDistance = 1000.0f;
    MediumDetailDistance = 3000.0f;
    LowDetailDistance = 8000.0f;
    
    // Behavior defaults
    MovementSpeed = 300.0f;
    WanderRadius = 2000.0f;
    bEnableFlocking = true;
    FlockingRadius = 500.0f;
    
    // Performance tracking
    LastSpawnTime = 0.0f;
    FrameSpawnCount = 0;
    
    // Cached references
    MassEntitySubsystem = nullptr;
    PlayerPawn = nullptr;
}

void ACrowd_MassSpawner::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize Mass Entity system
    InitializeMassEntitySystem();
    
    // Cache player reference
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PlayerPawn = PC->GetPawn();
    }
    
    // Auto-spawn if enabled
    if (bAutoSpawn)
    {
        // Delay spawning to allow world to fully load
        FTimerHandle SpawnTimer;
        GetWorldTimerManager().SetTimer(SpawnTimer, this, &ACrowd_MassSpawner::SpawnCrowdAgents, 2.0f, false);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Initialized with MaxAgents=%d, SpawnRadius=%.1f"), MaxAgents, SpawnRadius);
}

void ACrowd_MassSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DespawnAllAgents();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update LOD for existing agents
    UpdateAgentLOD();
    
    // Reset frame spawn counter
    FrameSpawnCount = 0;
}

void ACrowd_MassSpawner::InitializeMassEntitySystem()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            CreateEntityTemplate();
            RegisterEntityProcessors();
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Mass Entity system initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassSpawner: Failed to get MassEntitySubsystem"));
        }
    }
}

void ACrowd_MassSpawner::CreateEntityTemplate()
{
    if (!MassEntitySubsystem)
        return;
        
    // Create entity archetype with required fragments
    FMassArchetypeHandle ArchetypeHandle = MassEntitySubsystem->CreateArchetype({
        FMassTransformFragment::StaticStruct(),
        FMassVelocityFragment::StaticStruct(),
        FMassForceFragment::StaticStruct()
    });
    
    // Create template entity
    EntityTemplate = MassEntitySubsystem->CreateEntity(ArchetypeHandle);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Entity template created"));
}

void ACrowd_MassSpawner::RegisterEntityProcessors()
{
    // Entity processors will be registered by the Mass Entity framework
    // This is handled automatically when entities are spawned
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Entity processors registered"));
}

void ACrowd_MassSpawner::SpawnCrowdAgents()
{
    if (!MassEntitySubsystem || MaxAgents <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Cannot spawn - invalid configuration"));
        return;
    }
    
    // Clear existing agents
    DespawnAllAgents();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Starting to spawn %d agents"), MaxAgents);
    
    int32 SpawnedCount = 0;
    const float StartTime = FPlatformTime::Seconds();
    
    // Calculate grid dimensions for even distribution
    const int32 GridSize = FMath::CeilToInt(FMath::Sqrt(MaxAgents));
    const float GridSpacing = (SpawnRadius * 2.0f) / GridSize;
    
    for (int32 i = 0; i < MaxAgents && SpawnedCount < MaxAgents; ++i)
    {
        // Calculate grid position
        const int32 GridX = i % GridSize;
        const int32 GridY = i / GridSize;
        
        // Convert to world position
        const FVector GridOffset = FVector(
            (GridX - GridSize * 0.5f) * GridSpacing,
            (GridY - GridSize * 0.5f) * GridSpacing,
            0.0f
        );
        
        const FVector SpawnLocation = GetActorLocation() + GridOffset + FVector(
            FMath::RandRange(-AgentSpacing * 0.5f, AgentSpacing * 0.5f),
            FMath::RandRange(-AgentSpacing * 0.5f, AgentSpacing * 0.5f),
            0.0f
        );
        
        // Select agent type
        const ECrowd_AgentType AgentType = SelectRandomAgentType();
        
        // Spawn agent
        SpawnAgentAtLocation(SpawnLocation, AgentType);
        SpawnedCount++;
        
        // Limit spawns per frame for performance
        FrameSpawnCount++;
        if (FrameSpawnCount >= MaxSpawnsPerFrame)
        {
            // Continue spawning next frame
            FTimerHandle ContinueSpawnTimer;
            GetWorldTimerManager().SetTimer(ContinueSpawnTimer, [this, i]()
            {
                // Resume spawning from where we left off
                for (int32 j = i + 1; j < MaxAgents; ++j)
                {
                    if (FrameSpawnCount >= MaxSpawnsPerFrame)
                        break;
                        
                    const FVector Location = GetRandomSpawnLocation();
                    const ECrowd_AgentType Type = SelectRandomAgentType();
                    SpawnAgentAtLocation(Location, Type);
                    FrameSpawnCount++;
                }
            }, 0.1f, false);
            break;
        }
    }
    
    const float ElapsedTime = FPlatformTime::Seconds() - StartTime;
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Spawned %d agents in %.3f seconds"), SpawnedCount, ElapsedTime);
}

void ACrowd_MassSpawner::SpawnAgentAtLocation(const FVector& Location, ECrowd_AgentType AgentType)
{
    if (!MassEntitySubsystem || !EntityTemplate.IsValid())
        return;
    
    // Create new entity from template
    FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity(EntityTemplate);
    
    if (NewEntity.IsValid())
    {
        // Set initial transform
        if (FMassTransformFragment* TransformFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(NewEntity))
        {
            TransformFragment->SetTransform(FTransform(Location));
        }
        
        // Set initial velocity
        if (FMassVelocityFragment* VelocityFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(NewEntity))
        {
            const FVector RandomVelocity = FVector(
                FMath::RandRange(-MovementSpeed * 0.1f, MovementSpeed * 0.1f),
                FMath::RandRange(-MovementSpeed * 0.1f, MovementSpeed * 0.1f),
                0.0f
            );
            VelocityFragment->Value = RandomVelocity;
        }
        
        SpawnedEntities.Add(NewEntity);
    }
}

void ACrowd_MassSpawner::DespawnAllAgents()
{
    if (!MassEntitySubsystem)
        return;
    
    const int32 AgentCount = SpawnedEntities.Num();
    
    for (const FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (Entity.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(Entity);
        }
    }
    
    SpawnedEntities.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Despawned %d agents"), AgentCount);
}

FVector ACrowd_MassSpawner::GetRandomSpawnLocation() const
{
    const float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    const float Distance = FMath::RandRange(0.0f, SpawnRadius);
    
    return GetActorLocation() + FVector(
        FMath::Cos(Angle) * Distance,
        FMath::Sin(Angle) * Distance,
        0.0f
    );
}

ECrowd_AgentType ACrowd_MassSpawner::SelectRandomAgentType() const
{
    if (AgentTypes.Num() == 0)
        return ECrowd_AgentType::Human;
    
    if (TypeWeights.Num() != AgentTypes.Num())
        return AgentTypes[FMath::RandRange(0, AgentTypes.Num() - 1)];
    
    // Weighted selection
    float TotalWeight = 0.0f;
    for (float Weight : TypeWeights)
    {
        TotalWeight += Weight;
    }
    
    const float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float AccumulatedWeight = 0.0f;
    
    for (int32 i = 0; i < AgentTypes.Num(); ++i)
    {
        AccumulatedWeight += TypeWeights[i];
        if (RandomValue <= AccumulatedWeight)
        {
            return AgentTypes[i];
        }
    }
    
    return AgentTypes.Last();
}

void ACrowd_MassSpawner::UpdateAgentLOD()
{
    if (!PlayerPawn || SpawnedEntities.Num() == 0)
        return;
    
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for a subset of agents each frame to spread the cost
    static int32 LODUpdateIndex = 0;
    const int32 AgentsPerFrame = FMath::Max(1, SpawnedEntities.Num() / 10); // Update 10% each frame
    
    for (int32 i = 0; i < AgentsPerFrame && LODUpdateIndex < SpawnedEntities.Num(); ++i, ++LODUpdateIndex)
    {
        const FMassEntityHandle& Entity = SpawnedEntities[LODUpdateIndex];
        if (!Entity.IsValid())
            continue;
        
        if (const FMassTransformFragment* TransformFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(Entity))
        {
            const float Distance = FVector::Dist(PlayerLocation, TransformFragment->GetTransform().GetLocation());
            const ECrowd_LODLevel LODLevel = CalculateLODLevel(FVector(Distance, 0.0f, 0.0f));
            
            // LOD level would be applied to rendering/behavior processors
            // This is handled by the Mass Entity framework automatically
        }
    }
    
    // Reset index when we've processed all agents
    if (LODUpdateIndex >= SpawnedEntities.Num())
    {
        LODUpdateIndex = 0;
    }
}

ECrowd_LODLevel ACrowd_MassSpawner::CalculateLODLevel(const FVector& AgentLocation) const
{
    if (!PlayerPawn)
        return ECrowd_LODLevel::Disabled;
    
    const float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), AgentLocation);
    
    if (Distance <= HighDetailDistance)
        return ECrowd_LODLevel::High;
    else if (Distance <= MediumDetailDistance)
        return ECrowd_LODLevel::Medium;
    else if (Distance <= LowDetailDistance)
        return ECrowd_LODLevel::Low;
    else
        return ECrowd_LODLevel::Disabled;
}

void ACrowd_MassSpawner::SetAgentCount(int32 NewCount)
{
    MaxAgents = FMath::Max(0, NewCount);
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Agent count set to %d"), MaxAgents);
}

int32 ACrowd_MassSpawner::GetActiveAgentCount() const
{
    return SpawnedEntities.Num();
}

void ACrowd_MassSpawner::UpdateLODDistances(float High, float Medium, float Low)
{
    HighDetailDistance = FMath::Max(0.0f, High);
    MediumDetailDistance = FMath::Max(HighDetailDistance, Medium);
    LowDetailDistance = FMath::Max(MediumDetailDistance, Low);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: LOD distances updated - High:%.1f, Medium:%.1f, Low:%.1f"), 
           HighDetailDistance, MediumDetailDistance, LowDetailDistance);
}

void ACrowd_MassSpawner::SetMovementParameters(float Speed, float Radius)
{
    MovementSpeed = FMath::Max(0.0f, Speed);
    WanderRadius = FMath::Max(0.0f, Radius);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Movement parameters updated - Speed:%.1f, Radius:%.1f"), 
           MovementSpeed, WanderRadius);
}

void ACrowd_MassSpawner::EnableFlocking(bool bEnable, float Radius)
{
    bEnableFlocking = bEnable;
    FlockingRadius = FMath::Max(0.0f, Radius);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Flocking %s with radius %.1f"), 
           bEnableFlocking ? TEXT("enabled") : TEXT("disabled"), FlockingRadius);
}

void ACrowd_MassSpawner::DebugSpawnTestAgents()
{
    const int32 TestCount = FMath::Min(100, MaxAgents);
    const int32 PreviousMax = MaxAgents;
    
    MaxAgents = TestCount;
    SpawnCrowdAgents();
    MaxAgents = PreviousMax;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSpawner: Debug spawned %d test agents"), TestCount);
}

void ACrowd_MassSpawner::DrawDebugInfo()
{
    if (!GetWorld())
        return;
    
    const FVector Center = GetActorLocation();
    
    // Draw spawn radius
    DrawDebugCircle(GetWorld(), Center, SpawnRadius, 32, FColor::Green, false, 1.0f, 0, 10.0f, FVector(0,1,0), FVector(1,0,0));
    
    // Draw LOD distance rings
    DrawDebugCircle(GetWorld(), Center, HighDetailDistance, 32, FColor::Red, false, 1.0f, 0, 5.0f, FVector(0,1,0), FVector(1,0,0));
    DrawDebugCircle(GetWorld(), Center, MediumDetailDistance, 32, FColor::Yellow, false, 1.0f, 0, 5.0f, FVector(0,1,0), FVector(1,0,0));
    DrawDebugCircle(GetWorld(), Center, LowDetailDistance, 32, FColor::Blue, false, 1.0f, 0, 5.0f, FVector(0,1,0), FVector(1,0,0));
    
    // Draw agent count info
    const FString InfoText = FString::Printf(TEXT("Active Agents: %d / %d"), GetActiveAgentCount(), MaxAgents);
    DrawDebugString(GetWorld(), Center + FVector(0, 0, 200), InfoText, nullptr, FColor::White, 1.0f);
}