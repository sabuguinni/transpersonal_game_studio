#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Engine/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "Perf_DinosaurPerformanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinosaurLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDetailDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDetailDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxVisibleDinosaurs = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableAnimationCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnablePhysicsCulling = true;

    FPerf_DinosaurLODSettings()
    {
        HighDetailDistance = 2000.0f;
        MediumDetailDistance = 5000.0f;
        LowDetailDistance = 10000.0f;
        CullingDistance = 15000.0f;
        MaxVisibleDinosaurs = 50;
        bEnableAnimationCulling = true;
        bEnablePhysicsCulling = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinosaurPerformanceStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 TotalDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 VisibleDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 HighDetailDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 MediumDetailDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 LowDetailDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 CulledDinosaurs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float DinosaurRenderCost = 0.0f;

    FPerf_DinosaurPerformanceStats()
    {
        TotalDinosaurs = 0;
        VisibleDinosaurs = 0;
        HighDetailDinosaurs = 0;
        MediumDetailDinosaurs = 0;
        LowDetailDinosaurs = 0;
        CulledDinosaurs = 0;
        AverageFrameTime = 0.0f;
        DinosaurRenderCost = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_DinosaurLODLevel : uint8
{
    HighDetail      UMETA(DisplayName = "High Detail"),
    MediumDetail    UMETA(DisplayName = "Medium Detail"),
    LowDetail       UMETA(DisplayName = "Low Detail"),
    Culled          UMETA(DisplayName = "Culled")
};

/**
 * Performance manager specifically for dinosaur actors
 * Handles LOD, culling, and optimization for large numbers of dinosaurs
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_DinosaurPerformanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_DinosaurPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance")
    FPerf_DinosaurLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance")
    bool bEnableDynamicLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance")
    bool bEnablePerformanceLogging = false;

    // Performance stats
    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    FPerf_DinosaurPerformanceStats CurrentStats;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void UpdateDinosaurLOD();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void RegisterDinosaur(APawn* DinosaurPawn);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void UnregisterDinosaur(APawn* DinosaurPawn);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    EPerf_DinosaurLODLevel GetDinosaurLODLevel(APawn* DinosaurPawn) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void SetDinosaurLODLevel(APawn* DinosaurPawn, EPerf_DinosaurLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void OptimizeDinosaurRendering();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void CullDistantDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    FPerf_DinosaurPerformanceStats GetPerformanceStats() const { return CurrentStats; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void SetMaxVisibleDinosaurs(int32 MaxCount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void EnableAnimationCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Performance")
    void EnablePhysicsCulling(bool bEnable);

private:
    // Internal tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<APawn>> RegisteredDinosaurs;

    UPROPERTY()
    TMap<TWeakObjectPtr<APawn>, EPerf_DinosaurLODLevel> DinosaurLODLevels;

    float TimeSinceLastUpdate = 0.0f;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;

    // Helper functions
    void UpdatePerformanceStats();
    void ApplyLODToDinosaur(APawn* DinosaurPawn, EPerf_DinosaurLODLevel LODLevel);
    float CalculateDistanceToPlayer(APawn* DinosaurPawn) const;
    void LogPerformanceData();
    void CleanupInvalidReferences();
    void SortDinosaursByDistance();
    void ApplyVisibilityLimits();
};