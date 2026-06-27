// PerformanceOptimizer.h
// Agent #4 — Performance Optimizer
// Guarantees 60fps PC / 30fps console via dynamic LOD, culling, and budget enforcement

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PerformanceOptimizer.generated.h"

// Forward declarations
class UStaticMeshComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid-Range)"),
    Medium      UMETA(DisplayName = "Medium (PC Low / Console)"),
    Low         UMETA(DisplayName = "Low (Performance Mode)")
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target frame time in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f; // 60fps

    // GPU budget (ms) — rendering, shadows, reflections
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMs = 10.0f;

    // CPU budget (ms) — AI, physics, game logic
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUBudgetMs = 5.0f;

    // Shadow budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ShadowBudgetMs = 3.0f;

    // Max dynamic lights in scene
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDynamicLights = 8;

    // Max visible dinosaurs (full LOD)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleDinosaurs = 12;
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    // Distance at which LOD0 transitions to LOD1 (meters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 15.0f;

    // Distance at which LOD1 transitions to LOD2
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 40.0f;

    // Distance at which LOD2 transitions to LOD3 (lowest)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 80.0f;

    // Cull distance for small props (rocks, debris)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float SmallPropCullDistance = 50.0f;

    // Cull distance for foliage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageCullDistance = 120.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    // Current quality tier — set at startup based on hardware
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    // Frame budget configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_FrameBudget FrameBudget;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODSettings LODSettings;

    // Current measured FPS (updated every second)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    // Current GPU frame time in ms
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentGPUTimeMs = 0.0f;

    // Whether dynamic quality scaling is active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bDynamicQualityScaling = true;

    // Minimum FPS before quality is reduced
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFPSThreshold = 45.0f;

    // FPS above which quality can be increased
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFPSThreshold = 70.0f;

    // Apply current quality tier settings to the engine
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyQualitySettings();

    // Force a specific quality tier
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier NewTier);

    // Get recommended quality tier based on current hardware
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_QualityTier GetRecommendedQualityTier() const;

    // Apply LOD bias to a static mesh component
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeStaticMesh(UStaticMeshComponent* MeshComp, float ImportanceScale = 1.0f);

    // Apply LOD bias to a skeletal mesh (dinosaur, character)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSkeletalMesh(USkeletalMeshComponent* MeshComp, float ImportanceScale = 1.0f);

    // Run full scene performance audit — logs results
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceAudit();

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Accumulated time for FPS measurement
    float FPSAccumulator = 0.0f;
    int32 FPSFrameCount = 0;

    // Time since last quality adjustment
    float TimeSinceLastQualityAdjust = 0.0f;

    // Minimum seconds between quality adjustments (prevent thrashing)
    static constexpr float QualityAdjustCooldown = 5.0f;

    // Apply console variable settings for a given tier
    void ApplyConsoleVariables(EPerf_QualityTier Tier);

    // Check if FPS is below threshold and reduce quality if needed
    void UpdateDynamicQuality(float DeltaTime);
};
