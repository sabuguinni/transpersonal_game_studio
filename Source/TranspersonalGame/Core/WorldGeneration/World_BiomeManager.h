#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TargetActorCount = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> RockAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurAssets;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    bool bAutoPopulateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 MaxActorsPerFrame = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    float PopulationTickInterval = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 TotalSpawnedActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    TMap<EBiomeType, int32> ActorsPerBiome;

private:
    FTimerHandle PopulationTimerHandle;
    int32 CurrentBiomeIndex = 0;
    int32 CurrentActorIndex = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void PopulateBiome(const FWorld_BiomeDefinition& BiomeDefinition);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void ClearBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void ClearAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetActorCountInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    TArray<AActor*> GetActorsInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management", CallInEditor)
    void InitializeDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management", CallInEditor)
    void ValidateBiomeAssets();

private:
    void PopulationTick();
    AActor* SpawnActorInBiome(const FString& AssetPath, const FVector& Location, const FRotator& Rotation);
    FVector GetRandomLocationInBiome(const FWorld_BiomeDefinition& BiomeDefinition) const;
    bool IsLocationValidForSpawn(const FVector& Location) const;
    void UpdateBiomeStatistics();
};