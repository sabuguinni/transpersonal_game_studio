#include "Crowd_PathfindingManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

const float UCrowd_PathfindingManager::PATHFINDING_UPDATE_INTERVAL = 0.1f;
const float UCrowd_PathfindingManager::DENSITY_DECAY_RATE = 0.95f;
const int32 UCrowd_PathfindingManager::MAX_ACTIVE_REQUESTS = 100;

UCrowd_PathfindingManager::UCrowd_PathfindingManager()
{
    NextRequestID = 1;
    NavSystem = nullptr;
}

void UCrowd_PathfindingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingManager::Initialize - Starting pathfinding manager"));
    
    UWorld* World = GetWorld();
    if (World)
    {
        NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (NavSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Navigation system found and initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Navigation system not found - pathfinding will be limited"));
        }
        
        // Start pathfinding update timer
        World->GetTimerManager().SetTimer(
            PathfindingTimerHandle,
            this,
            &UCrowd_PathfindingManager::ProcessPathfindingRequests,
            PATHFINDING_UPDATE_INTERVAL,
            true
        );
        
        // Start density update timer
        World->GetTimerManager().SetTimer(
            DensityUpdateTimerHandle,
            this,
            &UCrowd_PathfindingManager::UpdateCrowdDensityGrid,
            1.0f,
            true
        );
    }
}

void UCrowd_PathfindingManager::Deinitialize()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(PathfindingTimerHandle);
        World->GetTimerManager().ClearTimer(DensityUpdateTimerHandle);
    }
    
    ActiveRequests.Empty();
    CompletedPaths.Empty();
    CrowdDensityMap.Empty();
    
    Super::Deinitialize();
}

bool UCrowd_PathfindingManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

int32 UCrowd_PathfindingManager::RequestPath(const FVector& StartLocation, const FVector& TargetLocation, float AgentRadius)
{
    if (ActiveRequests.Num() >= MAX_ACTIVE_REQUESTS)
    {
        UE_LOG(LogTemp, Warning, TEXT("Too many active pathfinding requests - rejecting new request"));
        return -1;
    }
    
    FCrowd_PathfindingRequest NewRequest;
    NewRequest.StartLocation = StartLocation;
    NewRequest.TargetLocation = TargetLocation;
    NewRequest.AgentRadius = AgentRadius;
    NewRequest.RequestID = NextRequestID++;
    NewRequest.bIsCompleted = false;
    
    ActiveRequests.Add(NewRequest);
    
    UE_LOG(LogTemp, Log, TEXT("Pathfinding request %d queued from %s to %s"), 
           NewRequest.RequestID, 
           *StartLocation.ToString(), 
           *TargetLocation.ToString());
    
    return NewRequest.RequestID;
}

bool UCrowd_PathfindingManager::GetPathResult(int32 RequestID, TArray<FVector>& OutPath)
{
    if (CompletedPaths.Contains(RequestID))
    {
        OutPath = CompletedPaths[RequestID];
        CompletedPaths.Remove(RequestID);
        return true;
    }
    
    return false;
}

void UCrowd_PathfindingManager::CancelPathRequest(int32 RequestID)
{
    ActiveRequests.RemoveAll([RequestID](const FCrowd_PathfindingRequest& Request)
    {
        return Request.RequestID == RequestID;
    });
    
    CompletedPaths.Remove(RequestID);
}

bool UCrowd_PathfindingManager::IsPathValid(const TArray<FVector>& Path)
{
    if (Path.Num() < 2)
    {
        return false;
    }
    
    if (!NavSystem)
    {
        return true; // Assume valid if no nav system
    }
    
    // Check if all points are on navmesh
    for (const FVector& Point : Path)
    {
        FNavLocation NavLoc;
        if (!NavSystem->ProjectPointToNavigation(Point, NavLoc, FVector(100.0f, 100.0f, 100.0f)))
        {
            return false;
        }
    }
    
    return true;
}

FVector UCrowd_PathfindingManager::GetRandomReachablePoint(const FVector& Origin, float Radius)
{
    if (!NavSystem)
    {
        // Fallback: random point in circle
        FVector2D RandomCircle = FMath::RandPointInCircle(Radius);
        return Origin + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
    }
    
    FNavLocation RandomNavLoc;
    if (NavSystem->GetRandomReachablePointInRadius(Origin, Radius, RandomNavLoc))
    {
        return RandomNavLoc.Location;
    }
    
    return Origin;
}

void UCrowd_PathfindingManager::UpdateNavMeshData()
{
    if (NavSystem)
    {
        // Force navmesh rebuild if needed
        NavSystem->Build();
        UE_LOG(LogTemp, Log, TEXT("NavMesh data updated"));
    }
}

bool UCrowd_PathfindingManager::CanReachLocation(const FVector& StartLocation, const FVector& TargetLocation)
{
    if (!NavSystem)
    {
        return true; // Assume reachable if no nav system
    }
    
    FPathFindingQuery Query;
    Query.StartLocation = StartLocation;
    Query.EndLocation = TargetLocation;
    Query.NavData = NavSystem->GetDefaultNavDataInstance();
    
    FPathFindingResult Result = NavSystem->FindPathSync(Query);
    return Result.IsSuccessful();
}

void UCrowd_PathfindingManager::RegisterCrowdDensity(const FVector& Location, int32 AgentCount)
{
    FVector QuantizedLoc = QuantizeLocation(Location);
    
    if (CrowdDensityMap.Contains(QuantizedLoc))
    {
        CrowdDensityMap[QuantizedLoc] += AgentCount;
    }
    else
    {
        CrowdDensityMap.Add(QuantizedLoc, AgentCount);
    }
}

