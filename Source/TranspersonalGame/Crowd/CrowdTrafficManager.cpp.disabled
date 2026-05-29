#include "CrowdTrafficManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"

ACrowdTrafficManager::ACrowdTrafficManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    MaxTotalAgents = 1000;
    SpawnInterval = 2.0f;
    CullingDistance = 10000.0f;
    LastSpawnTime = 0.0f;
    
    bEnableLODSystem = true;
    HighDetailDistance = 2000.0f;
    MediumDetailDistance = 5000.0f;
    LowDetailDistance = 8000.0f;
}

void ACrowdTrafficManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTrafficSystem();
    CreateDefaultRoutes();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdTrafficManager initialized with %d routes"), TrafficRoutes.Num());
}

void ACrowdTrafficManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateAgentMovement(DeltaTime);
    
    if (bEnableLODSystem)
    {
        UpdateLODSystem();
    }
    
    // Spawn new agents periodically
    if (GetWorld()->GetTimeSeconds() - LastSpawnTime > SpawnInterval)
    {
        if (ActiveAgents.Num() < MaxTotalAgents && TrafficRoutes.Num() > 0)
        {
            int32 RandomRoute = FMath::RandRange(0, TrafficRoutes.Num() - 1);
            SpawnTrafficAgent(RandomRoute);
            LastSpawnTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void ACrowdTrafficManager::InitializeTrafficSystem()
{
    ActiveAgents.Empty();
    SpawnedAgentActors.Empty();
    LastSpawnTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Traffic system initialized"));
}

void ACrowdTrafficManager::AddTrafficRoute(const FCrowd_TrafficRoute& NewRoute)
{
    TrafficRoutes.Add(NewRoute);
    UE_LOG(LogTemp, Warning, TEXT("Added traffic route: %s with %d waypoints"), 
           *NewRoute.RouteName, NewRoute.Waypoints.Num());
}

void ACrowdTrafficManager::SpawnTrafficAgent(int32 RouteIndex)
{
    if (!TrafficRoutes.IsValidIndex(RouteIndex) || TrafficRoutes[RouteIndex].Waypoints.Num() == 0)
    {
        return;
    }
    
    FCrowd_TrafficAgent NewAgent;
    NewAgent.RouteID = RouteIndex;
    NewAgent.CurrentWaypointIndex = 0;
    NewAgent.CurrentPosition = TrafficRoutes[RouteIndex].Waypoints[0];
    NewAgent.TargetPosition = TrafficRoutes[RouteIndex].Waypoints.Num() > 1 ? 
                              TrafficRoutes[RouteIndex].Waypoints[1] : 
                              TrafficRoutes[RouteIndex].Waypoints[0];
    NewAgent.MovementSpeed = TrafficRoutes[RouteIndex].RouteSpeed + FMath::RandRange(-50.0f, 50.0f);
    NewAgent.bIsActive = true;
    
    ActiveAgents.Add(NewAgent);
    
    // Spawn visual actor if class is set
    if (AgentActorClass)
    {
        FVector SpawnLocation = NewAgent.CurrentPosition;
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(AgentActorClass, SpawnLocation, SpawnRotation);
        if (SpawnedActor)
        {
            SpawnedAgentActors.Add(SpawnedActor);
            SpawnedActor->SetActorLabel(FString::Printf(TEXT("TrafficAgent_%d"), ActiveAgents.Num()));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned traffic agent on route %d at %s"), 
           RouteIndex, *NewAgent.CurrentPosition.ToString());
}

void ACrowdTrafficManager::UpdateAgentMovement(float DeltaTime)
{
    for (int32 i = ActiveAgents.Num() - 1; i >= 0; i--)
    {
        FCrowd_TrafficAgent& Agent = ActiveAgents[i];
        
        if (!Agent.bIsActive)
        {
            continue;
        }
        
        // Check if route is valid
        if (!TrafficRoutes.IsValidIndex(Agent.RouteID))
        {
            ActiveAgents.RemoveAt(i);
            if (SpawnedAgentActors.IsValidIndex(i) && SpawnedAgentActors[i])
            {
                SpawnedAgentActors[i]->Destroy();
                SpawnedAgentActors.RemoveAt(i);
            }
            continue;
        }
        
        const FCrowd_TrafficRoute& Route = TrafficRoutes[Agent.RouteID];
        
        // Move towards target
        FVector Direction = (Agent.TargetPosition - Agent.CurrentPosition).GetSafeNormal();
        float MovementDistance = Agent.MovementSpeed * DeltaTime;
        Agent.CurrentPosition += Direction * MovementDistance;
        
        // Check if reached target waypoint
        float DistanceToTarget = FVector::Dist(Agent.CurrentPosition, Agent.TargetPosition);
        if (DistanceToTarget < 100.0f)
        {
            Agent.CurrentWaypointIndex++;
            
            // Check if completed route
            if (Agent.CurrentWaypointIndex >= Route.Waypoints.Num())
            {
                // Remove agent
                ActiveAgents.RemoveAt(i);
                if (SpawnedAgentActors.IsValidIndex(i) && SpawnedAgentActors[i])
                {
                    SpawnedAgentActors[i]->Destroy();
                    SpawnedAgentActors.RemoveAt(i);
                }
                continue;
            }
            
            // Set next target
            Agent.TargetPosition = Route.Waypoints[Agent.CurrentWaypointIndex];
        }
        
        // Update visual actor position
        if (SpawnedAgentActors.IsValidIndex(i) && SpawnedAgentActors[i])
        {
            SpawnedAgentActors[i]->SetActorLocation(Agent.CurrentPosition);
            
            // Set rotation to face movement direction
            if (!Direction.IsZero())
            {
                FRotator NewRotation = Direction.Rotation();
                SpawnedAgentActors[i]->SetActorRotation(NewRotation);
            }
        }
        
        // Cull agents too far from player
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            float DistanceToPlayer = FVector::Dist(Agent.CurrentPosition, PlayerPawn->GetActorLocation());
            if (DistanceToPlayer > CullingDistance)
            {
                ActiveAgents.RemoveAt(i);
                if (SpawnedAgentActors.IsValidIndex(i) && SpawnedAgentActors[i])
                {
                    SpawnedAgentActors[i]->Destroy();
                    SpawnedAgentActors.RemoveAt(i);
                }
            }
        }
    }
}

void ACrowdTrafficManager::UpdateLODSystem()
{
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (int32 i = 0; i < SpawnedAgentActors.Num(); i++)
    {
        if (!SpawnedAgentActors[i])
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, SpawnedAgentActors[i]->GetActorLocation());
        
        // Set LOD based on distance
        if (Distance < HighDetailDistance)
        {
            // High detail - full visibility
            SpawnedAgentActors[i]->SetActorHiddenInGame(false);
            SpawnedAgentActors[i]->SetActorTickEnabled(true);
        }
        else if (Distance < MediumDetailDistance)
        {
            // Medium detail - visible but reduced tick
            SpawnedAgentActors[i]->SetActorHiddenInGame(false);
            SpawnedAgentActors[i]->SetActorTickEnabled(i % 2 == 0); // Tick every other frame
        }
        else if (Distance < LowDetailDistance)
        {
            // Low detail - visible but no tick
            SpawnedAgentActors[i]->SetActorHiddenInGame(false);
            SpawnedAgentActors[i]->SetActorTickEnabled(false);
        }
        else
        {
            // Very far - hidden
            SpawnedAgentActors[i]->SetActorHiddenInGame(true);
            SpawnedAgentActors[i]->SetActorTickEnabled(false);
        }
    }
}

void ACrowdTrafficManager::ClearAllAgents()
{
    for (AActor* Actor : SpawnedAgentActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    SpawnedAgentActors.Empty();
    ActiveAgents.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Cleared all traffic agents"));
}

void ACrowdTrafficManager::CreateDefaultRoutes()
{
    // Savana migration route (circular)
    FCrowd_TrafficRoute SavanaRoute;
    SavanaRoute.RouteName = TEXT("Savana_Migration");
    SavanaRoute.RouteSpeed = 400.0f;
    SavanaRoute.MaxAgents = 200;
    SavanaRoute.Waypoints.Add(FVector(0, 0, 100));
    SavanaRoute.Waypoints.Add(FVector(5000, 0, 100));
    SavanaRoute.Waypoints.Add(FVector(5000, 5000, 100));
    SavanaRoute.Waypoints.Add(FVector(0, 5000, 100));
    SavanaRoute.Waypoints.Add(FVector(-5000, 5000, 100));
    SavanaRoute.Waypoints.Add(FVector(-5000, 0, 100));
    SavanaRoute.Waypoints.Add(FVector(-5000, -5000, 100));
    SavanaRoute.Waypoints.Add(FVector(0, -5000, 100));
    AddTrafficRoute(SavanaRoute);
    
    // Forest to water route
    FCrowd_TrafficRoute ForestRoute;
    ForestRoute.RouteName = TEXT("Forest_To_Water");
    ForestRoute.RouteSpeed = 300.0f;
    ForestRoute.MaxAgents = 150;
    ForestRoute.Waypoints.Add(FVector(-45000, 40000, 100));
    ForestRoute.Waypoints.Add(FVector(-30000, 30000, 100));
    ForestRoute.Waypoints.Add(FVector(-15000, 15000, 100));
    ForestRoute.Waypoints.Add(FVector(0, 0, 100));
    ForestRoute.Waypoints.Add(FVector(15000, -15000, 100));
    AddTrafficRoute(ForestRoute);
    
    // Desert patrol route
    FCrowd_TrafficRoute DesertRoute;
    DesertRoute.RouteName = TEXT("Desert_Patrol");
    DesertRoute.RouteSpeed = 250.0f;
    DesertRoute.MaxAgents = 100;
    DesertRoute.Waypoints.Add(FVector(55000, 0, 100));
    DesertRoute.Waypoints.Add(FVector(60000, 5000, 100));
    DesertRoute.Waypoints.Add(FVector(55000, 10000, 100));
    DesertRoute.Waypoints.Add(FVector(50000, 5000, 100));
    AddTrafficRoute(DesertRoute);
    
    UE_LOG(LogTemp, Warning, TEXT("Created %d default traffic routes"), TrafficRoutes.Num());
}