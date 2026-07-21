#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "EngineArchitect.generated.h"

/**
 * Engine Architect System - Core architectural validation and compliance
 * Ensures all game systems follow architectural standards and integration rules
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitect();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core architectural validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateModuleCompliance();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidatePerformanceStandards();

    // System registration and management
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    void RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<FString> GetRegisteredSystems() const;

    // Compilation and integration checks
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    void RunArchitecturalComplianceCheck();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    void ValidateAllModuleDependencies();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architect")
    void CheckSystemIntegration();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    float GetSystemPerformanceScore() const { return SystemPerformanceScore; }

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<FString> GetPerformanceWarnings() const { return PerformanceWarnings; }

protected:
    // Core system registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    TMap<FString, UObject*> RegisteredSystems;

    // Performance metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    float SystemPerformanceScore = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    TArray<FString> PerformanceWarnings;

    // Validation flags
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    bool bArchitectureValid = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    bool bModulesCompliant = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    bool bPerformanceAcceptable = true;

private:
    // Internal validation methods
    bool ValidatePhysicsIntegration();
    bool ValidateBiomeSystemIntegration();
    bool ValidateCharacterSystemIntegration();
    bool ValidateAISystemIntegration();
    
    // Performance analysis
    void AnalyzeSystemPerformance();
    void CheckMemoryUsage();
    void ValidateFrameRate();
    
    // Compliance checking
    void CheckModuleDependencies();
    void ValidateClassHierarchy();
    void CheckSharedTypesUsage();
    
    // Logging and reporting
    void LogArchitecturalIssue(const FString& Issue, const FString& System);
    void GenerateComplianceReport();
};

/**
 * Engine Architect World Subsystem - Per-world architectural management
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngineArchitectWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectWorldSubsystem();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // World-specific validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateWorldConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    bool ValidateActorIntegration();

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    void CheckWorldPerformance();

    // Actor registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    void RegisterCriticalActor(AActor* Actor, const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architect")
    TArray<AActor*> GetCriticalActors(const FString& SystemName) const;

protected:
    // World-specific data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    TMap<FString, TArray<AActor*>> CriticalActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    float WorldPerformanceScore = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Engine Architect")
    bool bWorldConfigurationValid = true;

private:
    void ValidateGameModeSetup();
    void CheckPlayerControllerConfiguration();
    void ValidateLevelStreamingSetup();
    void AnalyzeActorDensity();
};