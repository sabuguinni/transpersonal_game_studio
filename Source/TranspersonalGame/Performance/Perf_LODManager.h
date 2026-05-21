#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    Ultra    UMETA(DisplayName = "Ultra (No LOD)"),
    High     UMETA(DisplayName = "High (LOD 0-1)"),
    Medium   UMETA(DisplayName = "Medium (LOD 0-2)"),
    Low      UMETA(DisplayName = "Low (LOD 0-3)"),
    Critical UMETA(DisplayName = "Critical (LOD 3+ only)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UltraDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CriticalDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance = 20000.0f;

    FPerf_LODSettings()
    {
        UltraDistance = 1000.0f;
        HighDistance = 2500.0f;
        MediumDistance = 5000.0f;
        LowDistance = 10000.0f;
        CriticalDistance = 15000.0f;
        bEnableDistanceCulling = true;
        CullingDistance = 20000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 UltraLODActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 HighLODActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 MediumLODActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 LowLODActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 CriticalLODActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 CulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    float AverageDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    float PerformanceImpact = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnLODLevelChanged, EPerf_LODLevel, NewLODLevel);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_LODManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bEnableAutomaticLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bEnablePerformanceBasedLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    float TargetFrameTime = 16.67f; // 60 FPS

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "LOD State")
    EPerf_LODLevel CurrentGlobalLODLevel = EPerf_LODLevel::High;

    UPROPERTY(BlueprintReadOnly, Category = "LOD State")
    FPerf_LODStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category = "LOD State")
    bool bIsOptimizing = false;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "LOD Events")
    FPerf_OnLODLevelChanged OnLODLevelChanged;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetGlobalLODLevel(EPerf_LODLevel NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODForAllActors();

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    EPerf_LODLevel CalculateLODLevelForDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void OptimizeBasedOnPerformance(float CurrentFrameTime);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterActorForLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterActorFromLOD(AActor* Actor);

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "LOD Statistics")
    FPerf_LODStats GetCurrentLODStats();

    UFUNCTION(BlueprintCallable, Category = "LOD Statistics")
    float GetEstimatedPerformanceGain();

    UFUNCTION(BlueprintCallable, Category = "LOD Statistics")
    int32 GetActorCountInLODLevel(EPerf_LODLevel LODLevel);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "LOD Utility")
    void SetLODSettingsFromPerformanceLevel(EPerf_LODLevel PerformanceLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD Utility")
    void ResetToDefaultLODSettings();

    UFUNCTION(BlueprintCallable, Category = "LOD Utility", CallInEditor = true)
    void DebugPrintLODStats();

private:
    // Internal tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ManagedActors;

    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, EPerf_LODLevel> ActorLODLevels;

    float TimeSinceLastUpdate = 0.0f;
    float LastFrameTime = 16.67f;
    int32 PerformanceAdjustmentCounter = 0;

    // Internal functions
    void UpdateManagedActorsList();
    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    float CalculateDistanceToPlayer(AActor* Actor);
    void UpdateStatistics();
    bool ShouldCullActor(AActor* Actor, float Distance);
};