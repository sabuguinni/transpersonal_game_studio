#include "CrowdSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize flocking parameters with realistic prehistoric values
    FlockingParams.SeparationRadius = 200.0f;
    FlockingParams.AlignmentRadius = 400.0f;
    FlockingParams.CohesionRadius = 600.0f;
    FlockingParams.SeparationWeight = 2.0f;
    FlockingParams.AlignmentWeight = 1.0f;
    FlockingParams.CohesionWeight = 1.0f;
    FlockingParams.MaxSpeed = 300.0f;
    FlockingParams.MaxForce = 100.0f;
    
    MaxAgents = 1000;
    SpawnRadius = 5000.0f;
    bUseFlocking = true;
    bAvoidPlayer = true;
    PlayerAvoidanceRadius = 800.0f;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: BeginPlay - Initializing prehistoric crowd simulation"));
    
    // Initialize the crowd system
    InitializeCrowd();
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (CrowdAgents.Num() > 0)
    {
        // Update flocking behavior
        if (bUseFlocking)
        {
            UpdateFlocking(DeltaTime);
        }
        
        // Update agent positions
        UpdateAgentPositions(DeltaTime);
    }
}

void ACrowdSimulationManager::InitializeCrowd()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Initializing crowd with %d max agents"), MaxAgents);
    
    // Clear existing agents
    ClearAllAgents();
    
    // Spawn initial agents
    SpawnAgents(FMath::Min(MaxAgents, 500)); // Start with 500 agents
}

void ACrowdSimulationManager::SpawnAgents(int32 NumAgents)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Spawning %d agents"), NumAgents);
    
    for (int32 i = 0; i < NumAgents; i++)
    {
        FCrowd_AgentData NewAgent;
        
        // Random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(100.0f, SpawnRadius);
        
        NewAgent.Position = GetActorLocation() + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            FMath::RandRange(-100.0f, 100.0f)
        );
        
        // Random initial velocity
        NewAgent.Velocity = FVector(
            FMath::RandRange(-50.0f, 50.0f),
            FMath::RandRange(-50.0f, 50.0f),
            0.0f
        );
        
        // Random agent type
        int32 TypeRand = FMath::RandRange(0, 2);
        switch (TypeRand)
        {
            case 0: NewAgent.AgentType = ECrowd_AgentType::Herbivore; break;
            case 1: NewAgent.AgentType = ECrowd_AgentType::Carnivore; break;
            case 2: NewAgent.AgentType = ECrowd_AgentType::Scavenger; break;
            default: NewAgent.AgentType = ECrowd_AgentType::Herbivore; break;
        }
        
        NewAgent.Speed = FMath::RandRange(80.0f, 200.0f);
        NewAgent.Health = 100.0f;
        NewAgent.bIsAlive = true;
        NewAgent.Fear = 0.0f;
        
        CrowdAgents.Add(NewAgent);
        CreateAgentMeshComponent(CrowdAgents.Num() - 1);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Successfully spawned %d agents, total: %d"), 
           NumAgents, CrowdAgents.Num());
}

void ACrowdSimulationManager::UpdateFlocking(float DeltaTime)
{
    // Calculate flocking forces for each agent
    for (int32 i = 0; i < CrowdAgents.Num(); i++)
    {
        if (!CrowdAgents[i].bIsAlive) continue;
        
        FVector Separation = CalculateSeparation(i);
        FVector Alignment = CalculateAlignment(i);
        FVector Cohesion = CalculateCohesion(i);
        FVector PlayerAvoidance = FVector::ZeroVector;
        
        if (bAvoidPlayer)
        {
            PlayerAvoidance = CalculatePlayerAvoidance(i);
        }
        
        // Combine forces
        FVector TotalForce = 
            Separation * FlockingParams.SeparationWeight +
            Alignment * FlockingParams.AlignmentWeight +
            Cohesion * FlockingParams.CohesionWeight +
            PlayerAvoidance * 3.0f; // Player avoidance has higher weight
        
        // Limit force magnitude
        if (TotalForce.Size() > FlockingParams.MaxForce)
        {
            TotalForce = TotalForce.GetSafeNormal() * FlockingParams.MaxForce;
        }
        
        // Update velocity
        CrowdAgents[i].Velocity += TotalForce * DeltaTime;
        
        // Limit velocity magnitude
        if (CrowdAgents[i].Velocity.Size() > FlockingParams.MaxSpeed)
        {
            CrowdAgents[i].Velocity = CrowdAgents[i].Velocity.GetSafeNormal() * FlockingParams.MaxSpeed;
        }
    }
}

