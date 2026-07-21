#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_CharacterPerformanceOptimizer.generated.h"

class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct FPerf_CharacterPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float MovementCost = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float SurvivalSystemCost = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PhysicsCost = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float AnimationCost = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TotalFrameCost = 0.0f;

    FPerf_CharacterPerformanceData()
    {
        MovementCost = 0.0f;
        SurvivalSystemCost = 0.0f;
        PhysicsCost = 0.0f;
        AnimationCost = 0.0f;
        TotalFrameCost = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_CharacterOptimizationLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CharacterPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CharacterPerformanceOptimizer();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CharacterPerformanceData GetPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCharacterPerformance(EPerf_CharacterOptimizationLevel OptimizationLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinTarget() const;

    // Survival system optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSurvivalSystems(bool bEnableOptimization);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetSurvivalUpdateFrequency(float UpdateFrequency);

    // Movement optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMovementSystem(bool bEnableOptimization);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMovementLOD(float Distance);

    // Physics optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsSystem(bool bEnableOptimization);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPhysicsComplexity(EPerf_CharacterOptimizationLevel Complexity);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceTolerancePercent = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_CharacterOptimizationLevel CurrentOptimizationLevel = EPerf_CharacterOptimizationLevel::High;

    // Survival system settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Performance")
    float SurvivalUpdateFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Performance")
    bool bOptimizeSurvivalSystems = true;

    // Movement settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Performance")
    float MovementLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Performance")
    bool bOptimizeMovement = true;

    // Physics settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bOptimizePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    EPerf_CharacterOptimizationLevel PhysicsComplexity = EPerf_CharacterOptimizationLevel::High;

private:
    UPROPERTY()
    FPerf_CharacterPerformanceData CurrentPerformanceData;

    UPROPERTY()
    ATranspersonalCharacter* OwnerCharacter;

    float LastOptimizationCheck = 0.0f;
    float AccumulatedFrameTime = 0.0f;
    int32 FrameCount = 0;

    // Performance monitoring methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void CheckPerformanceThresholds();
    void ApplyOptimizations();

    // Specific optimization methods
    void OptimizeSurvivalSystemsInternal();
    void OptimizeMovementSystemInternal();
    void OptimizePhysicsSystemInternal();
    void OptimizeAnimationSystemInternal();

    // Utility methods
    float CalculateMovementCost() const;
    float CalculateSurvivalCost() const;
    float CalculatePhysicsCost() const;
    float CalculateAnimationCost() const;
};