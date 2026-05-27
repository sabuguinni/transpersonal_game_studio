#include "Crowd_PathfindingManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"
#include "AI/Navigation/RecastNavMesh.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UCrowd_PathfindingManager::UCrowd_PathfindingManager()
{
    NavSystem = nullptr;
    MaxConcurrentRequests = 100;
    PathfindingTimeSlice = 0.016f; // 16ms per frame
    bUseAsyncPathfinding = true;
    CrowdDensityThreshold = 5.0f;
    AvoidanceStrength = 2.0f;
    NextRequestID = 1;
}

void UCrowd_PathfindingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingManager: Initializing pathfinding subsystem"));
    
    // Get navigation system reference
    if (UWorld* World = GetWorld())
    {
        NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (NavSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingManager: Navigation system found"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_PathfindingManager: No navigation system found"));
        }
        
        // Start async processing timer
        if (bUseAsyncPathfinding)
        {
            World->GetTimerManager().SetTimer(
                PathfindingTimerHandle,
                this,
                &UCrowd_PathfindingManager::ProcessPendingRequests,
                PathfindingTimeSlice,
                true
            );
        }
    }
}

void UCrowd_PathfindingManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PathfindingTimerHandle);
    }
    
    PendingRequests.Empty();
    CompletedPaths.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingManager: Subsystem deinitialized"));
    
    Super::Deinitialize();
}

bool UCrowd_PathfindingManager::FindPath(const FCrowd_PathRequest& Request, TArray<FVector>& OutPath)
{
    OutPath.Empty();
    
    if (!ValidatePathRequest(Request))
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingManager: Invalid path request"));
        return false;
    }
    
    return ProcessPathRequest(Request, OutPath);
}

bool UCrowd_PathfindingManager::FindPathAsync(const FCrowd_PathRequest& Request, int32& OutRequestID)
{
    if (!ValidatePathRequest(Request))
    {
        OutRequestID = -1;
        return false;
    }
    
    if (PendingRequests.Num() >= MaxConcurrentRequests)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingManager: Too many pending requests"));
        OutRequestID = -1;
        return false;
    }
    
    FCrowd_PathRequest NewRequest = Request;
    NewRequest.RequestID = NextRequestID++;
    OutRequestID = NewRequest.RequestID;
    
    // Insert high priority requests at the front
    if (NewRequest.bHighPriority)
    {
        PendingRequests.Insert(NewRequest, 0);
    }
    else
    {
        PendingRequests.Add(NewRequest);
    }
    
    return true;
}

bool UCrowd_PathfindingManager::GetAsyncPathResult(int32 RequestID, TArray<FVector>& OutPath)
{
    OutPath.Empty();
    
    if (TArray<FVector>* FoundPath = CompletedPaths.Find(RequestID))
    {
        OutPath = *FoundPath;
        CompletedPaths.Remove(RequestID);
        return true;
    }
    
    return false;
}

bool UCrowd_PathfindingManager::FindCrowdSafePath(const FVector& Start, const FVector& End, float CrowdRadius, TArray<FVector>& OutPath)
{
    OutPath.Empty();
    
    if (!NavSystem)
    {
        return false;
    }
    
    // Check if start and end locations are in crowded areas
    FVector SafeStart = Start;
    FVector SafeEnd = End;
    
    if (IsCrowdedArea(Start, CrowdRadius))
    {
        if (!AvoidCrowdedAreas(Start, CrowdRadius * 2.0f, SafeStart))
        {
            SafeStart = Start; // Fallback to original
        }
    }
    
    if (IsCrowdedArea(End, CrowdRadius))
    {
        if (!AvoidCrowdedAreas(End, CrowdRadius * 2.0f, SafeEnd))
        {
            SafeEnd = End; // Fallback to original
        }
    }
    
    // Create path request with adjusted locations
    FCrowd_PathRequest Request;
    Request.StartLocation = SafeStart;
    Request.EndLocation = SafeEnd;
    Request.MaxSearchDistance = FVector::Dist(SafeStart, SafeEnd) * 2.0f;
    
    return ProcessPathRequest(Request, OutPath);
}

bool UCrowd_PathfindingManager::AvoidCrowdedAreas(const FVector& Location, float AvoidanceRadius, FVector& OutSafeLocation)
{
    if (!NavSystem)
    {
        OutSafeLocation = Location;
        return false;
    }
    
    // Try multiple directions to find a safe location
    const int32 NumDirections = 8;
    const float AngleStep = 360.0f / NumDirections;
    
    for (int32 i = 0; i < NumDirections; i++)
    {
        float Angle = i * AngleStep;
        FVector Direction = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)),
            FMath::Sin(FMath::DegreesToRadians(Angle)),
            0.0f
        );
        
        FVector TestLocation = Location + (Direction * AvoidanceRadius);
        
        if (IsLocationNavigable(TestLocation) && !IsCrowdedArea(TestLocation, AvoidanceRadius * 0.5f))
        {
            OutSafeLocation = TestLocation;
            return true;
        }
    }
    
    OutSafeLocation = Location;
    return false;
}

