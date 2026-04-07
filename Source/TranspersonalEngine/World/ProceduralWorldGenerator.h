#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralWorldGenerator.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessZoneType : uint8
{
    Meditation     UMETA(DisplayName = "Meditation Zone"),
    Awareness      UMETA(DisplayName = "Awareness Zone"),
    Reflection     UMETA(DisplayName = "Reflection Zone"),
    Transformation UMETA(DisplayName = "Transformation Zone"),
    Integration    UMETA(DisplayName = "Integration Zone")
};

USTRUCT(BlueprintType)
struct FConsciousnessZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UStaticMesh*> ZoneMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UMaterialInterface* ZoneMaterial;

    FConsciousnessZoneData()
    {
        ZoneType = EConsciousnessZoneType::Meditation;
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        ConsciousnessIntensity = 1.0f;
        ZoneMaterial = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FProceduralBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerrainHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UStaticMesh*> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UStaticMesh*> RockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UMaterialInterface* TerrainMaterial;

    FProceduralBiomeData()
    {
        BiomeName = TEXT("Default");
        TerrainHeight = 0.0f;
        VegetationDensity = 0.5f;
        AmbientColor = FLinearColor::White;
        TerrainMaterial = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALENGINE_API AProceduralWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    AProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // World Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 RandomSeed;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    TArray<FConsciousnessZoneData> ConsciousnessZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float WorldResponseSensitivity;

    // Biome System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FProceduralBiomeData> AvailableBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionSmoothing;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LoadDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UnloadDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveChunks;

    // Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateChunk(int32 ChunkX, int32 ChunkY);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UnloadChunk(int32 ChunkX, int32 ChunkY);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateWorldBasedOnConsciousness(float ConsciousnessLevel, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    FConsciousnessZoneData GetNearestConsciousnessZone(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FProceduralBiomeData GetBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeWorldPerformance();

protected:
    // Internal generation functions
    float GenerateTerrainHeight(float X, float Y);
    void PlaceVegetation(FVector Location, const FProceduralBiomeData& Biome);
    void CreateConsciousnessZone(const FConsciousnessZoneData& ZoneData);
    void UpdateDynamicLighting(float ConsciousnessLevel);
    void UpdateAmbientSounds(const FConsciousnessZoneData& Zone);

    // Chunk management
    TMap<FIntPoint, class AWorldChunk*> LoadedChunks;
    TArray<FIntPoint> ChunksToLoad;
    TArray<FIntPoint> ChunksToUnload;

    // Consciousness integration
    float CurrentConsciousnessLevel;
    FVector LastPlayerLocation;
    float TimeSinceLastUpdate;

    // Performance tracking
    int32 CurrentActiveChunks;
    float LastPerformanceCheck;
};