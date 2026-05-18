#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationReport.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CharacterActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 StaticMeshAssetCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurPackAssetCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bModuleLoadingSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bLevelSavedSuccessfully = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildStatus = TEXT("Unknown");

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ValidationTimestamp;

    FBuild_IntegrationMetrics()
    {
        ValidationTimestamp = FDateTime::Now();
        BuildStatus = TEXT("Pending");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biomes")
    FString BiomeName = TEXT("");

    UPROPERTY(BlueprintReadOnly, Category = "Biomes")
    FVector BiomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Biomes")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biomes")
    bool bPopulated = false;

    FBuild_BiomeValidation()
    {
        BiomeName = TEXT("Unknown");
        bPopulated = false;
    }

    FBuild_BiomeValidation(const FString& InBiomeName, const FVector& InLocation)
        : BiomeName(InBiomeName), BiomeLocation(InLocation), ActorCount(0), bPopulated(false)
    {
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationReport : public UObject
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationReport();

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FBuild_IntegrationMetrics IntegrationMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FBuild_BiomeValidation> BiomeValidations;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FString FinalBuildStatus = TEXT("Unknown");

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateBiomePopulation(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsIntegrationSuccessful() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetDetailedReport() const;

private:
    void InitializeBiomeValidations();
    void CountActorsInBiome(UWorld* World, FBuild_BiomeValidation& BiomeValidation);
};