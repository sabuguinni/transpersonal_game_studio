#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Core_PhysicsWorldInterface.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsWorldState : uint8
{
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Paused          UMETA(DisplayName = "Paused"),
    Suspended       UMETA(DisplayName = "Suspended"),
    Error           UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsWorldConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float Gravity = -980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float TimeStep = 0.016667f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    int32 MaxSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    bool bEnableAsyncPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    bool bEnableCCD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float AngularDamping = 0.05f;

    FCore_PhysicsWorldConfig()
    {
        Gravity = -980.0f;
        TimeStep = 0.016667f;
        MaxSubsteps = 6;
        bEnableAsyncPhysics = true;
        bEnableCCD = true;
        LinearDamping = 0.01f;
        AngularDamping = 0.05f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Stats")
    int32 ActiveBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Stats")
    int32 SleepingBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Stats")
    int32 Constraints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Stats")
    float SimulationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Stats")
    float AverageFrameTime = 0.0f;

    FCore_PhysicsStats()
    {
        ActiveBodies = 0;
        SleepingBodies = 0;
        Constraints = 0;
        SimulationTime = 0.0f;
        AverageFrameTime = 0.0f;
    }
};

/**
 * Core Physics World Interface - Provides unified access to physics world functionality
 * Handles physics world initialization, configuration, and state management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsWorldInterface : public UObject
{
    GENERATED_BODY()

public:
    UCore_PhysicsWorldInterface();

    // Core Interface Methods
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    bool InitializePhysicsWorld(UWorld* World, const FCore_PhysicsWorldConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void ShutdownPhysicsWorld();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void UpdatePhysicsWorld(float DeltaTime);

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetPhysicsWorldState(ECore_PhysicsWorldState NewState);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    ECore_PhysicsWorldState GetPhysicsWorldState() const { return CurrentState; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void ApplyPhysicsConfig(const FCore_PhysicsWorldConfig& Config);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    FCore_PhysicsWorldConfig GetPhysicsConfig() const { return WorldConfig; }

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    FCore_PhysicsStats GetPhysicsStats();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void ResetPhysicsStats();

    // Gravity Control
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetGravity(float NewGravity);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    float GetGravity() const;

    // Time Step Control
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetTimeStep(float NewTimeStep);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    float GetTimeStep() const { return WorldConfig.TimeStep; }

    // Body Management
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void WakeAllBodies();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SleepAllBodies();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    int32 GetActiveBodyCount();

    // Collision Detection
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void EnableContinuousCollisionDetection(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    bool IsContinuousCollisionDetectionEnabled() const { return WorldConfig.bEnableCCD; }

    // Debug Functionality
    UFUNCTION(BlueprintCallable, Category = "Physics World", CallInEditor = true)
    void DebugDrawPhysicsWorld();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void LogPhysicsStats();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Physics World")
    ECore_PhysicsWorldState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    FCore_PhysicsWorldConfig WorldConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Physics World")
    FCore_PhysicsStats CachedStats;

    UPROPERTY()
    TWeakObjectPtr<UWorld> ManagedWorld;

    // Internal Methods
    void UpdateStatistics();
    void ApplyGravityToWorld();
    void ConfigurePhysicsSettings();
    bool ValidatePhysicsWorld() const;

private:
    float LastStatsUpdateTime;
    float StatsUpdateInterval;
};