#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Eng_UnifiedArchitectureManager.h"
#include "Core_PhysicsArchitectureIntegrator.generated.h"

class UCore_PhysicsManager;
class UCore_PhysicsIntegrator;
class UCore_CollisionSystem;
class UCore_RagdollSystem;
class UCore_DestructionSystem;
class UCore_TerrainPhysics;

/**
 * Physics Architecture Integrator - Bridges physics systems with Unified Architecture Manager
 * Manages physics system initialization, health monitoring, and performance coordination
 * Ensures proper physics system registration and lifecycle management
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsArchitectureIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsArchitectureIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Physics System References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsManager> PhysicsManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsIntegrator> PhysicsIntegrator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_CollisionSystem> CollisionSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_RagdollSystem> RagdollSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_DestructionSystem> DestructionSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_TerrainPhysics> TerrainPhysics;

    // Architecture Integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UEng_UnifiedArchitectureManager> ArchitectureManager;

    // Physics System Status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Architecture")
    bool bPhysicsSystemsInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Architecture")
    bool bArchitectureIntegrationActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Architecture")
    float PhysicsHealthCheckInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Architecture")
    float LastHealthCheckTime;

    // Performance Monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    float CurrentPhysicsFPS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsSimulationTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PerformanceThreshold;

public:
    // Physics System Management
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool InitializePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void ShutdownPhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool RegisterWithArchitectureManager();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void UnregisterFromArchitectureManager();

    // Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool CheckPhysicsSystemsHealth();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void RestartFailedPhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void UpdatePerformanceMetrics();

    // System Status
    UFUNCTION(BlueprintPure, Category = "Physics Architecture")
    bool ArePhysicsSystemsHealthy() const;

    UFUNCTION(BlueprintPure, Category = "Physics Architecture")
    float GetPhysicsPerformanceScore() const;

    UFUNCTION(BlueprintPure, Category = "Physics Architecture")
    FString GetPhysicsSystemsStatus() const;

    // Architecture Integration Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Architecture")
    void OnPhysicsSystemsInitialized();

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Architecture")
    void OnPhysicsSystemsShutdown();

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Architecture")
    void OnPhysicsPerformanceAlert(float PerformanceScore);

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Architecture")
    void OnPhysicsSystemFailure(const FString& FailedSystemName);

private:
    // Internal System Management
    void CreatePhysicsSystemInstances();
    void ConfigurePhysicsSystemSettings();
    bool ValidatePhysicsSystemIntegrity();
    void HandlePhysicsSystemFailure(const FString& SystemName);
    void LogPhysicsSystemStatus();
};

/**
 * Physics Architecture Subsystem - Game Instance level physics coordination
 * Manages global physics settings and cross-level physics state
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCore_PhysicsArchitectureSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    // Global Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Physics")
    bool bGlobalPhysicsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Physics")
    float GlobalPhysicsScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Physics")
    float GlobalGravityMultiplier;

    // Cross-Level Physics State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics State")
    TMap<FString, float> PhysicsPerformanceHistory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics State")
    int32 TotalPhysicsSystemsRegistered;

public:
    // Global Physics Management
    UFUNCTION(BlueprintCallable, Category = "Global Physics")
    void SetGlobalPhysicsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Global Physics")
    void SetGlobalPhysicsScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Global Physics")
    void SetGlobalGravityMultiplier(float Multiplier);

    // Performance Tracking
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void RecordPhysicsPerformance(const FString& LevelName, float PerformanceScore);

    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    float GetAveragePhysicsPerformance() const;

    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    TArray<FString> GetPhysicsPerformanceReport() const;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Physics Registration")
    void RegisterPhysicsSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Physics Registration")
    void UnregisterPhysicsSystem(const FString& SystemName);

    UFUNCTION(BlueprintPure, Category = "Physics Registration")
    int32 GetRegisteredPhysicsSystemsCount() const { return TotalPhysicsSystemsRegistered; }
};