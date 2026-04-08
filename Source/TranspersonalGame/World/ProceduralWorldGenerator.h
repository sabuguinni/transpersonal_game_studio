#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Math/Vector.h"
#include "Math/Color.h"
#include "ProceduralWorldGenerator.generated.h"

class UConsciousnessSystem;
class AConsciousnessActor;

UENUM(BlueprintType)
enum class EConsciousnessLevel : uint8
{
    Ordinary        UMETA(DisplayName = "Ordinary Consciousness"),
    Heightened      UMETA(DisplayName = "Heightened Awareness"),
    Transcendent    UMETA(DisplayName = "Transcendent State"),
    Unity           UMETA(DisplayName = "Unity Consciousness")
};

USTRUCT(BlueprintType)
struct FBiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessResonance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AuraColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SacredGeometryIntensity = 0.0f;
};

USTRUCT(BlueprintType)
struct FTerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ChunkCoordinates = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ChunkSize = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> HeightMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBiomeParameters BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLoaded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNeedsRegeneration = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastConsciousnessLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct FSacredGeometryPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PatternName = TEXT("Flower of Life");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> GeometryPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessLevel RequiredLevel = EConsciousnessLevel::Ordinary;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProceduralWorldGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "Procedural World")
    void GenerateWorldChunk(FVector ChunkCoordinates);

    UFUNCTION(BlueprintCallable, Category = "Procedural World")
    void RegenerateChunkForConsciousness(FVector ChunkCoordinates, float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Procedural World")
    FTerrainChunk GetOrCreateChunk(FVector ChunkCoordinates);

    UFUNCTION(BlueprintCallable, Category = "Procedural World")
    void UpdateWorldForConsciousness(float ConsciousnessLevel, FVector PlayerLocation);

    // Biome Generation
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    FBiomeParameters GenerateBiome(FVector Location, float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    float GetTerrainHeight(FVector Location, const FBiomeParameters& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    FLinearColor GetAuraColor(FVector Location, float ConsciousnessLevel);

    // Sacred Geometry
    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    void GenerateSacredGeometry(FVector Location, EConsciousnessLevel ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    TArray<FVector> GetFlowerOfLifePattern(FVector Center, float Scale);

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    TArray<FVector> GetMeridianPattern(FVector Center, float Scale);

    UFUNCTION(BlueprintCallable, Category = "Sacred Geometry")
    TArray<FVector> GetSriYantraPattern(FVector Center, float Scale);

    // Noise Functions
    UFUNCTION(BlueprintCallable, Category = "Noise")
    float GetPerlinNoise(FVector Location, float Scale = 1.0f, int32 Octaves = 4);

    UFUNCTION(BlueprintCallable, Category = "Noise")
    float GetSimplexNoise(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Noise")
    float GetConsciousnessNoise(FVector Location, float ConsciousnessLevel);

    // Streaming and LOD
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateChunkLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadDistantChunks(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    bool ShouldRegenerateChunk(const FTerrainChunk& Chunk, float CurrentConsciousnessLevel);

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ChunkSize = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 RenderDistance = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float TerrainScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 HeightMapResolution = 513;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessInfluenceRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessRegenerationThreshold = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    bool bEnableConsciousnessBasedGeneration = true;

    // Noise Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 NoiseOctaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoisePersistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseLacunarity = 2.0f;

    // Sacred Geometry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    TArray<FSacredGeometryPattern> SacredPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    float GeometrySpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    float GeometryDensity = 0.1f;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxChunksPerFrame = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ChunkUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAsyncGeneration = true;

private:
    // Internal State
    UPROPERTY()
    TMap<FVector, FTerrainChunk> LoadedChunks;

    UPROPERTY()
    TArray<FVector> ChunksToGenerate;

    UPROPERTY()
    TArray<FVector> ChunksToRegenerate;

    UPROPERTY()
    UConsciousnessSystem* ConsciousnessSystem;

    float LastUpdateTime = 0.0f;
    FVector LastPlayerLocation = FVector::ZeroVector;
    float LastConsciousnessLevel = 0.0f;

    // Helper Functions
    FVector WorldToChunkCoordinates(FVector WorldLocation);
    FVector ChunkToWorldCoordinates(FVector ChunkCoordinates);
    float GetDistanceToChunk(FVector ChunkCoordinates, FVector PlayerLocation);
    void ProcessChunkQueue();
    void InitializeSacredPatterns();
};