#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

/**
 * Build Integration Manager - Handles cross-system integration and build validation
 * Ensures all agent outputs work together cohesively
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateActorIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidatePerformanceMetrics();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool SaveCurrentBuildState();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GenerateBuildReport();

    // Cross-system testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestDinosaurSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestEnvironmentSystems();

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float AverageComponentsPerActor;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bSystemsHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString LastBuildStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime LastValidationTime;

private:
    // Internal validation helpers
    bool ValidateModuleCompilation();
    bool ValidateClassLoading();
    bool ValidateActorSpawning();
    
    // Performance tracking
    void UpdatePerformanceMetrics();
    void CheckMemoryUsage();
    
    // Build state management
    void CacheBuildState();
    bool CompareBuildStates();

    // Internal state
    TMap<FString, int32> ActorTypeDistribution;
    TArray<FString> ValidationErrors;
    TArray<FString> ValidationWarnings;
    
    bool bInitialized;
    float LastMemoryUsage;
    int32 LastActorCount;
};