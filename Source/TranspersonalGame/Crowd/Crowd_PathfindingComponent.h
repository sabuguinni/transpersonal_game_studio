#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingComponent.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_PathfindingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MaxPathDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathfindingRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AcceptanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bUseStringPulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bAllowPartialPaths = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathRecalculationInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 MaxPathPoints = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bAvoidOtherAgents = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AvoidanceRadius = 150.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_PathData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Data")
    TArray<FVector> PathPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Data")
    int32 CurrentPathIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Data")
    bool bIsPathValid = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Data")
    float PathLength = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Data")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Data")
    float LastPathUpdateTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Data")
    bool bReachedTarget = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_PathfindingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FCrowd_PathfindingSettings PathfindingSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    FCrowd_PathData CurrentPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bEnablePathfinding = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bDebugDrawPath = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    BiomeType PreferredBiome = BiomeType::Savanna;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindPathToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindPathToBiome(BiomeType TargetBiome);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindRandomPathInRadius(float Radius);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetNextPathPoint();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool HasValidPath() const { return CurrentPath.bIsPathValid; }

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool HasReachedTarget() const { return CurrentPath.bReachedTarget; }

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void ClearPath();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetPathfindingSettings(const FCrowd_PathfindingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> GetPathPoints() const { return CurrentPath.PathPoints; }

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 GetCurrentPathIndex() const { return CurrentPath.CurrentPathIndex; }

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool AdvanceToNextPathPoint();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector CalculateAvoidanceVector(const TArray<AActor*>& NearbyAgents);

protected:
    UFUNCTION()
    void UpdatePathProgress(float DeltaTime);

    UFUNCTION()
    void CheckPathRecalculation(float DeltaTime);

    UFUNCTION()
    bool IsLocationInPreferredBiome(const FVector& Location) const;

    UFUNCTION()
    FVector FindBiomeLocation(BiomeType TargetBiome) const;

    UFUNCTION()
    void OptimizePathWithStringPulling();

    UFUNCTION()
    void DrawDebugPath() const;

private:
    UNavigationSystemV1* NavigationSystem;
    float TimeSinceLastPathUpdate = 0.0f;
    
    bool ProcessNavigationPath(UNavigationPath* NavPath);
    void ValidateCurrentPath();
    FVector GetOwnerLocation() const;
};