FVector ACrowdSimulationManager::CalculateSeparation(int32 AgentIndex)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    const FCrowd_AgentData& CurrentAgent = CrowdAgents[AgentIndex];
    
    for (int32 i = 0; i < CrowdAgents.Num(); i++)
    {
        if (i == AgentIndex || !CrowdAgents[i].bIsAlive) continue;
        
        float Distance = FVector::Dist(CurrentAgent.Position, CrowdAgents[i].Position);
        
        if (Distance < FlockingParams.SeparationRadius && Distance > 0.0f)
        {
            FVector Diff = CurrentAgent.Position - CrowdAgents[i].Position;
            Diff = Diff.GetSafeNormal() / Distance; // Weight by distance
            SeparationForce += Diff;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce = SeparationForce.GetSafeNormal() * FlockingParams.MaxSpeed;
        SeparationForce -= CurrentAgent.Velocity;
    }
    
    return SeparationForce;
}

FVector ACrowdSimulationManager::CalculateAlignment(int32 AgentIndex)
{
    FVector AlignmentForce = FVector::ZeroVector;
    int32 Count = 0;
    
    const FCrowd_AgentData& CurrentAgent = CrowdAgents[AgentIndex];
    
    for (int32 i = 0; i < CrowdAgents.Num(); i++)
    {
        if (i == AgentIndex || !CrowdAgents[i].bIsAlive) continue;
        
        float Distance = FVector::Dist(CurrentAgent.Position, CrowdAgents[i].Position);
        
        if (Distance < FlockingParams.AlignmentRadius)
        {
            AlignmentForce += CrowdAgents[i].Velocity;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        AlignmentForce /= Count;
        AlignmentForce = AlignmentForce.GetSafeNormal() * FlockingParams.MaxSpeed;
        AlignmentForce -= CurrentAgent.Velocity;
    }
    
    return AlignmentForce;
}

FVector ACrowdSimulationManager::CalculateCohesion(int32 AgentIndex)
{
    FVector CohesionForce = FVector::ZeroVector;
    int32 Count = 0;
    
    const FCrowd_AgentData& CurrentAgent = CrowdAgents[AgentIndex];
    
    for (int32 i = 0; i < CrowdAgents.Num(); i++)
    {
        if (i == AgentIndex || !CrowdAgents[i].bIsAlive) continue;
        
        float Distance = FVector::Dist(CurrentAgent.Position, CrowdAgents[i].Position);
        
        if (Distance < FlockingParams.CohesionRadius)
        {
            CohesionForce += CrowdAgents[i].Position;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        CohesionForce /= Count;
        CohesionForce -= CurrentAgent.Position;
        CohesionForce = CohesionForce.GetSafeNormal() * FlockingParams.MaxSpeed;
        CohesionForce -= CurrentAgent.Velocity;
    }
    
    return CohesionForce;
}

FVector ACrowdSimulationManager::CalculatePlayerAvoidance(int32 AgentIndex)
{
    FVector AvoidanceForce = FVector::ZeroVector;
    
    FVector PlayerLocation = GetPlayerLocation();
    const FCrowd_AgentData& CurrentAgent = CrowdAgents[AgentIndex];
    
    float Distance = FVector::Dist(CurrentAgent.Position, PlayerLocation);
    
    if (Distance < PlayerAvoidanceRadius && Distance > 0.0f)
    {
        FVector Diff = CurrentAgent.Position - PlayerLocation;
        Diff = Diff.GetSafeNormal() / Distance; // Weight by distance
        AvoidanceForce = Diff * FlockingParams.MaxSpeed;
        
        // Increase fear based on proximity to player
        float FearIncrease = (PlayerAvoidanceRadius - Distance) / PlayerAvoidanceRadius;
        CrowdAgents[AgentIndex].Fear = FMath::Min(100.0f, CrowdAgents[AgentIndex].Fear + FearIncrease);
    }
    else
    {
        // Decrease fear when away from player
        CrowdAgents[AgentIndex].Fear = FMath::Max(0.0f, CrowdAgents[AgentIndex].Fear - 0.5f);
    }
    
    return AvoidanceForce;
}

void ACrowdSimulationManager::UpdateAgentPositions(float DeltaTime)
{
    for (int32 i = 0; i < CrowdAgents.Num(); i++)
    {
        if (!CrowdAgents[i].bIsAlive) continue;
        
        // Update position based on velocity
        CrowdAgents[i].Position += CrowdAgents[i].Velocity * DeltaTime;
        
        // Keep agents within bounds (simple boundary check)
        FVector ManagerLocation = GetActorLocation();
        float MaxDistance = SpawnRadius * 1.5f;
        
        if (FVector::Dist(CrowdAgents[i].Position, ManagerLocation) > MaxDistance)
        {
            FVector Direction = (ManagerLocation - CrowdAgents[i].Position).GetSafeNormal();
            CrowdAgents[i].Position = ManagerLocation + Direction * MaxDistance;
        }
        
        // Update visual representation
        UpdateAgentMeshTransform(i);
    }
}

FVector ACrowdSimulationManager::GetPlayerLocation() const
{
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            return PlayerPawn->GetActorLocation();
        }
    }
    return GetActorLocation();
}

