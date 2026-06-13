#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Hills           UMETA(DisplayName = "Hills"),
    Mountains       UMETA(DisplayName = "Mountains"),
    Valley          UMETA(DisplayName = "Valley"),
    Plateau         UMETA(DisplayName = "Plateau"),
    Canyon          UMETA(DisplayName = "Canyon"),
    Coastline       UMETA(DisplayName = "Coastline")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkSize = FVector(1000.0f, 1000.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_TerrainType TerrainType = EWorld_TerrainType::Hills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightVariation = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 DetailLevel = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    bool bIsGenerated = false;

    FWorld_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = FVector(1000.0f, 1000.0f, 500.0f);
        TerrainType = EWorld_TerrainType::Hills;
        HeightVariation = 200.0f;
        NoiseScale = 0.01f;
        DetailLevel = 1;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<FWorld_TerrainChunk> TerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float WorldSize = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float ChunkSize = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 MaxActiveChunks = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float PlayerCheckRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TSoftObjectPtr<UStaticMesh> HillsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TSoftObjectPtr<UStaticMesh> MountainsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Generation")
    FVector PlayerLastLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Generation")
    int32 GeneratedChunksCount = 0;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainAroundPlayer();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainChunk(const FWorld_TerrainChunk& ChunkData);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ClearDistantChunks();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    FWorld_TerrainChunk CreateChunkData(const FVector& Location, EWorld_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    EWorld_TerrainType DetermineTerrainType(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    float GetHeightAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    TArray<FVector> GetChunkCentersAroundPlayer(float Radius) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Generation")
    void RegenerateAllTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Generation")
    void ClearAllTerrain();

private:
    UPROPERTY()
    TArray<UStaticMeshComponent*> ActiveChunkMeshes;

    UPROPERTY()
    float LastPlayerCheckTime = 0.0f;

    void UpdatePlayerLocation();
    FVector GetPlayerLocation() const;
    float PerlinNoise2D(float X, float Y, float Scale) const;
    void SpawnChunkMesh(const FWorld_TerrainChunk& ChunkData);
    void RemoveChunkMesh(int32 ChunkIndex);
};