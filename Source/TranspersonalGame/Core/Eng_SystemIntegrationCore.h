#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemIntegrationCore.generated.h"

class UEng_ArchitectureCore;
class UCore_PhysicsManager;
class UBiomeManager;
class ADinosaurBase;

/**
 * Central system integration coordinator for all major game systems.
 * Ensures proper initialization order and cross-system communication.
 * This is the master coordinator that Agent #02 (Engine Architect) uses
 * to orchestrate the entire technical architecture.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemIntegrationCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemIntegrationCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Initialize all core systems in proper dependency order */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void InitializeAllSystems();

    /** Validate that all systems are properly integrated */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool ValidateSystemIntegration();

    /** Get system initialization status */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    FString GetSystemStatus() const;

    /** Force reinitialize a specific system */
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool ReinitializeSystem(EEng_SystemType SystemType);

protected:
    /** Architecture core system reference */
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TObjectPtr<UEng_ArchitectureCore> ArchitectureCore;

    /** Physics management system */
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TObjectPtr<UCore_PhysicsManager> PhysicsManager;

    /** World generation and biome system */
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TObjectPtr<UBiomeManager> BiomeManager;

    /** System initialization status tracking */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TMap<EEng_SystemType, bool> SystemInitializationStatus;

    /** System dependency graph */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<EEng_SystemType, TArray<EEng_SystemType>> SystemDependencies;

private:
    /** Initialize system dependencies mapping */
    void InitializeSystemDependencies();

    /** Initialize architecture core */
    bool InitializeArchitectureCore();

    /** Initialize physics systems */
    bool InitializePhysicsCore();

    /** Initialize world generation systems */
    bool InitializeWorldGeneration();

    /** Initialize character and AI systems */
    bool InitializeCharacterSystems();

    /** Validate system dependency chain */
    bool ValidateDependencyChain(EEng_SystemType SystemType);

    /** Check if all dependencies are satisfied */
    bool AreDependenciesSatisfied(EEng_SystemType SystemType);

    /** Log system status for debugging */
    void LogSystemStatus() const;

    /** Current initialization phase */
    EEng_InitializationPhase CurrentPhase;

    /** Initialization start time for performance tracking */
    double InitializationStartTime;

    /** Track failed system initializations */
    TArray<EEng_SystemType> FailedSystems;
};