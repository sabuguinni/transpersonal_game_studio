#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EngineArchitecturalCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitecture, Log, All);

/**
 * Engine Architectural Core Subsystem
 * Central authority for all architectural decisions, module coordination, and system integration
 * Enforces architectural standards across all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitecturalCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitecturalCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core architectural management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterModule(const FString& ModuleName, const FString& ModuleVersion);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforcePerformanceConstraints();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RunArchitecturalCompliance();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB() const;

    // Module coordination
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    TArray<FString> GetRegisteredModules() const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    void ReloadModule(const FString& ModuleName);

    // System integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void LogSystemStatus();

    // Architectural constraints enforcement
    UFUNCTION(BlueprintCallable, Category = "Constraints")
    bool EnforceWorldPartitionLimits();

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    bool CheckMemoryConstraints();

protected:
    // Registered modules tracking
    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    TMap<FString, FString> RegisteredModules;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxActorsPerLevel = 50000;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxMemoryUsageMB = 8192.0f;

    // Architectural validation flags
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bArchitecturalComplianceEnabled = true;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bPerformanceMonitoringEnabled = true;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bModuleDependencyCheckEnabled = true;

private:
    // Internal validation methods
    bool ValidatePhysicsIntegration();
    bool ValidateWorldGeneration();
    bool ValidateCharacterSystems();
    bool ValidateBiomeManagement();
    bool ValidateAudioSystems();
    bool ValidateVFXSystems();
    bool ValidateNPCBehavior();
    bool ValidateCombatSystems();
    bool ValidateQuestSystems();
    bool ValidateNarrativeSystems();

    // Performance tracking
    float LastFrameTime = 0.0f;
    int32 LastActorCount = 0;
    float LastMemoryUsage = 0.0f;

    // Module dependency validation
    TMap<FString, TArray<FString>> ModuleDependencies;
    
    // System status tracking
    TMap<FString, bool> SystemStatusMap;
};