#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Perf_PhysicsLODOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsLODLevel : uint8
{
    FullPhysics     UMETA(DisplayName = "Full Physics"),
    ReducedPhysics  UMETA(DisplayName = "Reduced Physics"), 
    StaticOnly      UMETA(DisplayName = "Static Only"),
    Disabled        UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LOD0Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LOD1Distance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LOD2Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float CullingDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    bool bEnableFrameRateAdaptive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float TargetFrameRate = 60.0f;

    FPerf_PhysicsLODSettings()
    {
        LOD0Distance = 1000.0f;
        LOD1Distance = 2500.0f;
        LOD2Distance = 5000.0f;
        CullingDistance = 8000.0f;
        bEnableDistanceCulling = true;
        bEnableFrameRateAdaptive = true;
        TargetFrameRate = 60.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsObjectData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Data")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Data")
    EPerf_PhysicsLODLevel CurrentLODLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Data")
    float LastUpdateDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Data")
    bool bWasPhysicsEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Data")
    float LastUpdateTime;

    FPerf_PhysicsObjectData()
    {
        Actor = nullptr;
        CurrentLODLevel = EPerf_PhysicsLODLevel::FullPhysics;
        LastUpdateDistance = 0.0f;
        bWasPhysicsEnabled = true;
        LastUpdateTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsLODOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsLODOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD Settings")
    FPerf_PhysicsLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD Settings")
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD Settings")
    int32 MaxObjectsPerFrame = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD Settings")
    bool bDebugVisualization = false;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Physics LOD Runtime")
    TArray<FPerf_PhysicsObjectData> TrackedObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Physics LOD Runtime")
    int32 OptimizedObjectCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics LOD Runtime")
    float CurrentFrameRate = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics LOD Runtime")
    float AverageFrameTime = 16.67f;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void InitializePhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void UpdatePhysicsLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void RegisterPhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void UnregisterPhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    EPerf_PhysicsLODLevel GetOptimalLODLevel(float Distance, float FrameRate) const;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void ApplyPhysicsLOD(AActor* Actor, EPerf_PhysicsLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void ForceRefreshAllObjects();

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void SetLODSettings(const FPerf_PhysicsLODSettings& NewSettings);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    int32 GetTotalTrackedObjectCount() const;

    UFUNCTION(BlueprintPure, Category = "Physics LOD")
    FString GetPerformanceStats() const;

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Physics LOD Debug")
    void ToggleDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "Physics LOD Debug")
    void DrawDebugInfo(const FVector& ViewerLocation);

private:
    float LastUpdateTime = 0.0f;
    int32 CurrentUpdateIndex = 0;
    TArray<float> FrameTimeHistory;
    static constexpr int32 FrameHistorySize = 60;

    void UpdateFrameRateTracking(float DeltaTime);
    void ProcessObjectBatch(const FVector& ViewerLocation, int32 BatchSize);
    bool ShouldUpdateObject(const FPerf_PhysicsObjectData& ObjectData, float CurrentTime) const;
    void CleanupInvalidObjects();
};