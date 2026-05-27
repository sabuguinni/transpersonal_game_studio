#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "AI/NavigationSystemBase.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 NodeID;

    FCrowd_PathNode()
    {
        Location = FVector::ZeroVector;
        Cost = 1.0f;
        bIsBlocked = false;
        NodeID = -1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MaxSearchDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 RequestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bHighPriority;

    FCrowd_PathRequest()
    {
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector::ZeroVector;
        MaxSearchDistance = 10000.0f;
        RequestID = -1;
        bHighPriority = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_PathfindingManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core pathfinding functionality
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool FindPath(const FCrowd_PathRequest& Request, TArray<FVector>& OutPath);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool FindPathAsync(const FCrowd_PathRequest& Request, int32& OutRequestID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool GetAsyncPathResult(int32 RequestID, TArray<FVector>& OutPath);

    // Crowd-specific pathfinding
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool FindCrowdSafePath(const FVector& Start, const FVector& End, float CrowdRadius, TArray<FVector>& OutPath);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool AvoidCrowdedAreas(const FVector& Location, float AvoidanceRadius, FVector& OutSafeLocation);

    // Navigation mesh utilities
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsLocationNavigable(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool GetRandomNavigableLocation(const FVector& Origin, float Radius, FVector& OutLocation);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void SetMaxConcurrentRequests(int32 MaxRequests);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    int32 GetPendingRequestCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void ClearAllRequests();

protected:
    // Navigation system reference
    UPROPERTY()
    UNavigationSystemV1* NavSystem;

    // Pathfinding data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_PathRequest> PendingRequests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    TMap<int32, TArray<FVector>> CompletedPaths;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxConcurrentRequests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float PathfindingTimeSlice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bUseAsyncPathfinding;

    // Crowd avoidance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Avoidance", meta = (AllowPrivateAccess = "true"))
    float CrowdDensityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Avoidance", meta = (AllowPrivateAccess = "true"))
    float AvoidanceStrength;

private:
    // Internal pathfinding helpers
    bool ProcessPathRequest(const FCrowd_PathRequest& Request, TArray<FVector>& OutPath);
    bool ValidatePathRequest(const FCrowd_PathRequest& Request);
    void ProcessPendingRequests();
    
    // Crowd density calculation
    float CalculateCrowdDensity(const FVector& Location, float Radius);
    bool IsCrowdedArea(const FVector& Location, float Radius);
    
    // Request management
    int32 NextRequestID;
    void CleanupCompletedRequests();

    // Timer for async processing
    FTimerHandle PathfindingTimerHandle;
};