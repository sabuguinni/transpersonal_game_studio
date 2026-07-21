#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "Eng_TerrainManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ChunkSize = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType DominantBiome = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<float> HeightData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsLoaded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bHasVegetation = false;

    FEng_TerrainChunk()
    {
        ChunkSize = 2048;
        DominantBiome = EBiomeType::Grassland;
        bIsLoaded = false;
        bHasVegetation = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_TerrainManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_TerrainManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    int32 WorldSize = 16384;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    int32 ChunkSize = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    float LoadDistance = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    float UnloadDistance = 6144.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    TMap<FIntPoint, FEng_TerrainChunk> LoadedChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    TArray<ALandscape*> LandscapeActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void InitializeTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void UpdateTerrainStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    FEng_TerrainChunk GenerateTerrainChunk(const FIntPoint& ChunkCoordinate);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void LoadTerrainChunk(const FIntPoint& ChunkCoordinate);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void UnloadTerrainChunk(const FIntPoint& ChunkCoordinate);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    float GetHeightAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    FVector GetSurfaceNormal(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    bool IsLocationWalkable(const FVector& WorldLocation, float MaxSlope = 45.0f);

private:
    void GenerateHeightData(FEng_TerrainChunk& Chunk);
    FIntPoint WorldLocationToChunkCoordinate(const FVector& WorldLocation);
    FVector ChunkCoordinateToWorldLocation(const FIntPoint& ChunkCoordinate);
    void CreateLandscapeForChunk(const FEng_TerrainChunk& Chunk);
};