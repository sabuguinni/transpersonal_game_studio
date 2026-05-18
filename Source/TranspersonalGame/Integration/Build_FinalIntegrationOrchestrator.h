#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Complete    UMETA(DisplayName = "Complete"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationTime;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsValid = false;
        ValidationMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomePopulationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ActorTypes;

    FBuild_BiomePopulationData()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
    }
};

/**
 * Final Integration Orchestrator - Agent #19
 * Manages complete system integration, build validation, and cross-system compatibility
 * Ensures all 18 agent outputs work together as a cohesive game experience
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalIntegrationOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE INTEGRATION PROPERTIES ===

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    EBuild_IntegrationStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    float IntegrationProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    FString LastValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    TArray<FBuild_SystemValidationResult> SystemValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Status")
    TArray<FBuild_BiomePopulationData> BiomePopulationData;

    // === VALIDATION FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    // === BUILD MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void TriggerBuildValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void CheckCrossSystemCompatibility();

    UFUNCTION(BlueprintCallable, Category = "Build Management")
    void ValidatePerformanceMetrics();

    // === UTILITY FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TArray<AActor*> GetActorsInBiome(FVector BiomeCenter, float Radius = 10000.0f);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void LogIntegrationStatus();

private:
    // === INTERNAL VALIDATION ===
    void ValidateSystemInternal(const FString& SystemName, bool bValidationResult, const FString& Message);
    void UpdateBiomeData();
    void CheckActorIntegrity();
    
    // === TIMERS ===
    FTimerHandle ValidationTimerHandle;
    void PerformPeriodicValidation();
    
    // === INTEGRATION STATE ===
    bool bValidationInProgress;
    float ValidationStartTime;
    int32 TotalValidationSteps;
    int32 CompletedValidationSteps;
};