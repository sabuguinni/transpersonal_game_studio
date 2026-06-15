#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Perf_RagdollOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_RagdollLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxRagdollDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousRagdolls = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BoneConstraintCullDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDistanceBasedLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bCullOffscreenRagdolls = true;

    FPerf_RagdollPerformanceSettings()
    {
        MaxRagdollDistance = 5000.0f;
        MaxSimultaneousRagdolls = 8;
        BoneConstraintCullDistance = 2000.0f;
        PhysicsUpdateRate = 60.0f;
        bUseDistanceBasedLOD = true;
        bCullOffscreenRagdolls = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RagdollInstance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    TWeakObjectPtr<class UCore_RagdollSystem> RagdollComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    EPerf_RagdollLODLevel CurrentLOD = EPerf_RagdollLODLevel::High;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    bool bIsOnScreen = true;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    float LastUpdateTime = 0.0f;

    FPerf_RagdollInstance()
    {
        DistanceToPlayer = 0.0f;
        CurrentLOD = EPerf_RagdollLODLevel::High;
        bIsOnScreen = true;
        LastUpdateTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RagdollOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RagdollOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_RagdollPerformanceSettings PerformanceSettings;

    // LOD Distance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumLODDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowLODDistance = 5000.0f;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    int32 ActiveRagdollCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float RagdollPhysicsCost = 0.0f;

    // Ragdoll Management
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Management")
    TArray<FPerf_RagdollInstance> ActiveRagdolls;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void RegisterRagdoll(class UCore_RagdollSystem* RagdollComponent);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void UnregisterRagdoll(class UCore_RagdollSystem* RagdollComponent);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void OptimizeRagdollLOD(class UCore_RagdollSystem* RagdollComponent, EPerf_RagdollLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    EPerf_RagdollLODLevel CalculateOptimalLOD(float Distance, bool bIsOnScreen);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void UpdateRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void CullDistantRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Performance")
    void EnforceRagdollLimit();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    float GetPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void LogPerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance Settings")
    void SetPerformanceProfile(bool bHighPerformance);

    UFUNCTION(BlueprintCallable, Category = "Performance Settings")
    void ApplyPerformanceSettings(const FPerf_RagdollPerformanceSettings& NewSettings);

private:
    // Internal Functions
    void UpdateRagdollDistances();
    void UpdateRagdollLODs();
    void CheckScreenVisibility();
    void MonitorPerformanceMetrics();
    
    // Performance Tracking
    float LastPerformanceUpdate = 0.0f;
    float PerformanceUpdateInterval = 1.0f;
    TArray<float> FrameTimeHistory;
    int32 FrameTimeHistorySize = 60;
    
    // Player Reference
    UPROPERTY()
    class APawn* PlayerPawn;
};