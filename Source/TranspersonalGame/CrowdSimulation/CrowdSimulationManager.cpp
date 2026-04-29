#include "CrowdSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    NextAgentID = 1;
    LastUpdateTime = 0.0f;
    bIsInitialized = false;
    MaxAgents = 1000;
    UpdateFrequency = 60.0f;
    CullingDistance = 5000.0f;
    
    // Initialize default flocking parameters
    FlockingParams.SeparationWeight = 2.0f;
    FlockingParams.AlignmentWeight = 1.0f;
    FlockingParams.CohesionWeight = 1.0f;
    FlockingParams.AvoidanceWeight = 3.0f;
    FlockingParams.SeekWeight = 2.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Initializing crowd simulation subsystem"));
    
    InitializeCrowdSimulation();
}

void UCrowdSimulationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Deinitializing crowd simulation subsystem"));
    
    // Clean up all agents
    CrowdAgents.Empty();
    AgentActors.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UCrowdSimulationManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCrowdSimulationManager::InitializeCrowdSimulation()
{
    if (bIsInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Setting up crowd simulation system"));
    
    // Reserve space for agents
    CrowdAgents.Reserve(MaxAgents);
    AgentActors.Reserve(MaxAgents);
    
    // Reset counters
    NextAgentID = 1;
    LastUpdateTime = 0.0f;
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Crowd simulation initialized successfully"));
}

void UCrowdSimulationManager::UpdateCrowdSimulation(float DeltaTime)
{
    if (!bIsInitialized || CrowdAgents.Num() == 0)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    // Update at specified frequency
    if (LastUpdateTime < (1.0f / UpdateFrequency))
    {
        return;
    }
    
    // Update each agent
    for (int32 i = CrowdAgents.Num() - 1; i >= 0; i--)
    {
        FCrowd_AgentData& Agent = CrowdAgents[i];
        
        if (!Agent.bIsActive)
        {
            continue;
        }
        
        // Calculate flocking forces
        TArray<FCrowd_AgentData> Neighbors = GetNeighbors(Agent, FMath::Max3(Agent.SeparationRadius, Agent.AlignmentRadius, Agent.CohesionRadius));
        
        FVector Separation = CalculateSeparation(Agent, Neighbors) * FlockingParams.SeparationWeight;
        FVector Alignment = CalculateAlignment(Agent, Neighbors) * FlockingParams.AlignmentWeight;
        FVector Cohesion = CalculateCohesion(Agent, Neighbors) * FlockingParams.CohesionWeight;
        FVector Seek = CalculateSeek(Agent, Agent.Destination) * FlockingParams.SeekWeight;
        FVector Avoidance = CalculateAvoidance(Agent) * FlockingParams.AvoidanceWeight;
        
        // Combine all forces
        FVector TotalForce = Separation + Alignment + Cohesion + Seek + Avoidance;
        
        // Apply force to velocity
        Agent.Velocity += TotalForce * LastUpdateTime;
        
        // Limit velocity
        if (Agent.Velocity.Size() > Agent.MaxSpeed)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * Agent.MaxSpeed;
        }
        
        // Update position
        UpdateAgentPosition(Agent, LastUpdateTime);
        
        // Update visual actor if it exists
        if (AActor** ActorPtr = AgentActors.Find(i + 1))
        {
            if (AActor* Actor = *ActorPtr)
            {
                Actor->SetActorLocation(Agent.Position);
                
                // Set rotation based on velocity
                if (!Agent.Velocity.IsNearlyZero())
                {
                    FRotator NewRotation = Agent.Velocity.Rotation();
                    Actor->SetActorRotation(NewRotation);
                }
            }
        }
    }
    
    LastUpdateTime = 0.0f;
}

