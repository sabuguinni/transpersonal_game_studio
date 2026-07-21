#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perf_WalkAroundOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_WalkOptimizationLevel : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"), 
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WalkAroundMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MovementTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveCharacters = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MovingCharacters = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60fps

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bMilestoneReady = false;

    FPerf_WalkAroundMetrics()
    {
        MovementTickTime = 0.0f;
        PhysicsTickTime = 0.0f;
        ActiveCharacters = 0;
        MovingCharacters = 0;
        AverageFrameTime = 16.67f;
        TargetFrameTime = 16.67f;
        bMilestoneReady = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WalkAroundSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Optimization")
    float MaxWalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Optimization")
    float MaxAcceleration = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Optimization")
    float BrakingDeceleration = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Optimization")
    float GroundFriction = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MovementTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableMovementCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsLOD = true;

    FPerf_WalkAroundSettings()
    {
        MaxWalkSpeed = 600.0f;
        MaxAcceleration = 2048.0f;
        BrakingDeceleration = 2000.0f;
        GroundFriction = 8.0f;
        PhysicsTickRate = 60.0f;
        MovementTickRate = 60.0f;
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 2000.0f;
        LowDetailDistance = 4000.0f;
        bEnableMovementCulling = true;
        bEnablePhysicsLOD = true;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_WalkAroundOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_WalkAroundOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_WalkOptimizationLevel OptimizationLevel = EPerf_WalkOptimizationLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_WalkAroundSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_WalkAroundMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MetricsUpdateRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bLogPerformanceMetrics = false;

    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeCharacterMovement(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeAllCharacters();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationLevel(EPerf_WalkOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    bool ValidateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODOptimization(ACharacter* Character, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableMovementCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    // Event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptimizationLevelChanged, EPerf_WalkOptimizationLevel, NewLevel);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnOptimizationLevelChanged OnOptimizationLevelChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMilestoneValidated, bool, bIsReady);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMilestoneValidated OnMilestoneValidated;

private:
    float LastMetricsUpdate = 0.0f;
    TArray<float> FrameTimeHistory;
    int32 MaxFrameHistorySize = 60;

    void ApplyOptimizationSettings(ACharacter* Character);
    void UpdateFrameTimeHistory(float DeltaTime);
    float CalculateAverageFrameTime() const;
    void LogPerformanceData() const;
};