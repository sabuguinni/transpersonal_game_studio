#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD0_Ultra = 0,     // Full detail - within 500 units
    LOD1_High = 1,      // High detail - 500-1500 units
    LOD2_Medium = 2,    // Medium detail - 1500-3000 units
    LOD3_Low = 3,       // Low detail - 3000-6000 units
    LOD4_Culled = 4     // Culled - beyond 6000 units
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD0Distance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD1Distance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD2Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD3Distance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableFrustumCulling = true;

    FPerf_LODSettings()
    {
        LOD0Distance = 500.0f;
        LOD1Distance = 1500.0f;
        LOD2Distance = 3000.0f;
        LOD3Distance = 6000.0f;
        CullDistance = 10000.0f;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
    }
};

USTRUCT(BlueprintType)
struct FPerf_LODStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 LOD0Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 LOD1Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 LOD2Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 LOD3Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 CulledCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    int32 TotalManagedObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    float AverageDistance = 0.0f;

    FPerf_LODStats()
    {
        LOD0Count = 0;
        LOD1Count = 0;
        LOD2Count = 0;
        LOD3Count = 0;
        CulledCount = 0;
        TotalManagedObjects = 0;
        AverageDistance = 0.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_LODManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    float UpdateFrequency = 0.1f; // Update every 100ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bDebugDrawLOD = false;

    // Runtime Stats
    UPROPERTY(BlueprintReadOnly, Category = "LOD Stats")
    FPerf_LODStats CurrentStats;

    // LOD Management Functions
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterActorForLOD(AActor* Actor, float CustomLODScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterActorFromLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    EPerf_LODLevel GetActorLODLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    float GetDistanceToPlayer(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateAllLODs();

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_LODStats GetLODStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetLODStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTotalManagedActors() const;

    // Culling Functions
    UFUNCTION(BlueprintCallable, Category = "Culling")
    void EnableDistanceCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void EnableFrustumCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    bool IsActorInViewFrustum(AActor* Actor) const;

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ToggleDebugDraw();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintLODStats();

private:
    // Internal data structures
    UPROPERTY()
    TMap<AActor*, float> ManagedActors; // Actor -> LOD Scale multiplier

    UPROPERTY()
    TMap<AActor*, EPerf_LODLevel> ActorLODLevels;

    float LastUpdateTime;
    APawn* PlayerPawn;

    // Internal helper functions
    void UpdateActorLOD(AActor* Actor, float LODScale);
    EPerf_LODLevel CalculateLODLevel(float Distance, float LODScale) const;
    void ApplyLODToActor(AActor* Actor, EPerf_LODLevel LODLevel);
    void UpdateStats();
    void DrawDebugLOD();
    APawn* GetPlayerPawn() const;
};