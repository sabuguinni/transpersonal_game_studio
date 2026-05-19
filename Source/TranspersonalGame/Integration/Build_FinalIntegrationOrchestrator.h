#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Timeout     UMETA(DisplayName = "Timeout")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationTime;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Pending;
        ErrorMessage = TEXT("");
        ActorCount = 0;
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
    int32 VegetationCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMeetsPopulationTarget;

    FBuild_BiomePopulationData()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        VegetationCount = 0;
        bMeetsPopulationTarget = false;
    }
};

/**
 * Final Integration Orchestrator - Agent #19
 * Validates all systems, checks biome population, ensures build stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void CheckBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateBridgeHealth();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void GenerateIntegrationReport();

    // System Validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemValidationResult> GetAllSystemResults() const { return SystemValidationResults; }

    // Biome Validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_BiomePopulationData> GetBiomePopulationData() const { return BiomePopulationData; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool DoesBiomeMeetTarget(const FString& BiomeName, int32 MinActors = 500) const;

    // Status Getters
    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetOverallStatus() const { return OverallIntegrationStatus; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetLastErrorMessage() const { return LastErrorMessage; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount() const { return TotalActorCount; }

protected:
    // Internal validation functions
    void ValidateWorldGeneration();
    void ValidateCharacterSystems();
    void ValidateDinosaurAI();
    void ValidateEnvironmentSystems();
    void ValidateAudioSystems();
    void ValidateVFXSystems();

    // Biome validation helpers
    void CountActorsInBiome(const FString& BiomeName, const FVector& Center, float Radius = 25000.0f);
    int32 CountDinosaursInRadius(const FVector& Center, float Radius) const;
    int32 CountVegetationInRadius(const FVector& Center, float Radius) const;

private:
    UPROPERTY()
    TArray<FBuild_SystemValidationResult> SystemValidationResults;

    UPROPERTY()
    TArray<FBuild_BiomePopulationData> BiomePopulationData;

    UPROPERTY()
    EBuild_IntegrationStatus OverallIntegrationStatus;

    UPROPERTY()
    FString LastErrorMessage;

    UPROPERTY()
    int32 TotalActorCount;

    UPROPERTY()
    float LastValidationTime;

    // Biome coordinates from memory
    static const TMap<FString, FVector> BiomeCoordinates;
};