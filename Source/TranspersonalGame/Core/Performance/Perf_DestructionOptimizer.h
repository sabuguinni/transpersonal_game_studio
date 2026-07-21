#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Perf_DestructionOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DestructionLOD
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction LOD")
    float Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction LOD")
    int32 MaxFragments = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction LOD")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction LOD")
    bool bUseSimplifiedPhysics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction LOD")
    bool bDisableCollision = false;

    FPerf_DestructionLOD()
    {
        Distance = 1000.0f;
        MaxFragments = 50;
        FragmentLifetime = 10.0f;
        bUseSimplifiedPhysics = false;
        bDisableCollision = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DestructionStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Stats")
    int32 ActiveFragments = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Stats")
    int32 TotalDestructions = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Stats")
    float AverageFragmentsPerDestruction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Stats")
    float DestructionImpactOnFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Stats")
    int32 FragmentsCleanedThisFrame = 0;

    FPerf_DestructionStats()
    {
        ActiveFragments = 0;
        TotalDestructions = 0;
        AverageFragmentsPerDestruction = 0.0f;
        DestructionImpactOnFPS = 0.0f;
        FragmentsCleanedThisFrame = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_DestructionOptimizer : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_DestructionOptimizer();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { return TStatId(); }

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void SetDestructionLODLevels(const TArray<FPerf_DestructionLOD>& LODLevels);

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    TArray<FPerf_DestructionLOD> GetDestructionLODLevels() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    FPerf_DestructionLOD GetLODForDistance(float Distance) const;

    // Fragment Management
    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void RegisterDestructionFragment(AActor* Fragment, float CreationTime);

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void UnregisterDestructionFragment(AActor* Fragment);

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void CleanupOldFragments();

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void ForceCleanupAllFragments();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    FPerf_DestructionStats GetDestructionStats() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void UpdateDestructionStats();

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    bool ShouldOptimizeDestruction() const;

    // Optimization Controls
    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void SetMaxActiveFragments(int32 MaxFragments);

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    int32 GetMaxActiveFragments() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void SetFragmentCleanupInterval(float IntervalSeconds);

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    float GetFragmentCleanupInterval() const;

    // Emergency Performance Mode
    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void EnableEmergencyMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    bool IsEmergencyModeActive() const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void DebugDrawFragmentInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction Optimization")
    void LogDestructionStats() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Destruction Optimization")
    TArray<FPerf_DestructionLOD> LODLevels;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Optimization")
    FPerf_DestructionStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Optimization")
    bool bEmergencyModeActive;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Optimization")
    int32 MaxActiveFragments;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction Optimization")
    float FragmentCleanupInterval;

    UPROPERTY()
    TMap<AActor*, float> ActiveFragments; // Actor -> Creation Time

    UPROPERTY()
    float LastCleanupTime;

    UPROPERTY()
    float LastStatsUpdateTime;

    UPROPERTY()
    int32 TotalDestructionsCount;

    // Internal Methods
    void InitializeDefaultLODLevels();
    void PerformFragmentCleanup();
    void OptimizeFragmentPerformance();
    void ApplyLODToFragment(AActor* Fragment, const FPerf_DestructionLOD& LOD);
    float GetDistanceToPlayer(const FVector& Location) const;
    void HandleEmergencyMode();
    bool IsFragmentInView(AActor* Fragment) const;

private:
    static const float DefaultCleanupInterval;
    static const int32 DefaultMaxFragments;
    static const int32 EmergencyModeMaxFragments;
};