#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Eng_GameplayArchitect.generated.h"

// Forward declarations
class UEng_SystemsRegistry;
class UEng_PerformanceMonitor;
class UEng_ModuleValidator;

/**
 * Core gameplay architecture coordinator
 * Manages all game systems, ensures proper initialization order,
 * and maintains system health throughout gameplay
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_GameplayArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_GameplayArchitect();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core architecture methods
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeGameplaySystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterGameplaySystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UObject* GetGameplaySystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemActive(const FString& SystemName);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemPerformanceMetric(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void OptimizeSystemPerformance();

    // Module validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ReportSystemStatus();

protected:
    // Core subsystem references
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    UEng_SystemsRegistry* SystemsRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    UEng_PerformanceMonitor* PerformanceMonitor;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    UEng_ModuleValidator* ModuleValidator;

    // System state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, UObject*> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, bool> SystemActiveStates;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, float> SystemPerformanceMetrics;

    // Architecture configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoInitializeSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bValidateModulesOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PerformanceUpdateInterval;

private:
    // Internal system management
    void InitializeSubsystems();
    void ShutdownSubsystems();
    void UpdateSystemMetrics();
    void ValidateSystemDependencies();

    // Performance tracking
    float LastPerformanceUpdate;
    bool bSystemsInitialized;
    bool bPerformanceMonitoringActive;
};