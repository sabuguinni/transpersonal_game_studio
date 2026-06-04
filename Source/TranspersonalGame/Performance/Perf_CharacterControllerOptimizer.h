#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"
#include "Perf_CharacterControllerOptimizer.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UCore_CharacterController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CharacterControllerMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CameraBoomTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CameraComponentTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MovementComponentTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float ControllerValidationTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float TotalControllerTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActiveCharacterControllers = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 CameraComponentsActive = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 SpringArmComponentsActive = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float AverageFrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MinFrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MaxFrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    bool bMilestonePerformanceTarget = false;

    FPerf_CharacterControllerMetrics()
    {
        CameraBoomTickTime = 0.0f;
        CameraComponentTickTime = 0.0f;
        MovementComponentTickTime = 0.0f;
        ControllerValidationTickTime = 0.0f;
        TotalControllerTickTime = 0.0f;
        ActiveCharacterControllers = 0;
        CameraComponentsActive = 0;
        SpringArmComponentsActive = 0;
        AverageFrameRate = 0.0f;
        MinFrameRate = 0.0f;
        MaxFrameRate = 0.0f;
        bMilestonePerformanceTarget = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CharacterControllerOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float MaxCameraBoomTickTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float MaxCameraComponentTickTime = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float MaxMovementComponentTickTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float MaxControllerValidationTickTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float MaxTotalControllerTickTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    int32 MaxActiveCharacterControllers = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float CameraCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float MovementOptimizationDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float ValidationTickInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float TargetFrameRatePC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    float TargetFrameRateConsole = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    bool bEnableValidationOptimization = true;

    FPerf_CharacterControllerOptimizationSettings()
    {
        MaxCameraBoomTickTime = 0.5f;
        MaxCameraComponentTickTime = 0.3f;
        MaxMovementComponentTickTime = 1.0f;
        MaxControllerValidationTickTime = 0.2f;
        MaxTotalControllerTickTime = 2.0f;
        MaxActiveCharacterControllers = 50;
        CameraCullingDistance = 5000.0f;
        MovementOptimizationDistance = 3000.0f;
        ValidationTickInterval = 0.1f;
        TargetFrameRatePC = 60.0f;
        TargetFrameRateConsole = 30.0f;
        bEnableLODOptimization = true;
        bEnableDistanceCulling = true;
        bEnableValidationOptimization = true;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CharacterControllerOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CharacterControllerOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void AnalyzeCharacterControllerPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void ProfileCameraSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void ProfileMovementSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void ProfileValidationSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    FPerf_CharacterControllerMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    // Optimization Implementation
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeCameraSystem();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeMovementSystem();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeValidationSystem();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyLODOptimization();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyDistanceCulling();

    // Milestone Performance Validation
    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    bool ValidateMilestonePerformance();

    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    bool CheckFrameRateTarget();

    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    bool CheckControllerPerformance();

    UFUNCTION(BlueprintCallable, Category = "Milestone Validation")
    void GeneratePerformanceReport();

    // Integration with Core Systems
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithCharacterController(UCore_CharacterController* Controller);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterCharacterForOptimization(ATranspersonalCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterCharacterFromOptimization(ATranspersonalCharacter* Character);

    // Settings Management
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void UpdateOptimizationSettings(const FPerf_CharacterControllerOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    FPerf_CharacterControllerOptimizationSettings GetOptimizationSettings() const { return OptimizationSettings; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_CharacterControllerMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    FPerf_CharacterControllerOptimizationSettings OptimizationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Registered Controllers")
    TArray<TWeakObjectPtr<UCore_CharacterController>> RegisteredControllers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Registered Characters")
    TArray<TWeakObjectPtr<ATranspersonalCharacter>> RegisteredCharacters;

private:
    // Performance tracking
    float LastAnalysisTime;
    float LastOptimizationTime;
    TArray<float> FrameRateHistory;
    TArray<float> TickTimeHistory;

    // Internal optimization methods
    void UpdatePerformanceMetrics();
    void CalculateFrameRateStats();
    void OptimizeComponentTicking();
    void CullDistantComponents();
    void UpdateLODLevels();
    bool IsCharacterInOptimizationRange(const ATranspersonalCharacter* Character) const;
    float GetDistanceToPlayer(const ATranspersonalCharacter* Character) const;
};