int32 UCrowdSimulationManager::SpawnCrowdAgent(const FVector& Location, int32 GroupID)
{
    if (CrowdAgents.Num() >= MaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Cannot spawn agent - max agents reached"));
        return -1;
    }
    
    FCrowd_AgentData NewAgent;
    NewAgent.Position = Location;
    NewAgent.Destination = Location;
    NewAgent.GroupID = GroupID;
    NewAgent.bIsActive = true;
    
    int32 AgentID = NextAgentID++;
    CrowdAgents.Add(NewAgent);
    
    // Spawn visual representation
    AActor* AgentActor = SpawnAgentActor(Location);
    if (AgentActor)
    {
        AgentActors.Add(AgentID, AgentActor);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned agent %d at location %s"), AgentID, *Location.ToString());
    
    return AgentID;
}

void UCrowdSimulationManager::RemoveCrowdAgent(int32 AgentID)
{
    int32 AgentIndex = AgentID - 1;
    if (CrowdAgents.IsValidIndex(AgentIndex))
    {
        CrowdAgents[AgentIndex].bIsActive = false;
        
        // Remove visual actor
        if (AActor** ActorPtr = AgentActors.Find(AgentID))
        {
            if (AActor* Actor = *ActorPtr)
            {
                Actor->Destroy();
            }
            AgentActors.Remove(AgentID);
        }
        
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Removed agent %d"), AgentID);
    }
}

void UCrowdSimulationManager::SetAgentDestination(int32 AgentID, const FVector& Destination)
{
    int32 AgentIndex = AgentID - 1;
    if (CrowdAgents.IsValidIndex(AgentIndex))
    {
        CrowdAgents[AgentIndex].Destination = Destination;
    }
}

void UCrowdSimulationManager::SetGroupDestination(int32 GroupID, const FVector& Destination)
{
    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.GroupID == GroupID && Agent.bIsActive)
        {
            Agent.Destination = Destination + FMath::VRand() * 100.0f; // Add some randomness
        }
    }
}

void UCrowdSimulationManager::SetFlockingParameters(const FCrowd_FlockingParameters& NewParameters)
{
    FlockingParams = NewParameters;
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAllAgentData() const
{
    TArray<FCrowd_AgentData> ActiveAgents;
    for (const FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.bIsActive)
        {
            ActiveAgents.Add(Agent);
        }
    }
    return ActiveAgents;
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.bIsActive)
        {
            Count++;
        }
    }
    return Count;
}

void UCrowdSimulationManager::SetAgentBehavior(int32 AgentID, ECrowd_BehaviorState NewBehavior)
{
    // Implementation for behavior state changes
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Set agent %d behavior to %d"), AgentID, (int32)NewBehavior);
}

void UCrowdSimulationManager::SetGroupBehavior(int32 GroupID, ECrowd_BehaviorState NewBehavior)
{
    // Implementation for group behavior changes
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Set group %d behavior to %d"), GroupID, (int32)NewBehavior);
}

void UCrowdSimulationManager::CreateDinosaurHerd(const FVector& CenterLocation, int32 HerdSize, float HerdRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Creating dinosaur herd with %d members at %s"), HerdSize, *CenterLocation.ToString());
    
    int32 HerdGroupID = NextAgentID + 1000; // Use high numbers for group IDs
    
    for (int32 i = 0; i < HerdSize; i++)
    {
        FVector RandomOffset = FMath::VRand() * HerdRadius;
        RandomOffset.Z = 0.0f; // Keep on ground level
        FVector SpawnLocation = CenterLocation + RandomOffset;
        
        int32 AgentID = SpawnCrowdAgent(SpawnLocation, HerdGroupID);
        if (AgentID != -1)
        {
            SetAgentBehavior(AgentID, ECrowd_BehaviorState::Flocking);
        }
    }
    
    // Set initial herd destination
    FVector HerdDestination = CenterLocation + FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), 0);
    SetGroupDestination(HerdGroupID, HerdDestination);
}

void UCrowdSimulationManager::CreateRaptorPack(const FVector& CenterLocation, int32 PackSize)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Creating raptor pack with %d members at %s"), PackSize, *CenterLocation.ToString());
    
    int32 PackGroupID = NextAgentID + 2000; // Use different range for pack IDs
    
    for (int32 i = 0; i < PackSize; i++)
    {
        FVector RandomOffset = FMath::VRand() * 200.0f; // Tighter formation for pack
        RandomOffset.Z = 0.0f;
        FVector SpawnLocation = CenterLocation + RandomOffset;
        
        int32 AgentID = SpawnCrowdAgent(SpawnLocation, PackGroupID);
        if (AgentID != -1)
        {
            SetAgentBehavior(AgentID, ECrowd_BehaviorState::Hunting);
            
            // Make raptors faster and more aggressive
            int32 AgentIndex = AgentID - 1;
            if (CrowdAgents.IsValidIndex(AgentIndex))
            {
                CrowdAgents[AgentIndex].MaxSpeed = 500.0f;
                CrowdAgents[AgentIndex].Speed = 300.0f;
                CrowdAgents[AgentIndex].SeparationRadius = 50.0f;
            }
        }
    }
}