void ACrowdSimulationManager::CreateAgentMeshComponent(int32 AgentIndex)
{
    if (AgentIndex < 0 || AgentIndex >= CrowdAgents.Num()) return;
    
    UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("AgentMesh_%d"), AgentIndex));
    
    if (MeshComp)
    {
        MeshComp->SetupAttachment(RootComponent);
        
        // Use a simple cube mesh as placeholder for now
        // In production, this would be replaced with proper prehistoric creature meshes
        if (!AgentMesh)
        {
            AgentMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        }
        
        if (AgentMesh)
        {
            MeshComp->SetStaticMesh(AgentMesh);
            MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
        }
        
        // Ensure we have enough space in the array
        while (AgentMeshComponents.Num() <= AgentIndex)
        {
            AgentMeshComponents.Add(nullptr);
        }
        
        AgentMeshComponents[AgentIndex] = MeshComp;
        UpdateAgentMeshTransform(AgentIndex);
    }
}

void ACrowdSimulationManager::UpdateAgentMeshTransform(int32 AgentIndex)
{
    if (AgentIndex < 0 || AgentIndex >= CrowdAgents.Num()) return;
    if (AgentIndex >= AgentMeshComponents.Num() || !AgentMeshComponents[AgentIndex]) return;
    
    const FCrowd_AgentData& Agent = CrowdAgents[AgentIndex];
    UStaticMeshComponent* MeshComp = AgentMeshComponents[AgentIndex];
    
    // Set position
    MeshComp->SetWorldLocation(Agent.Position);
    
    // Set rotation based on velocity direction
    if (Agent.Velocity.Size() > 0.1f)
    {
        FRotator NewRotation = Agent.Velocity.Rotation();
        MeshComp->SetWorldRotation(NewRotation);
    }
    
    // Set visibility based on agent state
    MeshComp->SetVisibility(Agent.bIsAlive);
}

void ACrowdSimulationManager::SetAgentType(int32 AgentIndex, ECrowd_AgentType NewType)
{
    if (AgentIndex >= 0 && AgentIndex < CrowdAgents.Num())
    {
        CrowdAgents[AgentIndex].AgentType = NewType;
        
        // Adjust agent properties based on type
        switch (NewType)
        {
            case ECrowd_AgentType::Herbivore:
                CrowdAgents[AgentIndex].Speed = FMath::RandRange(80.0f, 150.0f);
                break;
            case ECrowd_AgentType::Carnivore:
                CrowdAgents[AgentIndex].Speed = FMath::RandRange(150.0f, 250.0f);
                break;
            case ECrowd_AgentType::Scavenger:
                CrowdAgents[AgentIndex].Speed = FMath::RandRange(100.0f, 200.0f);
                break;
        }
    }
}

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : CrowdAgents)
    {
        if (Agent.bIsAlive)
        {
            Count++;
        }
    }
    return Count;
}

void ACrowdSimulationManager::ClearAllAgents()
{
    CrowdAgents.Empty();
    
    // Clean up mesh components
    for (UStaticMeshComponent* MeshComp : AgentMeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->DestroyComponent();
        }
    }
    AgentMeshComponents.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Cleared all agents"));
}