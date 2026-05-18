#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending,
    InProgress,
    Success,
    Failed,
    Timeout
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
    TArray<FString> AssetTypes;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMeetsMinimumRequirement;

    FBuild_BiomePopulationData()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        bMeetsMinimumRequirement = false;
    }
};

/**
 * Final Integration Orchestrator - Agent #19
 * Validates all systems, manages build integration, and ensures game stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateBiomePopulations();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemValidationResult> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_BiomePopulationData> GetBiomePopulations() const { return BiomePopulations; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetOverallStatus() const { return OverallStatus; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetIntegrationReport() const;

protected:
    // Validation Results Storage
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_BiomePopulationData> BiomePopulations;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CriticalSystemsLoaded;

private:
    // Internal validation methods
    FBuild_SystemValidationResult ValidateSystem(const FString& SystemName, UClass* SystemClass);
    void ValidateBiome(const FString& BiomeName, const FVector& BiomeCenter);
    bool CheckModuleRegistration(const FString& ModuleName);
    void GenerateIntegrationReport();

    // Critical system classes to validate
    TArray<FString> CriticalSystems;
    
    // Biome definitions
    TMap<FString, FVector> BiomeDefinitions;
};