#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitecture.generated.h"

/**
 * Engine Architecture Manager - Central system that enforces architectural rules
 * and manages the technical foundation of the Transpersonal Game Studio project.
 * 
 * CRITICAL RESPONSIBILITIES:
 * - Enforce compilation rules and prevent orphan headers
 * - Manage module dependencies and cross-system communication
 * - Validate biome coordinate usage across all systems
 * - Monitor performance constraints and memory usage
 * - Coordinate between different agent-created systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitecture();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, const FString& AgentOwner);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    // Biome coordinate validation (enforces brain memory rules)
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateBiomeCoordinates(const FVector& Location, EEng_BiomeType ExpectedBiome) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FVector GetValidBiomeLocation(EEng_BiomeType BiomeType) const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void MonitorPerformance();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetCurrentFrameRate() const;

    // Module dependency management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ReportArchitecturalViolation(const FString& ViolationType, const FString& Details);

protected:
    // Registered systems tracking
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, FString> RegisteredSystems;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    // Architecture violations log
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> ArchitecturalViolations;

    // Biome coordinate definitions (from brain memories)
    UPROPERTY(BlueprintReadOnly, Category = "Biomes")
    TMap<EEng_BiomeType, FEng_BiomeCoordinates> BiomeCoordinates;

private:
    void InitializeBiomeCoordinates();
    void ValidateExistingSystems();
    bool CheckForOrphanHeaders();
    void EnforceCompilationRules();
};

/**
 * Architecture Rule Enforcer - Static utility class for compile-time validation
 */
UCLASS(BlueprintType, NotBlueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureRules : public UObject
{
    GENERATED_BODY()

public:
    // Static validation functions
    UFUNCTION(BlueprintCallable, Category = "Architecture Rules", CallInEditor = true)
    static bool ValidateHeaderCppPairs();

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules", CallInEditor = true)
    static void CleanupOrphanHeaders();

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules", CallInEditor = true)
    static bool CheckDuplicateSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules", CallInEditor = true)
    static void EnforceNamingConventions();

    // Compilation enforcement
    UFUNCTION(BlueprintCallable, Category = "Architecture Rules", CallInEditor = true)
    static bool TriggerCompilationCheck();

    UFUNCTION(BlueprintCallable, Category = "Architecture Rules", CallInEditor = true)
    static void ReportCompilationStatus();
};