float UCrowd_PathfindingManager::GetCrowdDensityAtLocation(const FVector& Location)
{
    FVector QuantizedLoc = QuantizeLocation(Location);
    
    if (CrowdDensityMap.Contains(QuantizedLoc))
    {
        return CrowdDensityMap[QuantizedLoc];
    }
    
    return 0.0f;
}

FVector UCrowd_PathfindingManager::GetFlowDirection(const FVector& Location)
{
    FVector FlowDirection = FVector::ZeroVector;
    float TotalWeight = 0.0f;
    
    // Sample surrounding density to determine flow
    for (int32 X = -1; X <= 1; X++)
    {
        for (int32 Y = -1; Y <= 1; Y++)
        {
            if (X == 0 && Y == 0) continue;
            
            FVector SampleLoc = Location + FVector(X * 100.0f, Y * 100.0f, 0.0f);
            float Density = GetCrowdDensityAtLocation(SampleLoc);
            
            if (Density > 0.0f)
            {
                FVector Direction = (SampleLoc - Location).GetSafeNormal();
                FlowDirection += Direction * Density;
                TotalWeight += Density;
            }
        }
    }
    
    if (TotalWeight > 0.0f)
    {
        FlowDirection /= TotalWeight;
    }
    
    return FlowDirection.GetSafeNormal();
}

void UCrowd_PathfindingManager::ProcessPathfindingRequests()
{
    if (ActiveRequests.Num() == 0)
    {
        return;
    }
    
    // Process up to 5 requests per frame to avoid hitches
    int32 ProcessedCount = 0;
    const int32 MaxProcessPerFrame = 5;
    
    for (int32 i = ActiveRequests.Num() - 1; i >= 0 && ProcessedCount < MaxProcessPerFrame; i--)
    {
        FCrowd_PathfindingRequest& Request = ActiveRequests[i];
        
        if (!Request.bIsCompleted)
        {
            TArray<FVector> ResultPath;
            if (FindPathInternal(Request, ResultPath))
            {
                CompletedPaths.Add(Request.RequestID, ResultPath);
                Request.bIsCompleted = true;
                
                UE_LOG(LogTemp, Log, TEXT("Pathfinding request %d completed with %d waypoints"), 
                       Request.RequestID, ResultPath.Num());
            }
            else
            {
                // Failed - create simple straight line path
                TArray<FVector> FallbackPath;
                FallbackPath.Add(Request.StartLocation);
                FallbackPath.Add(Request.TargetLocation);
                CompletedPaths.Add(Request.RequestID, FallbackPath);
                Request.bIsCompleted = true;
                
                UE_LOG(LogTemp, Warning, TEXT("Pathfinding request %d failed - using fallback straight line"), 
                       Request.RequestID);
            }
            
            ProcessedCount++;
        }
    }
    
    // Remove completed requests
    ActiveRequests.RemoveAll([](const FCrowd_PathfindingRequest& Request)
    {
        return Request.bIsCompleted;
    });
    
    // Clean up old completed paths
    CleanupCompletedRequests();
}

bool UCrowd_PathfindingManager::FindPathInternal(const FCrowd_PathfindingRequest& Request, TArray<FVector>& OutPath)
{
    if (!NavSystem)
    {
        return false;
    }
    
    FPathFindingQuery Query;
    Query.StartLocation = Request.StartLocation;
    Query.EndLocation = Request.TargetLocation;
    Query.NavData = NavSystem->GetDefaultNavDataInstance();
    
    FPathFindingResult Result = NavSystem->FindPathSync(Query);
    
    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        const TArray<FNavPathPoint>& PathPoints = Result.Path->GetPathPoints();
        OutPath.Reserve(PathPoints.Num());
        
        for (const FNavPathPoint& Point : PathPoints)
        {
            OutPath.Add(Point.Location);
        }
        
        return true;
    }
    
    return false;
}

void UCrowd_PathfindingManager::CleanupCompletedRequests()
{
    // Remove paths older than 30 seconds
    const float MaxAge = 30.0f;
    UWorld* World = GetWorld();
    if (!World) return;
    
    float CurrentTime = World->GetTimeSeconds();
    
    // This is a simplified cleanup - in a real implementation, 
    // you'd track creation time for each path
    if (CompletedPaths.Num() > 200)
    {
        // Remove oldest 50% of paths
        TArray<int32> Keys;
        CompletedPaths.GetKeys(Keys);
        
        int32 RemoveCount = Keys.Num() / 2;
        for (int32 i = 0; i < RemoveCount; i++)
        {
            CompletedPaths.Remove(Keys[i]);
        }
    }
}

FVector UCrowd_PathfindingManager::QuantizeLocation(const FVector& Location, float GridSize)
{
    return FVector(
        FMath::RoundToFloat(Location.X / GridSize) * GridSize,
        FMath::RoundToFloat(Location.Y / GridSize) * GridSize,
        FMath::RoundToFloat(Location.Z / GridSize) * GridSize
    );
}

void UCrowd_PathfindingManager::UpdateCrowdDensityGrid()
{
    // Decay all density values
    for (auto& Pair : CrowdDensityMap)
    {
        Pair.Value *= DENSITY_DECAY_RATE;
    }
    
    // Remove very low density entries
    for (auto It = CrowdDensityMap.CreateIterator(); It; ++It)
    {
        if (It.Value() < 0.1f)
        {
            It.RemoveCurrent();
        }
    }
}

void UCrowd_PathfindingManager::DecayCrowdDensity(float DeltaTime)
{
    float DecayFactor = FMath::Pow(DENSITY_DECAY_RATE, DeltaTime);
    
    for (auto& Pair : CrowdDensityMap)
    {
        Pair.Value *= DecayFactor;
    }
}