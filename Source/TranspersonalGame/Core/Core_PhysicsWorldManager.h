#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "Core_PhysicsWorldManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsWorldType : uint8
{
    Standard        UMETA(DisplayName = "Standard Physics"),
    HighPrecision   UMETA(DisplayName = "High Precision"),
    LowLatency      UMETA(DisplayName = "Low Latency"),
    Destructible    UMETA(DisplayName = "Destructible Objects"),
    FluidDynamics   UMETA(DisplayName = "Fluid Dynamics"),
    VehiclePhysics  UMETA(DisplayName = "Vehicle Physics")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsWorldConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    ECore_PhysicsWorldType WorldType = ECore_PhysicsWorldType::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float PhysicsTimeStep = 0.016667f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    int32 MaxSubSteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    bool bEnableAsyncPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    bool bEnableCCD = false; // Continuous Collision Detection

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics World")
    float AngularDamping = 0.05f;

    FCore_PhysicsWorldConfig()
    {
        WorldType = ECore_PhysicsWorldType::Standard;
        GravityScale = 1.0f;
        PhysicsTimeStep = 0.016667f;
        MaxSubSteps = 6;
        bEnableAsyncPhysics = true;
        bEnableCCD = false;
        LinearDamping = 0.01f;
        AngularDamping = 0.05f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SleepingRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionPairs = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceOptimal = true;

    FCore_PhysicsPerformanceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActiveRigidBodies = 0;
        SleepingRigidBodies = 0;
        CollisionPairs = 0;
        MemoryUsageMB = 0.0f;
        bIsPerformanceOptimal = true;
    }
};

/**
 * Core Physics World Manager - Manages physics world configuration and performance
 * Handles physics world setup, optimization, and monitoring for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsWorldManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsWorldManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics World Configuration
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void ConfigurePhysicsWorld(const FCore_PhysicsWorldConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetPhysicsWorldType(ECore_PhysicsWorldType NewType);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    FCore_PhysicsWorldConfig GetCurrentConfig() const { return CurrentConfig; }

    UFUNCTION(BlueprintPure, Category = "Physics World")
    ECore_PhysicsWorldType GetCurrentWorldType() const { return CurrentConfig.WorldType; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FCore_PhysicsPerformanceMetrics GetPerformanceMetrics() const { return PerformanceMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceOptimal() const { return PerformanceMetrics.bIsPerformanceOptimal; }

    // Physics World Management
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void ResetPhysicsWorld();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void PausePhysicsSimulation(bool bPause);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetGravityScale(float NewGravityScale);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    float GetGravityScale() const { return CurrentConfig.GravityScale; }

    // Advanced Physics Features
    UFUNCTION(BlueprintCallable, Category = "Advanced Physics")
    void EnableContinuousCollisionDetection(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Advanced Physics")
    void SetPhysicsTimeStep(float NewTimeStep);

    UFUNCTION(BlueprintCallable, Category = "Advanced Physics")
    void SetMaxSubSteps(int32 NewMaxSubSteps);

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidatePhysicsWorldSetup();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPhysicsWorldStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawPhysicsWorld(bool bEnable);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCore_PhysicsWorldConfig CurrentConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_PhysicsPerformanceMetrics PerformanceMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoOptimizePerformance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PerformanceUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxAllowedPhysicsFrameTime = 0.033f; // 30 FPS threshold

private:
    // Internal state
    FTimerHandle PerformanceUpdateTimer;
    bool bIsPhysicsWorldInitialized = false;
    bool bIsPhysicsPaused = false;

    // Internal methods
    void InitializePhysicsWorld();
    void ApplyPhysicsConfiguration();
    void StartPerformanceMonitoring();
    void StopPerformanceMonitoring();
    void OnPerformanceUpdate();
    void HandlePerformanceIssues();
    UWorld* GetPhysicsWorld() const;
};