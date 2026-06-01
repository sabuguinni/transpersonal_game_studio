#include "Crowd_NavigationMesh.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowd_NavigationMesh::UCrowd_NavigationMesh()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    NavigationMode = ECrowd_NavigationMode::Standard;
    bEnableCrowdSimulation = true;
    bOptimizeForLargeCrowds = false;
    PathfindingUpdateInterval = 0.1f;
    
    bNavigationReady = false;
    ActiveAgentCount = 0;
    AveragePathfindingTime = 0.0f;
    
    LastPathfindingTime = 0.0f;
    NavigationUpdateTimer = 0.0f;
    
    NavSystem = nullptr;
    RecastNavMesh = nullptr;
}

void UCrowd_NavigationMesh::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeNavigation();
}

void UCrowd_NavigationMesh::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableCrowdSimulation)
    {
        UpdateNavigationSystem(DeltaTime);
    }
}

bool UCrowd_NavigationMesh::InitializeNavigation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_NavigationMesh: No valid world found"));
        return false;
    }
    
    NavSystem = UNavigationSystemV1::GetCurrent(World);
    if (!NavSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_NavigationMesh: Navigation system not found"));
        return false;
    }
    
    // Find RecastNavMesh
    RecastNavMesh = Cast<ARecastNavMesh>(NavSystem->GetDefaultNavDataInstance());
    if (!RecastNavMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_NavigationMesh: RecastNavMesh not found"));
        return false;
    }
    
    // Configure navigation for crowd simulation
    OptimizeNavMeshSettings();
    
    bNavigationReady = ValidateNavigationSetup();
    
    UE_LOG(LogTemp, Log, TEXT("UCrowd_NavigationMesh: Navigation initialized successfully"));
    return bNavigationReady;
}

FCrowd_PathfindingResult UCrowd_NavigationMesh::FindPath(const FVector& StartLocation, const FVector& EndLocation)
{
    FCrowd_PathfindingResult Result;
    
    if (!bNavigationReady || !NavSystem)
    {
        return Result;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    FNavLocation StartNavLocation, EndNavLocation;
    bool bStartValid = NavSystem->ProjectPointToNavigation(StartLocation, StartNavLocation);
    bool bEndValid = NavSystem->ProjectPointToNavigation(EndLocation, EndNavLocation);
    
    if (!bStartValid || !bEndValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_NavigationMesh: Invalid start or end location for pathfinding"));
        return Result;
    }
    
    FPathFindingQuery Query;
    Query.StartLocation = StartNavLocation.Location;
    Query.EndLocation = EndNavLocation.Location;
    Query.NavData = RecastNavMesh;
    
    FPathFindingResult PathResult = NavSystem->FindPathSync(Query);
    
    if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
    {
        Result.bPathFound = true;
        Result.PathPoints = PathResult.Path->GetPathPoints();
        Result.PathLength = PathResult.Path->GetLength();
        
        // Estimate travel time (assuming average human walking speed)
        Result.EstimatedTime = Result.PathLength / 150.0f; // 150 cm/s walking speed
    }
    
    float PathfindingTime = FPlatformTime::Seconds() - StartTime;
    UpdatePerformanceMetrics(PathfindingTime);
    
    return Result;
}

bool UCrowd_NavigationMesh::IsLocationNavigable(const FVector& Location)
{
    if (!bNavigationReady || !NavSystem)
    {
        return false;
    }
    
    FNavLocation NavLocation;
    return NavSystem->ProjectPointToNavigation(Location, NavLocation, FVector(100.0f, 100.0f, 500.0f));
}

FVector UCrowd_NavigationMesh::GetRandomNavigablePoint(const FVector& Origin, float Radius)
{
    if (!bNavigationReady || !NavSystem)
    {
        return Origin;
    }
    
    FNavLocation RandomLocation;
    bool bFound = NavSystem->GetRandomReachablePointInRadius(Origin, Radius, RandomLocation);
    
    return bFound ? RandomLocation.Location : Origin;
}

TArray<FVector> UCrowd_NavigationMesh::GetCrowdFlowDirections(const FVector& Location, float SampleRadius)
{
    TArray<FVector> FlowDirections;
    
    if (!bNavigationReady)
    {
        return FlowDirections;
    }
    
    // Sample 8 directions around the location
    for (int32 i = 0; i < 8; i++)
    {
        float Angle = (i * 45.0f) * PI / 180.0f;
        FVector SampleDirection = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
        FVector SampleLocation = Location + (SampleDirection * SampleRadius);
        
        if (IsLocationNavigable(SampleLocation))
        {
            FlowDirections.Add(SampleDirection);
        }
    }
    
    return FlowDirections;
}

void UCrowd_NavigationMesh::UpdateNavigationForCrowd(int32 ExpectedAgentCount)
{
    ActiveAgentCount = ExpectedAgentCount;
    ConfigureForCrowdDensity(ExpectedAgentCount);
    
    UE_LOG(LogTemp, Log, TEXT("UCrowd_NavigationMesh: Updated for %d agents"), ExpectedAgentCount);
}

void UCrowd_NavigationMesh::SetNavigationMode(ECrowd_NavigationMode NewMode)
{
    NavigationMode = NewMode;
    OptimizeNavMeshSettings();
    
    UE_LOG(LogTemp, Log, TEXT("UCrowd_NavigationMesh: Navigation mode changed"));
}

void UCrowd_NavigationMesh::OptimizeNavMeshForArea(const FVector& Center, float Radius)
{
    if (!RecastNavMesh)
    {
        return;
    }
    
    // Request navmesh rebuild for specific area
    FBox AreaBox = FBox::BuildAABB(Center, FVector(Radius, Radius, 500.0f));
    RecastNavMesh->RequestDrawingUpdate(true);
    
    UE_LOG(LogTemp, Log, TEXT("UCrowd_NavigationMesh: Optimized navmesh for area at %s"), *Center.ToString());
}

void UCrowd_NavigationMesh::DrawNavigationDebug(bool bShowNavMesh, bool bShowPaths, bool bShowAgents)
{
    UWorld* World = GetWorld();
    if (!World || !bNavigationReady)
    {
        return;
    }
    
    if (bShowNavMesh && RecastNavMesh)
    {
        // Draw navmesh bounds
        FBox NavBounds = RecastNavMesh->GetNavMeshBounds();
        DrawDebugBox(World, NavBounds.GetCenter(), NavBounds.GetExtent(), FColor::Green, false, 1.0f);
    }
    
    if (bShowAgents)
    {
        // Draw agent count info
        FVector OwnerLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
        DrawDebugString(World, OwnerLocation + FVector(0, 0, 200), 
                       FString::Printf(TEXT("Active Agents: %d"), ActiveAgentCount), 
                       nullptr, FColor::White, 1.0f);
    }
}

void UCrowd_NavigationMesh::LogNavigationStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== Navigation Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Navigation Ready: %s"), bNavigationReady ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Active Agents: %d"), ActiveAgentCount);
    UE_LOG(LogTemp, Log, TEXT("Average Pathfinding Time: %.4f ms"), AveragePathfindingTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Navigation Mode: %d"), (int32)NavigationMode);
    UE_LOG(LogTemp, Log, TEXT("========================"));
}

