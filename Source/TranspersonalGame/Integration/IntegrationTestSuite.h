#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "IntegrationTestSuite.generated.h"

/**
 * Integration Test Suite for validating cross-system interactions
 * Monitors system health, performance, and integration points
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AIntegrationTestSuite : public AActor
{
    GENERATED_BODY()

public:
    AIntegrationTestSuite();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Integration Test Results
    UPROPERTY(BlueprintReadOnly, Category = "Integration Tests")
    bool bCharacterGameStateIntegration;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Tests")
    bool bWorldGenFoliageIntegration;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Tests")
    bool bAICombatIntegration;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Tests")
    bool bQuestNarrativeIntegration;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Tests")
    bool bAudioVFXIntegration;

    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 EnvironmentActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    // Biome Distribution
    UPROPERTY(BlueprintReadOnly, Category = "World")
    TMap<FString, int32> BiomeActorCounts;

    // Test Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bRunContinuousTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxActorLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxDinosaurLimit;

public:
    // Integration Test Functions
    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    void RunAllIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    bool TestCharacterGameStateIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    bool TestWorldGenFoliageIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    bool TestAICombatIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    bool TestQuestNarrativeIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    bool TestAudioVFXIntegration();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CheckActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeActorCount();

    // Biome Validation
    UFUNCTION(BlueprintCallable, Category = "World")
    void ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "World")
    void RebalanceBiomeActors();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    bool IsSystemHealthy();

private:
    float LastTestTime;
    bool bTestsInitialized;

    // Helper functions
    void InitializeTests();
    void CountActorsByType();
    void ValidateSystemDependencies();
    void CheckMemoryUsage();
    void MonitorFrameRate();
};