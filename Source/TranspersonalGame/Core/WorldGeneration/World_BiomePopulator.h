#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkeletalMeshActor.h"
#include "SharedTypes.h"
#include "World_BiomePopulator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxTerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurTypes;

    FWorld_BiomeSpawnData()
    {
        Center = FVector::ZeroVector;
        Radius = 10000.0f;
        BiomeType = EBiomeType::Savana;
        MaxDinosaurs = 5;
        MaxTerrainFeatures = 20;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_DinosaurSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FString DinosaurName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float SpawnWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FVector MinScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FVector MaxScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    TArray<EBiomeType> PreferredBiomes;

    FWorld_DinosaurSpawnConfig()
    {
        DinosaurName = TEXT("Unknown");
        AssetPath = TEXT("");
        SpawnWeight = 1.0f;
        MinScale = FVector(0.8f, 0.8f, 0.8f);
        MaxScale = FVector(1.2f, 1.2f, 1.2f);
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomePopulator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomePopulator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_BiomeSpawnData> BiomeConfigurations;

    // Dinosaur spawn configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_DinosaurSpawnConfig> DinosaurConfigurations;

    // Population settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    bool bAutoPopulateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    float PopulationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    float MinSpawnDistance;

    // Runtime tracking
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 TotalSpawnedCount;

    // Core population methods
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PopulateBiome(const FWorld_BiomeSpawnData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    AActor* SpawnDinosaurInBiome(const FWorld_BiomeSpawnData& BiomeData, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    AActor* SpawnTerrainFeatureInBiome(const FWorld_BiomeSpawnData& BiomeData, const FString& FeatureType);

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector GetRandomPositionInBiome(const FWorld_BiomeSpawnData& BiomeData) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsPositionValidForSpawn(const FVector& Position, float CheckRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearSpawnedActors();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FString> GetAvailableDinosaursForBiome(EBiomeType BiomeType) const;

    // Configuration methods
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void InitializeDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void InitializeDefaultDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void AddBiomeConfiguration(const FWorld_BiomeSpawnData& NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void AddDinosaurConfiguration(const FWorld_DinosaurSpawnConfig& NewDinosaur);

    // Debug and validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateConfiguration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void TestSpawnSingleDinosaur();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawBiomeBounds();

private:
    // Internal helpers
    FWorld_DinosaurSpawnConfig* FindDinosaurConfig(const FString& DinosaurName);
    float CalculateSpawnWeight(const FWorld_DinosaurSpawnConfig& Config, EBiomeType BiomeType) const;
    FString SelectRandomDinosaurForBiome(EBiomeType BiomeType) const;
    
    // Spawn validation
    bool ValidateSpawnLocation(const FVector& Location, const FWorld_BiomeSpawnData& BiomeData) const;
    void AdjustActorForBiome(AActor* Actor, const FWorld_BiomeSpawnData& BiomeData) const;
};