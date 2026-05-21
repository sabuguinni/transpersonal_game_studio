#include "Crowd_MassSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    MaxTotalAgents = 50000;
    SimulationRadius = 100000.0f;
    LODDistance1 = 5000.0f;
    LODDistance2 = 15000.0f;
    LODDistance3 = 30000.0f;
    bEnableLODCulling = true;
    UpdateFrequency = 0.1f;
    AgentsPerFrame = 100;
    
    LastUpdateTime = 0.0f;
    CurrentUpdateIndex = 0;
    
    // Initialize biome locations
    InitializeBiomes();
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: BeginPlay - Initializing crowd simulation"));
    
    // Spawn initial agents in each biome
    SpawnAgentsInBiome(ECrowd_BiomeType::Savana, 500, ECrowd_AgentType::Herbivore);
    SpawnAgentsInBiome(ECrowd_BiomeType::Floresta, 300, ECrowd_AgentType::Herbivore);
    SpawnAgentsInBiome(ECrowd_BiomeType::Deserto, 200, ECrowd_AgentType::Scavenger);
    SpawnAgentsInBiome(ECrowd_BiomeType::Pantano, 150, ECrowd_AgentType::Neutral);
    SpawnAgentsInBiome(ECrowd_BiomeType::Montanha, 100, ECrowd_AgentType::Carnivore);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Initial agent spawning completed - %d total agents"), ActiveAgents.Num());
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateAgentBehavior(DeltaTime);
        
        // Apply LOD culling if enabled
        if (bEnableLODCulling)
        {
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn)
            {
                ApplyLODCulling(PlayerPawn->GetActorLocation());
            }
        }
        
        // Optimize simulation periodically
        if (FMath::Fmod(LastUpdateTime, 5.0f) < UpdateFrequency)
        {
            OptimizeSimulation();
        }
        
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassSimulationManager::InitializeBiomes()
{
    BiomeLocations.Empty();
    
    // Savana biome
    FCrowd_BiomeLocation SavanaBiome;
    SavanaBiome.BiomeType = ECrowd_BiomeType::Savana;
    SavanaBiome.Location = FVector(0.0f, 0.0f, 100.0f);
    SavanaBiome.Radius = 15000.0f;
    SavanaBiome.MaxAgents = 1000;
    BiomeLocations.Add(SavanaBiome);
    
    // Floresta biome
    FCrowd_BiomeLocation FlorestaBiome;
    FlorestaBiome.BiomeType = ECrowd_BiomeType::Floresta;
    FlorestaBiome.Location = FVector(-45000.0f, 40000.0f, 100.0f);
    FlorestaBiome.Radius = 12000.0f;
    FlorestaBiome.MaxAgents = 800;
    BiomeLocations.Add(FlorestaBiome);
    
    // Deserto biome
    FCrowd_BiomeLocation DesertoBiome;
    DesertoBiome.BiomeType = ECrowd_BiomeType::Deserto;
    DesertoBiome.Location = FVector(55000.0f, 0.0f, 100.0f);
    DesertoBiome.Radius = 10000.0f;
    DesertoBiome.MaxAgents = 600;
    BiomeLocations.Add(DesertoBiome);
    
    // Pantano biome
    FCrowd_BiomeLocation PantanoBiome;
    PantanoBiome.BiomeType = ECrowd_BiomeType::Pantano;
    PantanoBiome.Location = FVector(-50000.0f, -45000.0f, 100.0f);
    PantanoBiome.Radius = 8000.0f;
    PantanoBiome.MaxAgents = 500;
    BiomeLocations.Add(PantanoBiome);
    
    // Montanha biome
    FCrowd_BiomeLocation MontanhaBiome;
    MontanhaBiome.BiomeType = ECrowd_BiomeType::Montanha;
    MontanhaBiome.Location = FVector(40000.0f, 50000.0f, 200.0f);
    MontanhaBiome.Radius = 6000.0f;
    MontanhaBiome.MaxAgents = 400;
    BiomeLocations.Add(MontanhaBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Initialized %d biomes"), BiomeLocations.Num());
}

void ACrowd_MassSimulationManager::SpawnAgentsInBiome(ECrowd_BiomeType BiomeType, int32 Count, ECrowd_AgentType AgentType)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    if (BiomeCenter == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Invalid biome type for spawning"));
        return;
    }
    
    float BiomeRadius = 5000.0f;
    for (const FCrowd_BiomeLocation& Biome : BiomeLocations)
    {
        if (Biome.BiomeType == BiomeType)
        {
            BiomeRadius = Biome.Radius;
            break;
        }
    }
    
    for (int32 i = 0; i < Count; ++i)
    {
        FCrowd_AgentData NewAgent;
        NewAgent.AgentType = AgentType;
        
        // Random position within biome radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, BiomeRadius * 0.8f);
        
        NewAgent.Position = BiomeCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        // Set agent properties based on type
        switch (AgentType)
        {
            case ECrowd_AgentType::Herbivore:
                NewAgent.Speed = FMath::RandRange(80.0f, 120.0f);
                NewAgent.Health = FMath::RandRange(80.0f, 100.0f);
                break;
            case ECrowd_AgentType::Carnivore:
                NewAgent.Speed = FMath::RandRange(150.0f, 200.0f);
                NewAgent.Health = FMath::RandRange(100.0f, 150.0f);
                break;
            case ECrowd_AgentType::Scavenger:
                NewAgent.Speed = FMath::RandRange(100.0f, 140.0f);
                NewAgent.Health = FMath::RandRange(60.0f, 90.0f);
                break;
            case ECrowd_AgentType::Neutral:
                NewAgent.Speed = FMath::RandRange(70.0f, 110.0f);
                NewAgent.Health = FMath::RandRange(70.0f, 100.0f);
                break;
        }
        
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.GroupID = i / 10; // Groups of 10
        
        ActiveAgents.Add(NewAgent);
        SpawnVisualAgent(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Spawned %d agents in biome %d"), Count, (int32)BiomeType);
}

void ACrowd_MassSimulationManager::UpdateAgentBehavior(float DeltaTime)
{
    int32 AgentsToUpdate = FMath::Min(AgentsPerFrame, ActiveAgents.Num() - CurrentUpdateIndex);
    
    for (int32 i = 0; i < AgentsToUpdate; ++i)
    {
        int32 AgentIndex = CurrentUpdateIndex + i;
        if (AgentIndex >= ActiveAgents.Num()) break;
        
        FCrowd_AgentData& Agent = ActiveAgents[AgentIndex];
        
        // Get nearby agents for flocking behavior
        TArray<FCrowd_AgentData> NearbyAgents = GetAgentsInRadius(Agent.Position, 500.0f);
        
        // Apply flocking behaviors
        UpdateAgentFlocking(Agent, NearbyAgents);
        
        // Update position
        Agent.Position += Agent.Velocity * DeltaTime;
        
        // Keep agents within biome bounds
        FVector BiomeCenter = GetBiomeCenter(ECrowd_BiomeType::Savana); // Default to Savana
        float MaxDistance = 20000.0f;
        
        if (FVector::Dist(Agent.Position, BiomeCenter) > MaxDistance)
        {
            FVector Direction = (BiomeCenter - Agent.Position).GetSafeNormal();
            Agent.Velocity += Direction * 50.0f;
        }
    }
    
    CurrentUpdateIndex += AgentsToUpdate;
    if (CurrentUpdateIndex >= ActiveAgents.Num())
    {
        CurrentUpdateIndex = 0;
    }
}

void ACrowd_MassSimulationManager::UpdateAgentFlocking(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents)
{
    UpdateAgentSeparation(Agent, NearbyAgents);
    UpdateAgentAlignment(Agent, NearbyAgents);
    UpdateAgentCohesion(Agent, NearbyAgents);
    
    // Limit velocity
    if (Agent.Velocity.Size() > Agent.Speed)
    {
        Agent.Velocity = Agent.Velocity.GetSafeNormal() * Agent.Speed;
    }
}

void ACrowd_MassSimulationManager::UpdateAgentSeparation(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_AgentData& Other : NearbyAgents)
    {
        float Distance = FVector::Dist(Agent.Position, Other.Position);
        if (Distance > 0.0f && Distance < 200.0f)
        {
            FVector Diff = (Agent.Position - Other.Position).GetSafeNormal();
            Diff /= Distance; // Weight by distance
            SeparationForce += Diff;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce = SeparationForce.GetSafeNormal() * Agent.Speed;
        SeparationForce -= Agent.Velocity;
        Agent.Velocity += SeparationForce * 0.3f;
    }
}

void ACrowd_MassSimulationManager::UpdateAgentAlignment(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents)
{
    FVector AlignmentForce = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_AgentData& Other : NearbyAgents)
    {
        if (Other.GroupID == Agent.GroupID)
        {
            AlignmentForce += Other.Velocity;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        AlignmentForce /= Count;
        AlignmentForce = AlignmentForce.GetSafeNormal() * Agent.Speed;
        AlignmentForce -= Agent.Velocity;
        Agent.Velocity += AlignmentForce * 0.1f;
    }
}

void ACrowd_MassSimulationManager::UpdateAgentCohesion(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_AgentData& Other : NearbyAgents)
    {
        if (Other.GroupID == Agent.GroupID)
        {
            CenterOfMass += Other.Position;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        CenterOfMass /= Count;
        FVector CohesionForce = (CenterOfMass - Agent.Position).GetSafeNormal() * Agent.Speed;
        CohesionForce -= Agent.Velocity;
        Agent.Velocity += CohesionForce * 0.05f;
    }
}

void ACrowd_MassSimulationManager::ApplyLODCulling(const FVector& PlayerLocation)
{
    for (int32 i = SpawnedAgents.Num() - 1; i >= 0; --i)
    {
        if (!SpawnedAgents[i] || !IsValid(SpawnedAgents[i]))
        {
            SpawnedAgents.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, SpawnedAgents[i]->GetActorLocation());
        
        if (Distance > LODDistance3)
        {
            SpawnedAgents[i]->SetActorHiddenInGame(true);
            SpawnedAgents[i]->SetActorTickEnabled(false);
        }
        else if (Distance > LODDistance2)
        {
            SpawnedAgents[i]->SetActorHiddenInGame(false);
            SpawnedAgents[i]->SetActorTickEnabled(false);
        }
        else
        {
            SpawnedAgents[i]->SetActorHiddenInGame(false);
            SpawnedAgents[i]->SetActorTickEnabled(true);
        }
    }
}

void ACrowd_MassSimulationManager::MigrateAgentsBetweenBiomes(ECrowd_BiomeType FromBiome, ECrowd_BiomeType ToBiome, int32 AgentCount)
{
    FVector FromCenter = GetBiomeCenter(FromBiome);
    FVector ToCenter = GetBiomeCenter(ToBiome);
    
    if (FromCenter == FVector::ZeroVector || ToCenter == FVector::ZeroVector)
    {
        return;
    }
    
    int32 MigratedCount = 0;
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (MigratedCount >= AgentCount) break;
        
        if (FVector::Dist(Agent.Position, FromCenter) < 5000.0f)
        {
            FVector Direction = (ToCenter - Agent.Position).GetSafeNormal();
            Agent.Velocity += Direction * Agent.Speed * 0.5f;
            MigratedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Migrated %d agents from biome %d to %d"), 
           MigratedCount, (int32)FromBiome, (int32)ToBiome);
}

void ACrowd_MassSimulationManager::OptimizeSimulation()
{
    CleanupDeadAgents();
    
    // Reduce agent count if performance is poor
    if (ActiveAgents.Num() > MaxTotalAgents)
    {
        int32 AgentsToRemove = ActiveAgents.Num() - MaxTotalAgents;
        for (int32 i = 0; i < AgentsToRemove; ++i)
        {
            if (ActiveAgents.Num() > 0)
            {
                ActiveAgents.RemoveAt(ActiveAgents.Num() - 1);
            }
        }
    }
}

void ACrowd_MassSimulationManager::CleanupDeadAgents()
{
    for (int32 i = ActiveAgents.Num() - 1; i >= 0; --i)
    {
        if (ActiveAgents[i].Health <= 0.0f)
        {
            ActiveAgents.RemoveAt(i);
        }
    }
}

void ACrowd_MassSimulationManager::SpawnVisualAgent(const FCrowd_AgentData& AgentData)
{
    // Spawn a simple visual representation
    AActor* VisualActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), AgentData.Position, FRotator::ZeroRotator);
    if (VisualActor)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(VisualActor);
        VisualActor->SetRootComponent(MeshComp);
        
        // Set a simple cube mesh as placeholder
        static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMesh.Succeeded())
        {
            MeshComp->SetStaticMesh(CubeMesh.Object);
            MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
        }
        
        SpawnedAgents.Add(VisualActor);
    }
}

FVector ACrowd_MassSimulationManager::GetBiomeCenter(ECrowd_BiomeType BiomeType) const
{
    for (const FCrowd_BiomeLocation& Biome : BiomeLocations)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome.Location;
        }
    }
    return FVector::ZeroVector;
}

TArray<FCrowd_AgentData> ACrowd_MassSimulationManager::GetAgentsInRadius(const FVector& Center, float Radius) const
{
    TArray<FCrowd_AgentData> Result;
    
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (FVector::Dist(Agent.Position, Center) <= Radius)
        {
            Result.Add(Agent);
        }
    }
    
    return Result;
}

void ACrowd_MassSimulationManager::DebugDrawBiomes()
{
    if (!GetWorld()) return;
    
    for (const FCrowd_BiomeLocation& Biome : BiomeLocations)
    {
        DrawDebugSphere(GetWorld(), Biome.Location, Biome.Radius, 32, FColor::Green, false, 5.0f);
    }
}

int32 ACrowd_MassSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgents.Num();
}

float ACrowd_MassSimulationManager::GetSimulationPerformance() const
{
    return (float)ActiveAgents.Num() / (float)MaxTotalAgents;
}