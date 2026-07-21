#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "Crowd_NavigationMesh.generated.h"

UENUM(BlueprintType)
enum class ECrowd_NavigationMode : uint8
{
    Standard        UMETA(DisplayName = "Standard Navigation"),
    CrowdOptimized  UMETA(DisplayName = "Crowd Optimized"),
    HighDensity     UMETA(DisplayName = "High Density Areas"),
    LowLatency      UMETA(DisplayName = "Low Latency Pathfinding")
};

USTRUCT(BlueprintType)
struct FCrowd_NavMeshConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float CellSize = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float CellHeight = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float AgentRadius = 34.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float AgentHeight = 144.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float AgentMaxClimb = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float AgentMaxSlope = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    int32 MaxCrowdAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float CrowdSeparationWeight = 2.0f;

    FCrowd_NavMeshConfig()
    {
        CellSize = 300.0f;
        CellHeight = 200.0f;
        AgentRadius = 34.0f;
        AgentHeight = 144.0f;
        AgentMaxClimb = 420.0f;
        AgentMaxSlope = 45.0f;
        MaxCrowdAgents = 1000;
        CrowdSeparationWeight = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_PathfindingResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
    bool bPathFound = false;

    UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
    TArray<FVector> PathPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
    float PathLength = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
    float EstimatedTime = 0.0f;

    FCrowd_PathfindingResult()
    {
        bPathFound = false;
        PathLength = 0.0f;
        EstimatedTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_NavigationMesh : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_NavigationMesh();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Navigation Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Config")
    ECrowd_NavigationMode NavigationMode = ECrowd_NavigationMode::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Config")
    FCrowd_NavMeshConfig NavMeshConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Config")
    bool bEnableCrowdSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Config")
    bool bOptimizeForLargeCrowds = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation Config")
    float PathfindingUpdateInterval = 0.1f;

    // Runtime Properties
    UPROPERTY(BlueprintReadOnly, Category = "Navigation State")
    bool bNavigationReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Navigation State")
    int32 ActiveAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Navigation State")
    float AveragePathfindingTime = 0.0f;

    // Navigation Functions
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    bool InitializeNavigation();

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    FCrowd_PathfindingResult FindPath(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    bool IsLocationNavigable(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    FVector GetRandomNavigablePoint(const FVector& Origin, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    TArray<FVector> GetCrowdFlowDirections(const FVector& Location, float SampleRadius);

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void UpdateNavigationForCrowd(int32 ExpectedAgentCount);

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void SetNavigationMode(ECrowd_NavigationMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void OptimizeNavMeshForArea(const FVector& Center, float Radius);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Navigation Debug")
    void DrawNavigationDebug(bool bShowNavMesh, bool bShowPaths, bool bShowAgents);

    UFUNCTION(BlueprintCallable, Category = "Navigation Debug")
    void LogNavigationStats();

private:
    // Internal Navigation Data
    UPROPERTY()
    class UNavigationSystemV1* NavSystem;

    UPROPERTY()
    class ARecastNavMesh* RecastNavMesh;

    // Performance Tracking
    float LastPathfindingTime;
    TArray<float> PathfindingTimes;
    float NavigationUpdateTimer;

    // Internal Functions
    void UpdateNavigationSystem(float DeltaTime);
    void OptimizeNavMeshSettings();
    void UpdatePerformanceMetrics(float PathfindingTime);
    bool ValidateNavigationSetup();
    void ConfigureForCrowdDensity(int32 AgentCount);
};