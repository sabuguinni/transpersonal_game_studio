#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitectureCore.generated.h"

/**
 * Core Architecture System - Defines the technical foundation for all game systems
 * This subsystem enforces architectural rules and manages system dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureCore();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterSystemModule(const FString& ModuleName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemModuleActive(const FString& ModuleName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport() const;

    // System State Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetSystemEnabled(const FString& SystemName, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemEnabled(const FString& SystemName) const;

protected:
    // Registered system modules with priorities
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, int32> RegisteredModules;

    // System enable/disable states
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, bool> SystemStates;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LastActorCount;

    // Architecture validation
    void ValidateModuleDependencies();
    void InitializeCoreSystems();
    void SetupPerformanceMonitoring();

private:
    // Internal state tracking
    bool bIsInitialized;
    FDateTime LastValidationTime;
};