// ProceduralWorldGenerator.h
// Sistema principal de geração procedural de mundo
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralWorldGenerator.generated.h"

USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UStaticMesh*> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UMaterialInterface* TerrainMaterial;
};

USTRUCT(BlueprintType)
struct FChunkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ChunkCoordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLoaded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGenerated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> HeightmapData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBiomeData BiomeData;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProceduralWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    AProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Configurações de geração
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ChunkSize = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ViewDistance = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float HeightMultiplier = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 Seed = 12345;

    // Biomas disponíveis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeData> AvailableBiomes;

    // Componentes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UProceduralMeshComponent* TerrainMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

private:
    // Gestão de chunks
    TMap<FVector2D, FChunkData> LoadedChunks;
    TArray<FVector2D> ChunksToGenerate;
    TArray<FVector2D> ChunksToUnload;

    // Cache de player position
    FVector LastPlayerPosition;
    class APawn* CachedPlayer;

    // Funções principais
    void UpdateChunks();
    void GenerateChunk(const FVector2D& ChunkCoord);
    void UnloadChunk(const FVector2D& ChunkCoord);
    bool ShouldLoadChunk(const FVector2D& ChunkCoord) const;

    // Geração de terreno
    void GenerateTerrain(FChunkData& ChunkData);
    float GenerateHeight(float X, float Y) const;
    FBiomeData DetermineBiome(float X, float Y, float Height) const;

    // Geração de vegetação
    void GenerateVegetation(const FChunkData& ChunkData);
    bool ShouldPlaceVegetation(float X, float Y, float Height, const FBiomeData& Biome) const;

    // Geração de estruturas
    void GenerateStructures(const FChunkData& ChunkData);
    bool ShouldPlaceStructure(float X, float Y, float Height) const;

    // Utilitários de noise
    float PerlinNoise(float X, float Y) const;
    float FractalNoise(float X, float Y, int32 Octaves = 4) const;
    float RidgedNoise(float X, float Y) const;

public:
    // Interface pública
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void RegenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetSeed(int32 NewSeed);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FBiomeData GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "World Generation")
    void OnChunkGenerated(const FVector2D& ChunkCoord);

    UFUNCTION(BlueprintImplementableEvent, Category = "World Generation")
    void OnChunkUnloaded(const FVector2D& ChunkCoord);
};