bool UCrowd_PathfindingManager::IsLocationNavigable(const FVector& Location)
{
    if (!NavSystem)
    {
        return false;
    }
    
    FNavLocation NavLocation;
    return NavSystem->ProjectPointToNavigation(Location, NavLocation, FVector(100.0f, 100.0f, 500.0f));
}

bool UCrowd_PathfindingManager::GetRandomNavigableLocation(const FVector& Origin, float Radius, FVector& OutLocation)
{
    if (!NavSystem)
    {
        OutLocation = Origin;
        return false;
    }
    
    FNavLocation NavLocation;
    bool bFound = NavSystem->GetRandomReachablePointInRadius(Origin, Radius, NavLocation);
    
    if (bFound)
    {
        OutLocation = NavLocation.Location;
        return true;
    }
    
    OutLocation = Origin;
    return false;
}

void UCrowd_PathfindingManager::SetMaxConcurrentRequests(int32 MaxRequests)
{
    MaxConcurrentRequests = FMath::Max(1, MaxRequests);
    
    // Trim pending requests if necessary
    if (PendingRequests.Num() > MaxConcurrentRequests)
    {
        PendingRequests.SetNum(MaxConcurrentRequests);
    }
}

int32 UCrowd_PathfindingManager::GetPendingRequestCount() const
{
    return PendingRequests.Num();
}

void UCrowd_PathfindingManager::ClearAllRequests()
{
    PendingRequests.Empty();
    CompletedPaths.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingManager: All requests cleared"));
}

bool UCrowd_PathfindingManager::ProcessPathRequest(const FCrowd_PathRequest& Request, TArray<FVector>& OutPath)
{
    OutPath.Empty();
    
    if (!NavSystem)
    {
        return false;
    }
    
    // Project start and end points to navigation mesh
    FNavLocation StartNavLocation, EndNavLocation;
    
    if (!NavSystem->ProjectPointToNavigation(Request.StartLocation, StartNavLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingManager: Cannot project start location to navmesh"));
        return false;
    }
    
    if (!NavSystem->ProjectPointToNavigation(Request.EndLocation, EndNavLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingManager: Cannot project end location to navmesh"));
        return false;
    }
    
    // Find path using navigation system
    FPathFindingQuery Query;
    Query.StartLocation = StartNavLocation.Location;
    Query.EndLocation = EndNavLocation.Location;
    Query.NavData = NavSystem->GetDefaultNavDataInstance();
    
    FPathFindingResult Result = NavSystem->FindPathSync(Query);
    
    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        const TArray<FNavPathPoint>& PathPoints = Result.Path->GetPathPoints();
        
        for (const FNavPathPoint& Point : PathPoints)
        {
            OutPath.Add(Point.Location);
        }
        
        return true;
    }
    
    return false;
}

bool UCrowd_PathfindingManager::ValidatePathRequest(const FCrowd_PathRequest& Request)
{
    // Check if locations are valid
    if (Request.StartLocation.IsZero() || Request.EndLocation.IsZero())
    {
        return false;
    }
    
    // Check distance
    float Distance = FVector::Dist(Request.StartLocation, Request.EndLocation);
    if (Distance > Request.MaxSearchDistance)
    {
        return false;
    }
    
    return true;
}

void UCrowd_PathfindingManager::ProcessPendingRequests()
{
    if (PendingRequests.Num() == 0)
    {
        return;
    }
    
    // Process one request per frame to maintain performance
    FCrowd_PathRequest Request = PendingRequests[0];
    PendingRequests.RemoveAt(0);
    
    TArray<FVector> ResultPath;
    if (ProcessPathRequest(Request, ResultPath))
    {
        CompletedPaths.Add(Request.RequestID, ResultPath);
    }
    
    // Cleanup old completed paths periodically
    static int32 CleanupCounter = 0;
    if (++CleanupCounter >= 100)
    {
        CleanupCompletedRequests();
        CleanupCounter = 0;
    }
}

float UCrowd_PathfindingManager::CalculateCrowdDensity(const FVector& Location, float Radius)
{
    if (!GetWorld())
    {
        return 0.0f;
    }
    
    // Count nearby pawns/actors
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
    
    int32 CrowdCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && FVector::Dist(Actor->GetActorLocation(), Location) <= Radius)
        {
            CrowdCount++;
        }
    }
    
    // Calculate density (actors per square meter)
    float Area = PI * Radius * Radius / 10000.0f; // Convert to square meters
    return CrowdCount / FMath::Max(Area, 1.0f);
}

bool UCrowd_PathfindingManager::IsCrowdedArea(const FVector& Location, float Radius)
{
    float Density = CalculateCrowdDensity(Location, Radius);
    return Density > CrowdDensityThreshold;
}

void UCrowd_PathfindingManager::CleanupCompletedRequests()
{
    // Remove old completed paths to prevent memory bloat
    const int32 MaxCompletedPaths = 50;
    
    if (CompletedPaths.Num() > MaxCompletedPaths)
    {
        // Remove oldest entries (simple cleanup - could be improved with timestamps)
        TArray<int32> Keys;
        CompletedPaths.GetKeys(Keys);
        
        int32 ToRemove = CompletedPaths.Num() - MaxCompletedPaths;
        for (int32 i = 0; i < ToRemove && i < Keys.Num(); i++)
        {
            CompletedPaths.Remove(Keys[i]);
        }
    }
}