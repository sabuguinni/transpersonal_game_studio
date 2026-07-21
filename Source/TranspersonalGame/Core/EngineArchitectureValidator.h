#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitectureValidator.generated.h"

/**
 * Engine Architecture Validator - Critical System Validation
 * Enforces architectural rules and validates system integrity
 * Agent #2 (Engine Architect) responsibility
 */
UCLASS()
class TRANSPERSONALGAME_API UEngineArchitectureValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureValidator();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateBiomeCoordinates();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void LogSystemStatus();

    // Biome coordinate validation (from brain memories)
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsLocationInValidBiome(FVector Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(FVector Location);

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceTargets();

    // Module validation
    UFUNCTION(BlueprintCallable, Category = "Modules")
    TArray<FString> GetMissingCppFiles();

    UFUNCTION(BlueprintCallable, Category = "Modules")
    TArray<FString> GetOrphanedHeaders();

protected:
    // Biome boundaries (from brain memories)
    UPROPERTY(BlueprintReadOnly, Category = "Biomes")
    TMap<EBiomeType, FBiomeBounds> BiomeBoundaries;

    // System validation flags
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bSystemIntegrityValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bBiomeCoordinatesValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bModuleDependenciesValid;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

private:
    // Internal validation helpers
    void InitializeBiomeBoundaries();
    bool ValidateFileStructure();
    bool ValidateClassHierarchy();
    void LogValidationResults();

    // Validation timers
    FTimerHandle ValidationTimerHandle;
    void PerformPeriodicValidation();
};