// PerformanceManager.h
// Transpersonal Game Studio — Performance Optimizer Agent #4
// PROD_CYCLE_AUTO_20260628_009
// Manages runtime performance: LOD culling, tick throttling, scalability

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PerformanceManager.generated.h"

// ─── Enums (global scope — UHT requirement) ───────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Low     UMETA(DisplayName = "Low (Console Min)"),
    Medium  UMETA(DisplayName = "Medium (Console Target)"),
    High    UMETA(DisplayName = "High (PC Target)"),
    Ultra   UMETA(DisplayName = "Ultra (PC High-end)")
};

UENUM(BlueprintType)
enum class EPerf_TickLODLevel : uint8
{
    Full        UMETA(DisplayName = "Full Tick (< 200m)"),
    Reduced     UMETA(DisplayName = "Reduced Tick (200-500m)"),
    Minimal     UMETA(DisplayName = "Minimal Tick (500-1000m)"),
    Disabled    UMETA(DisplayName = "Disabled (> 1000m)")
};

// ─── Structs (global scope — UHT requirement) ─────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameBudget_ms = 16.67f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SurvivalSystem_ms = 2.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AISystem_ms = 2.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsSystem_ms = 1.67f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderingBudget_ms = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime_ms = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bOverBudget = false;
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    // Distance at which survival drain is disabled for non-player characters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Survival")
    float SurvivalDrain_DisableDistance = 500.0f;

    // Distance at which ALL survival ticks are disabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Survival")
    float SurvivalAll_DisableDistance = 1000.0f;

    // Distance at which AI behavior trees are simplified
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|AI")
    float AISimplify_Distance = 300.0f;

    // Distance at which AI is fully disabled (only position updated)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|AI")
    float AIDisable_Distance = 800.0f;

    // Distance at which physics simulation is disabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD|Physics")
    float PhysicsDisable_Distance = 400.0f;
};

// ─── Main Class ───────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API APerf_PerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Frame Budget ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget PCBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget ConsoleBudget;

    // ─── LOD Settings ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings LODSettings;

    // ─── Quality Preset ───────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality")
    EPerf_QualityPreset ActivePreset = EPerf_QualityPreset::High;

    // ─── Runtime Stats ────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    float AverageFrameTime_ms = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    int32 ActiveTickingActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    bool bIsOverBudget = false;

    // ─── Public API ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityPreset(EPerf_QualityPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_TickLODLevel GetTickLODForDistance(float DistanceFromPlayer) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetActiveBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const { return CurrentFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsOverBudget() const { return bIsOverBudget; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance|Debug")
    void PrintPerformanceReport() const;

    // ─── Singleton Access ─────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance", meta = (WorldContext = "WorldContextObject"))
    static APerf_PerformanceManager* GetInstance(UObject* WorldContextObject);

private:
    // Frame time accumulator for rolling average
    TArray<float> FrameTimeHistory;
    static const int32 FrameHistorySize = 60;

    // Singleton instance
    static TWeakObjectPtr<APerf_PerformanceManager> Instance;

    void UpdateFrameStats(float DeltaTime);
    void CheckBudgetOverrun();
    void ApplyConsoleCommands(const TArray<FString>& Commands);
};
