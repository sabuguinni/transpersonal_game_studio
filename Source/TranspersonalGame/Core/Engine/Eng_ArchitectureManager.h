#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "../../SharedTypes.h"
#include "Eng_ArchitectureManager.generated.h"

/**
 * Core Architecture Manager - Centralized system for managing game architecture
 * Handles module initialization, system validation, and cross-system communication
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool InitializeGameSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterGameModule(const FString& ModuleName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemReady(const FString& SystemName) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSystemPerformanceMetric(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(const FString& SystemName, float TargetFPS);

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Modules")
    TArray<FString> GetLoadedModules() const;

    UFUNCTION(BlueprintCallable, Category = "Modules")
    bool UnloadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Modules")
    bool ReloadModule(const FString& ModuleName);

protected:
    // System State Management
    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<FString, bool> SystemReadyStates;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<FString, float> PerformanceMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FString> LoadedModuleNames;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bAutoInitializeSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float SystemInitializationTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bEnablePerformanceMonitoring;

private:
    // Internal system management
    void InitializeWorldGeneration();
    void InitializeCharacterSystems();
    void InitializeCombatSystems();
    void InitializeAISystems();
    void InitializeAudioSystems();
    void InitializeVFXSystems();

    // Performance tracking
    void UpdatePerformanceMetrics();
    bool ValidateSystemPerformance(const FString& SystemName) const;

    // Module lifecycle
    bool LoadGameModule(const FString& ModuleName);
    void OnModuleLoaded(const FString& ModuleName);
    void OnModuleUnloaded(const FString& ModuleName);

    // Internal state
    bool bSystemsInitialized;
    float LastPerformanceUpdate;
    TMap<FString, int32> ModulePriorities;
};