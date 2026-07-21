#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Perf_WalkAroundPerformanceValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WalkAroundMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveCharacters;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleMeshes;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTickTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bMeetsTargetFPS;

    FPerf_WalkAroundMetrics()
        : AverageFrameTime(0.0f)
        , MinFrameTime(0.0f)
        , MaxFrameTime(0.0f)
        , CurrentFPS(0.0f)
        , ActiveCharacters(0)
        , VisibleMeshes(0)
        , PhysicsTickTime(0.0f)
        , RenderThreadTime(0.0f)
        , GameThreadTime(0.0f)
        , bMeetsTargetFPS(false)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WalkAroundRequirements
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    bool bHasThirdPersonCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    bool bHasLandscapeTerrain;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    bool bHasDirectionalLight;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    bool bHasSkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    bool bHasDinosaurMeshes;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    int32 DinosaurMeshCount;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    float MilestoneProgress;

    FPerf_WalkAroundRequirements()
        : bHasThirdPersonCharacter(false)
        , bHasLandscapeTerrain(false)
        , bHasDirectionalLight(false)
        , bHasSkyAtmosphere(false)
        , bHasDinosaurMeshes(false)
        , DinosaurMeshCount(0)
        , MilestoneProgress(0.0f)
    {
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_WalkAroundPerformanceValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_WalkAroundPerformanceValidator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceValidation();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceValidation();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_WalkAroundMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_WalkAroundRequirements GetRequirementStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForWalkAround();

    UFUNCTION(CallInEditor, Category = "Performance")
    void RunPerformanceTest();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPerf_WalkAroundMetrics CurrentMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPerf_WalkAroundRequirements RequirementStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bLogPerformanceWarnings;

private:
    void UpdatePerformanceMetrics();
    void ValidateRequirements();
    void CheckCharacterSystem();
    void CheckTerrainSystem();
    void CheckLightingSystem();
    void CheckDinosaurMeshes();
    void ApplyWalkAroundOptimizations();
    void LogPerformanceData();

    float ValidationTimer;
    TArray<float> FrameTimeHistory;
    bool bIsValidating;
    
    // Performance thresholds
    static constexpr float PC_TARGET_FPS = 60.0f;
    static constexpr float CONSOLE_TARGET_FPS = 30.0f;
    static constexpr float WARNING_FRAME_TIME = 0.020f; // 50 FPS warning
    static constexpr int32 MIN_DINOSAUR_COUNT = 3;
    static constexpr int32 FRAME_HISTORY_SIZE = 60;
};