void UCrowdSimulationManager::TriggerHerdPanic(const FVector& ThreatLocation, float PanicRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Triggering herd panic at %s with radius %f"), *ThreatLocation.ToString(), PanicRadius);
    
    for (FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (!Agent.bIsActive)
        {
            continue;
        }
        
        float DistanceToThreat = FVector::Dist(Agent.Position, ThreatLocation);
        if (DistanceToThreat <= PanicRadius)
        {
            // Set flee destination opposite to threat
            FVector FleeDirection = (Agent.Position - ThreatLocation).GetSafeNormal();
            Agent.Destination = Agent.Position + FleeDirection * 2000.0f;
            Agent.MaxSpeed = 600.0f; // Panic speed boost
            
            // Change behavior to fleeing
            // SetAgentBehavior would be called here in a full implementation
        }
    }
}

// Core flocking algorithm implementations
FVector UCrowdSimulationManager::CalculateSeparation(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_AgentData& Neighbor : Neighbors)
    {
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        if (Distance > 0 && Distance < Agent.SeparationRadius)
        {
            FVector Diff = Agent.Position - Neighbor.Position;
            Diff.Normalize();
            Diff /= Distance; // Weight by distance
            SeparationForce += Diff;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce.Normalize();
        SeparationForce *= Agent.MaxSpeed;
        SeparationForce -= Agent.Velocity;
    }
    
    return SeparationForce;
}

FVector UCrowdSimulationManager::CalculateAlignment(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors)
{
    FVector AlignmentForce = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_AgentData& Neighbor : Neighbors)
    {
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        if (Distance > 0 && Distance < Agent.AlignmentRadius)
        {
            AlignmentForce += Neighbor.Velocity;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        AlignmentForce /= Count;
        AlignmentForce.Normalize();
        AlignmentForce *= Agent.MaxSpeed;
        AlignmentForce -= Agent.Velocity;
    }
    
    return AlignmentForce;
}

FVector UCrowdSimulationManager::CalculateCohesion(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_AgentData& Neighbor : Neighbors)
    {
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        if (Distance > 0 && Distance < Agent.CohesionRadius)
        {
            CenterOfMass += Neighbor.Position;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        CenterOfMass /= Count;
        return CalculateSeek(Agent, CenterOfMass);
    }
    
    return FVector::ZeroVector;
}

FVector UCrowdSimulationManager::CalculateSeek(const FCrowd_AgentData& Agent, const FVector& Target)
{
    FVector Desired = Target - Agent.Position;
    Desired.Normalize();
    Desired *= Agent.MaxSpeed;
    
    FVector Steer = Desired - Agent.Velocity;
    return Steer;
}

FVector UCrowdSimulationManager::CalculateAvoidance(const FCrowd_AgentData& Agent)
{
    // Simple obstacle avoidance - can be expanded with proper collision detection
    return FVector::ZeroVector;
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetNeighbors(const FCrowd_AgentData& Agent, float Radius)
{
    TArray<FCrowd_AgentData> Neighbors;
    
    for (const FCrowd_AgentData& OtherAgent : CrowdAgents)
    {
        if (!OtherAgent.bIsActive || OtherAgent.Position == Agent.Position)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Position, OtherAgent.Position);
        if (Distance <= Radius)
        {
            Neighbors.Add(OtherAgent);
        }
    }
    
    return Neighbors;
}

void UCrowdSimulationManager::UpdateAgentPosition(FCrowd_AgentData& Agent, float DeltaTime)
{
    Agent.Position += Agent.Velocity * DeltaTime;
    
    // Basic boundary checking - keep agents in reasonable bounds
    Agent.Position.X = FMath::Clamp(Agent.Position.X, -10000.0f, 10000.0f);
    Agent.Position.Y = FMath::Clamp(Agent.Position.Y, -10000.0f, 10000.0f);
    Agent.Position.Z = FMath::Max(Agent.Position.Z, 0.0f); // Don't go underground
}

bool UCrowdSimulationManager::IsValidLocation(const FVector& Location)
{
    // Basic validation - can be expanded with navigation mesh checks
    return true;
}

AActor* UCrowdSimulationManager::SpawnAgentActor(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Spawn a simple static mesh actor as visual representation
    AActor* NewActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (NewActor)
    {
        // Set a simple cube mesh for visualization
        UStaticMeshComponent* MeshComp = NewActor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            // Try to load a basic cube mesh
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            if (CubeMesh)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f)); // Make it smaller
            }
        }
        
        NewActor->SetActorLabel(FString::Printf(TEXT("CrowdAgent_%d"), NextAgentID));
    }
    
    return NewActor;
}