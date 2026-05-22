#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_AdvancedBiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Pantano     UMETA(DisplayName = "Pantano"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 25000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TargetActorCount = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> PreferredDinosaurSpecies;

    FWorld_BiomeConfiguration()
    {
        PreferredDinosaurSpecies.Add("Trex");
        PreferredDinosaurSpecies.Add("Velociraptor");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_ProceduralSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString ActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    EWorld_BiomeType TargetBiome = EWorld_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bApplyLODOptimization = true;
};

/**
 * Advanced Biome System for Procedural World Generation
 * Manages 5 distinct biomes with realistic population distribution
 * Integrates with Performance LOD system for optimal rendering
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_AdvancedBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_AdvancedBiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bAutoPopulateBiomes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float PopulationUpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame = 10;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void PopulateBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeConfiguration GetBiomeConfiguration(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    int32 GetActorCountInBiome(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    AActor* SpawnProceduralActor(const FWorld_ProceduralSpawnData& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnDinosaurInBiome(EWorld_BiomeType BiomeType, const FString& DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnVegetationInBiome(EWorld_BiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void SpawnRocksInBiome(EWorld_BiomeType BiomeType, int32 Count);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomLocationInBiome(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsLocationInBiome(const FVector& Location, EWorld_BiomeType BiomeType) const;

    // Performance Integration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActorWithLODManager(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeAllBiomeActors();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorPopulateAllBiomes();

    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorClearAllBiomes();

    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorValidateBiomePopulation();

private:
    // Internal state
    float LastPopulationUpdate = 0.0f;
    int32 CurrentProcessingBiome = 0;
    bool bIsPopulating = false;

    // Asset paths for procedural spawning
    TArray<FString> VegetationAssetPaths;
    TArray<FString> RockAssetPaths;
    TArray<FString> DinosaurAssetPaths;

    // Helper functions
    void InitializeAssetPaths();
    void ProcessBiomePopulation();
    FVector GetValidSpawnLocation(const FVector& BaseLocation, float Radius) const;
    bool ValidateAssetPath(const FString& AssetPath) const;
    void ApplyBiomeSpecificSettings(AActor* Actor, EWorld_BiomeType BiomeType);
};