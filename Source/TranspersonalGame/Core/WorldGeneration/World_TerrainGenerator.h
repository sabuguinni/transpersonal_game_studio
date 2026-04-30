#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "../SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Plains      UMETA(DisplayName = "Plains"),
    Forest      UMETA(DisplayName = "Forest"),
    Mountain    UMETA(DisplayName = "Mountain"),
    River       UMETA(DisplayName = "River"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Desert      UMETA(DisplayName = "Desert")
};

USTRUCT(BlueprintType)
struct FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_TerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<AActor*> SpawnedActors;

    FWorld_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        TerrainType = EWorld_TerrainType::Plains;
        HeightVariation = 100.0f;
        VegetationDensity = 5;
        SpawnedActors.Empty();
    }
};

USTRUCT(BlueprintType)
struct FWorld_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_TerrainType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MinHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MaxHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnRadius;

    FWorld_BiomeSettings()
    {
        BiomeType = EWorld_TerrainType::Plains;
        BiomeColor = FLinearColor::Green;
        MinHeight = 0.0f;
        MaxHeight = 200.0f;
        VegetationCount = 10;
        SpawnRadius = 1000.0f;
    }
};

/**
 * Terrain Generator Component for procedural world creation
 * Handles biome generation, height variation, and environmental placement
 */
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

    // Terrain Generation Methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainChunk(const FVector& ChunkCenter, EWorld_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateBiomeArea(const FVector& Center, const FWorld_BiomeSettings& BiomeSettings);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void SpawnVegetationCluster(const FVector& Center, int32 Count, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateWaterBody(const FVector& Start, const FVector& End, float Width);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateRockFormations(const FVector& Center, int32 Count, float HeightVariation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation", CallInEditor)
    void RegenerateAllTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation", CallInEditor)
    void ClearGeneratedTerrain();

    // Utility Methods
    UFUNCTION(BlueprintPure, Category = "Terrain Generation")
    float GetHeightAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "Terrain Generation")
    EWorld_TerrainType GetTerrainTypeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void SetBiomeSettings(EWorld_TerrainType BiomeType, const FWorld_BiomeSettings& Settings);

protected:
    // Configuration Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    TMap<EWorld_TerrainType, FWorld_BiomeSettings> BiomeSettingsMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    int32 MaxChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float HeightMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    bool bAutoGenerate;

    // Runtime Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    TArray<FWorld_TerrainChunk> GeneratedChunks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    TArray<AActor*> SpawnedTerrainActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    int32 CurrentChunkCount;

private:
    // Internal Methods
    void InitializeBiomeSettings();
    FVector CalculateNoiseOffset(const FVector& Location) const;
    AActor* SpawnTerrainActor(UClass* ActorClass, const FVector& Location, const FRotator& Rotation);
    void CleanupTerrainActors();
};