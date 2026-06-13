#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "World_TerrainGenerator.generated.h"

UENUM(BlueprintType)
enum class EWorld_TerrainType : uint8
{
    Flat        UMETA(DisplayName = "Flat"),
    Hills       UMETA(DisplayName = "Hills"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Valley      UMETA(DisplayName = "Valley"),
    Canyon      UMETA(DisplayName = "Canyon"),
    Plateau     UMETA(DisplayName = "Plateau")
};

USTRUCT(BlueprintType)
struct FWorld_TerrainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_TerrainType TerrainType = EWorld_TerrainType::Hills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 Octaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Persistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Lacunarity = 2.0f;

    FWorld_TerrainSettings()
    {
        TerrainType = EWorld_TerrainType::Hills;
        HeightScale = 100.0f;
        NoiseScale = 0.01f;
        Octaves = 4;
        Persistence = 0.5f;
        Lacunarity = 2.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* TerrainMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FWorld_TerrainSettings TerrainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 TerrainSize = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float TerrainScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bGenerateRivers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    int32 RiverCount = 3;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Terrain", CallInEditor)
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Water", CallInEditor)
    void GenerateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetSurfaceNormal(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    bool IsLocationUnderwater(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void ApplyErosion(int32 Iterations = 100);

private:
    float PerlinNoise(float X, float Y) const;
    float FractalNoise(float X, float Y) const;
    void CreateTerrainMesh();
    void GenerateRivers();
    void PlaceLakes();

    TArray<FVector> TerrainVertices;
    TArray<int32> TerrainIndices;
    TArray<FVector> TerrainNormals;
    TArray<FVector2D> TerrainUVs;
};