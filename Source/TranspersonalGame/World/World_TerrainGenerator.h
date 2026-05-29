#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 TerrainSize = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float WaterLevel = 100.0f;

    FWorld_TerrainSettings()
    {
        HeightScale = 1000.0f;
        NoiseScale = 0.001f;
        TerrainSize = 2048;
        WaterLevel = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    FWorld_BiomeZone()
    {
        BiomeType = EBiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 50000.0f;
        HeightModifier = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_TerrainGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_TerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Terrain generation methods
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBaseTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWaterSystems();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlaceVegetation();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SpawnDinosaurs();

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetTerrainHeightAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsLocationInWater(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void RegenerateWorld();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FWorld_TerrainSettings TerrainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoGenerateOnPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bGenerateWater = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bGenerateVegetation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bSpawnDinosaurs = true;

    // Runtime data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> GeneratedActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedDinosaurs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> WaterActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bWorldGenerated = false;

private:
    // Internal generation methods
    void SetupDefaultBiomes();
    void CreateRiverSystem();
    void SpawnBiomeVegetation(const FWorld_BiomeZone& Biome);
    void SpawnDinosaursInBiome(const FWorld_BiomeZone& Biome);
    AActor* SpawnDinosaurMesh(const FString& AssetPath, const FVector& Location, const FRotator& Rotation, const FString& Label);
    
    // Noise generation
    float GeneratePerlinNoise(float X, float Y) const;
    float GenerateRidgedNoise(float X, float Y) const;
};