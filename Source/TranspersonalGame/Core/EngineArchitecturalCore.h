#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EngineArchitecturalCore.generated.h"

/**
 * Engine Architectural Core - Central coordination system for all engine-level architecture
 * Ensures compliance with architectural standards across all game systems
 * Manages module dependencies, performance constraints, and system integration
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

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeArchitecturalSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateSystemCompliance();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemModule(const FString& ModuleName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystemModule(const FString& ModuleName);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(float TargetFPS, float MaxMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CheckPerformanceCompliance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSystemLoad();

    // Module Integration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ResolveModuleConflicts();

    // Biome System Integration
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void InitializeBiomeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void ValidateBiomeSystemIntegration();

protected:
    // System Registry
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, int32> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> SystemLoadOrder;

    // Performance Constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB = 8192.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentFrameRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentMemoryUsageMB = 0.0f;

    // Architecture State
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bSystemsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bComplianceValidated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 ActiveSystemCount = 0;

private:
    // Internal validation methods
    void ValidatePhysicsIntegration();
    void ValidateWorldGenerationIntegration();
    void ValidateCharacterSystemIntegration();
    void ValidateAISystemIntegration();

    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckMemoryUsage();
    void CheckFrameRate();

    // System coordination
    void InitializeSystemLoadOrder();
    void LoadSystemsInOrder();
    void ValidateSystemInterfaces();
};

/**
 * Engine Architecture Configuration
 * Defines architectural standards and constraints for the entire game
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEngineArchitectureConfig
{
    GENERATED_BODY()

    // Performance Targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsage_PC = 8192.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsage_Console = 4096.0f;

    // System Constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
    int32 MaxConcurrentPhysicsObjects = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
    int32 MaxConcurrentAIAgents = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
    int32 MaxWorldPartitionCells = 100;

    // Quality Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 PhysicsLODLevels = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 RenderingLODLevels = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 AudioLODLevels = 3;

    FEngineArchitectureConfig()
    {
        // Default constructor
    }
};