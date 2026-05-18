#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
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
struct FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly)
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly)
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
struct FBuild_BiomePopulationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly)
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly)
    int32 RequiredActors;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentActors;

    UPROPERTY(BlueprintReadOnly)
    bool bIsPopulated;

    FBuild_BiomePopulationData()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        RequiredActors = 500;
        CurrentActors = 0;
        bIsPopulated = false;
    }
};

/**
 * Final Integration Orchestrator - Manages complete system validation and build integration
 * Coordinates all subsystems and ensures stable build state
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

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StartFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateDinosaurSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemValidationResult GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_BiomePopulationData> GetBiomePopulationStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsSystemStable();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateIntegrationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, FBuild_SystemValidationResult> SystemValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_BiomePopulationData> BiomeData;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime;

private:
    void InitializeBiomeData();
    void ValidateActorsInBiome(const FString& BiomeName, const FVector& Center, float Radius);
    void LogValidationResult(const FString& SystemName, EBuild_IntegrationStatus Status, const FString& Message);
};