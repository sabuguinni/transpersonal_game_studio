#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "IntegrationValidator.generated.h"

/**
 * Integration Validator - Monitors system health and validates cross-module compatibility
 * Used by Integration & Build Agent #19 to ensure all systems work together correctly
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    AIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === VALIDATION FUNCTIONS ===
    
    /** Validate all core systems are loaded and functional */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCoreSystemsLoaded();
    
    /** Check actor count limits and memory usage */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateActorLimits();
    
    /** Verify biome distribution is balanced */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBiomeDistribution();
    
    /** Test cross-system integration scenarios */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCrossSystemIntegration();
    
    /** Run full integration test suite */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunFullIntegrationTest();
    
    /** Generate integration health report */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GenerateHealthReport();

    // === MONITORING PROPERTIES ===
    
    /** Maximum allowed actors in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxActorCount = 8000;
    
    /** Maximum allowed dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxDinosaurCount = 150;
    
    /** Maximum actors per biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxActorsPerBiome = 4000;
    
    /** Enable continuous monitoring */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bEnableContinuousMonitoring = true;
    
    /** Monitoring interval in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MonitoringInterval = 30.0f;
    
    /** Last validation results */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bLastValidationPassed = true;
    
    /** Current actor count */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 CurrentActorCount = 0;
    
    /** Current dinosaur count */
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 CurrentDinosaurCount = 0;

private:
    /** Timer for continuous monitoring */
    float MonitoringTimer = 0.0f;
    
    /** Core system class names to validate */
    TArray<FString> CoreSystemClasses;
    
    /** Biome center coordinates for distribution checking */
    TArray<FVector2D> BiomeCenters;
    
    /** Initialize core system list */
    void InitializeCoreSystemsList();
    
    /** Initialize biome coordinates */
    void InitializeBiomeCoordinates();
    
    /** Count actors in specific biome */
    int32 CountActorsInBiome(const FVector2D& BiomeCenter, float Radius = 20000.0f);
    
    /** Count dinosaur actors */
    int32 CountDinosaurActors();
    
    /** Log validation result */
    void LogValidationResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};