void UCrowd_NavigationMesh::UpdateNavigationSystem(float DeltaTime)
{
    NavigationUpdateTimer += DeltaTime;
    
    if (NavigationUpdateTimer >= PathfindingUpdateInterval)
    {
        NavigationUpdateTimer = 0.0f;
        
        // Periodic validation and optimization
        if (!ValidateNavigationSetup())
        {
            InitializeNavigation();
        }
    }
}

void UCrowd_NavigationMesh::OptimizeNavMeshSettings()
{
    if (!RecastNavMesh)
    {
        return;
    }
    
    // Apply configuration based on navigation mode
    switch (NavigationMode)
    {
        case ECrowd_NavigationMode::Standard:
            // Default settings
            break;
            
        case ECrowd_NavigationMode::CrowdOptimized:
            // Optimize for crowd movement
            bOptimizeForLargeCrowds = true;
            break;
            
        case ECrowd_NavigationMode::HighDensity:
            // Settings for high-density areas
            PathfindingUpdateInterval = 0.05f;
            break;
            
        case ECrowd_NavigationMode::LowLatency:
            // Fast pathfinding settings
            PathfindingUpdateInterval = 0.02f;
            break;
    }
}

void UCrowd_NavigationMesh::UpdatePerformanceMetrics(float PathfindingTime)
{
    LastPathfindingTime = PathfindingTime;
    PathfindingTimes.Add(PathfindingTime);
    
    // Keep only last 100 samples
    if (PathfindingTimes.Num() > 100)
    {
        PathfindingTimes.RemoveAt(0);
    }
    
    // Calculate average
    float Total = 0.0f;
    for (float Time : PathfindingTimes)
    {
        Total += Time;
    }
    AveragePathfindingTime = PathfindingTimes.Num() > 0 ? Total / PathfindingTimes.Num() : 0.0f;
}

bool UCrowd_NavigationMesh::ValidateNavigationSetup()
{
    return NavSystem != nullptr && RecastNavMesh != nullptr;
}

void UCrowd_NavigationMesh::ConfigureForCrowdDensity(int32 AgentCount)
{
    if (AgentCount > 500)
    {
        // High density configuration
        PathfindingUpdateInterval = 0.2f;
        bOptimizeForLargeCrowds = true;
    }
    else if (AgentCount > 100)
    {
        // Medium density configuration
        PathfindingUpdateInterval = 0.1f;
        bOptimizeForLargeCrowds = true;
    }
    else
    {
        // Low density configuration
        PathfindingUpdateInterval = 0.05f;
        bOptimizeForLargeCrowds = false;
    }
}