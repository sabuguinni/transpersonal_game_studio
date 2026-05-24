#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Eng_UnifiedArchitectureManager.generated.h"

class UCore_PhysicsManager;
class UBiomeManager;
class ADir_ProductionManager;
class UCore_PhysicsIntegrator;

/**
 * UNIFIED ARCHITECTURE MANAGER - Engine Architect Core System
 * 
 * This is the central architectural coordinator that manages all major game systems
 * according to the technical architecture defined by Agent #02 (Engine Architect).
 * 
 * ARCHITECTURAL PRINCIPLES:
 * 1. Single Point of Truth - All system initialization goes through this manager
 * 2. Dependency Resolution - Systems are initialized in correct dependency order
 * 3. Performance Monitoring - All systems report metrics through this hub
 * 4. Graceful Degradation - System failures are contained and reported
 * 5. Hot-Reload Support - Systems can be restarted without full game restart
 * 
 * SYSTEM INITIALIZATION ORDER (CRITICAL - DO NOT CHANGE):
 * 1. Core Physics Systems (PhysicsManager, PhysicsIntegrator)
 * 2. World Generation (BiomeManager, TerrainPhysics)
 * 3. Character Systems (Movement, Animation)
 * 4. AI Systems (Dinosaur AI, NPC Behavior)
 * 5. Production Systems (ProductionManager, AssetManager)
 * 6. Performance Monitoring (PerformanceMonitor, Debugger)
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_UnifiedArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_UnifiedArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RestartFailedSystems();

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterPhysicsSystem(UCore_PhysicsManager* PhysicsManager);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterBiomeSystem(UBiomeManager* BiomeManager);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterProductionSystem(ADir_ProductionManager* ProductionManager);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_SystemPerformanceReport GetSystemPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetOverallSystemHealth();

    // Debug and Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void RunArchitectureValidation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateSystemDependencyReport();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetFailedSystems();

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnSystemInitialized(const FString& SystemName);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnSystemFailed(const FString& SystemName, const FString& ErrorMessage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnArchitectureValidationComplete(bool bSuccess);

protected:
    // Core System References
    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsManager> PhysicsManager;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UBiomeManager> BiomeManager;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ADir_ProductionManager> ProductionManager;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsIntegrator> PhysicsIntegrator;

    // System State Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, bool> SystemInitializationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> FailedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float LastValidationTime;

    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FEng_SystemPerformanceReport CurrentPerformanceReport;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureValid;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float SystemHealthCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoRestartFailedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnablePerformanceMonitoring;

private:
    // Internal System Management
    void InitializePhysicsSystems();
    void InitializeWorldSystems();
    void InitializeCharacterSystems();
    void InitializeAISystems();
    void InitializeProductionSystems();
    void InitializePerformanceSystems();

    // Validation Helpers
    bool ValidatePhysicsIntegration();
    bool ValidateBiomeIntegration();
    bool ValidateProductionIntegration();
    bool ValidatePerformanceMetrics();

    // Error Handling
    void HandleSystemFailure(const FString& SystemName, const FString& ErrorMessage);
    void LogArchitectureStatus();

    // Performance Monitoring
    void UpdatePerformanceMetrics();
    void CheckSystemHealth();

    FTimerHandle HealthCheckTimer;
    FTimerHandle PerformanceUpdateTimer;
};

/**
 * WORLD SUBSYSTEM COMPONENT - Per-Level Architecture Management
 * 
 * Manages architecture systems that are specific to individual levels/worlds.
 * Works in conjunction with the main UEng_UnifiedArchitectureManager.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_WorldArchitectureSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void InitializeWorldSpecificSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateWorldArchitecture();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "World Architecture")
    bool bWorldSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "World Architecture")
    TObjectPtr<UEng_UnifiedArchitectureManager> MainArchitectureManager;
};