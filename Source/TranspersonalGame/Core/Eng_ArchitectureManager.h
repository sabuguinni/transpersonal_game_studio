#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitectureManager.generated.h"

/**
 * ENGINE ARCHITECT CORE SYSTEM
 * Manages the technical architecture and enforces design patterns across all systems.
 * This is the central authority for architectural decisions and system coordination.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateMilestone1Requirements();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void EnforceArchitecturalRules();

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    UObject* GetCoreSystem(const FString& SystemName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void MonitorSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetSystemPerformanceMetric(const FString& SystemName);

    // Biome Architecture
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetBiomeCenterLocation(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType);

    // Character System Architecture
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetupDefaultCharacterController();

    // Dinosaur System Architecture
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateDinosaurSystem();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnDinosaurPlaceholders();

    // World Generation Architecture
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateWorldGenerationSystem();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeWorldPartition();

    // Compilation and Build Architecture
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetMissingImplementations();

protected:
    // Core system registry
    UPROPERTY()
    TMap<FString, UObject*> RegisteredSystems;

    // Performance metrics
    UPROPERTY()
    TMap<FString, float> SystemPerformanceMetrics;

    // Architecture validation state
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureValid;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bMilestone1Ready;

    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeConfigurations;

    // System requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    TArray<FString> RequiredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    TArray<FString> OptionalSystems;

private:
    // Internal validation methods
    bool ValidateSystemDependencies();
    bool ValidateMemoryUsage();
    bool ValidateRenderingPipeline();
    void InitializeBiomeConfigurations();
    void SetupPerformanceMonitoring();
    void ValidateActorDistribution();
};