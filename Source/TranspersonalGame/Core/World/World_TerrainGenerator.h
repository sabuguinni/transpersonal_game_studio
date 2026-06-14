#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<float> HeightData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsGenerated;

    FWorld_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = 512;
        BiomeType = TEXT("Plains");
        bIsGenerated = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainChunk(const FVector& ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateInitialTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void UpdateTerrainAroundPlayer(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    float GetHeightAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void SetTerrainParameters(float InNoiseScale, float InHeightMultiplier, int32 InOctaves);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ClearAllTerrain();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float HeightMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    int32 NoiseOctaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    int32 ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    float ChunkLoadDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    TSoftObjectPtr<UStaticMesh> TerrainMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Data")
    TMap<FVector, FWorld_TerrainChunk> GeneratedChunks;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Data")
    TArray<UStaticMeshComponent*> TerrainMeshComponents;

private:
    float GenerateNoiseValue(float X, float Y);
    void CreateTerrainMesh(const FWorld_TerrainChunk& Chunk);
    void UpdateChunkVisibility(const FVector& PlayerLocation);
    FVector WorldToChunkCoordinate(const FVector& WorldLocation);
};