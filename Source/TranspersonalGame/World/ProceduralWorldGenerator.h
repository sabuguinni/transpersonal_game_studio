/**
 * @file ProceduralWorldGenerator.h
 * @brief Advanced procedural world generation system optimized for consciousness mechanics
 * @author Transpersonal Game Studio - Procedural World Generator Agent
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Landscape/Classes/Landscape.h"
#include "Foliage/Public/FoliageType.h"
#include "../Core/ConsciousnessSystem.h"
#include "../Performance/PhysicsOptimizer.h"
#include "ProceduralWorldGenerator.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Sacred_Grove     UMETA(DisplayName = "Sacred Grove"),
    Meditation_Valley UMETA(DisplayName = "Meditation Valley"),
    Crystal_Caves    UMETA(DisplayName = "Crystal Caves"),
    Ethereal_Plains  UMETA(DisplayName = "Ethereal Plains"),
    Wisdom_Mountains UMETA(DisplayName = "Wisdom Mountains"),
    Reflection_Lakes UMETA(DisplayName = "Reflection Lakes"),
    Void_Spaces      UMETA(DisplayName = "Void Spaces")
};

UENUM(BlueprintType)
enum class EConsciousnessZone : uint8
{
    Grounding       UMETA(DisplayName = "Grounding Zone"),
    Awakening       UMETA(DisplayName = "Awakening Zone"),
    Transcendence   UMETA(DisplayName = "Transcendence Zone"),
    Unity           UMETA(DisplayName = "Unity Zone"),
    Void_Meditation UMETA(DisplayName = "Void Meditation Zone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Sacred_Grove;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EConsciousnessZone ConsciousnessZone = EConsciousnessZone::Grounding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float EnergyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AuraColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> BiomeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UFoliageType>> FoliageTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D HeightRange = FVector2D(0.0f, 1000.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChunkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    FIntPoint ChunkCoordinates = FIntPoint::ZeroValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    bool bIsLoaded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    bool bIsGenerated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    EBiomeType PrimaryBiome = EBiomeType::Sacred_Grove;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    TArray<AActor*> SpawnedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    float ConsciousnessResonance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk")
    FDateTime LastAccessTime;
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
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateChunk(FIntPoint ChunkCoordinates);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UnloadChunk(FIntPoint ChunkCoordinates);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UpdateChunksAroundPlayer(FVector PlayerLocation);

    // Biome and Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    EBiomeType DetermineBiomeFromConsciousness(float ConsciousnessLevel, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateChunkConsciousnessResonance(FIntPoint ChunkCoordinates, float ResonanceValue);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    FLinearColor GetConsciousnessAura(FVector Location);

    // Optimization Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLoadedChunks();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODLevel(FIntPoint ChunkCoordinates, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAsyncGeneration(bool bEnable);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FIntPoint WorldToChunkCoordinates(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector ChunkToWorldLocation(FIntPoint ChunkCoordinates);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsChunkLoaded(FIntPoint ChunkCoordinates);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChunkGenerated, FIntPoint, ChunkCoordinates, EBiomeType, BiomeType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChunkUnloaded, FIntPoint, ChunkCoordinates, float, ConsciousnessResonance);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnConsciousnessZoneEntered, EConsciousnessZone, Zone, FVector, Location, float, Intensity);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnChunkGenerated OnChunkGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnChunkUnloaded OnChunkUnloaded;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnConsciousnessZoneEntered OnConsciousnessZoneEntered;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float ChunkSize = 25600.0f; // 256m x 256m chunks

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 RenderDistance = 5; // Chunks around player

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 MaxLoadedChunks = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    bool bUseAsyncGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 WorldSeed = 12345;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, FBiomeData> BiomeDataMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float ConsciousnessInfluenceRadius = 10000.0f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ChunkUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerChunk = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 10000.0f;

private:
    // Internal Data
    UPROPERTY()
    TMap<FIntPoint, FChunkData> LoadedChunks;

    UPROPERTY()
    UConsciousnessSystem* ConsciousnessSystem;

    UPROPERTY()
    UPhysicsOptimizer* PhysicsOptimizer;

    UPROPERTY()
    FVector LastPlayerLocation;

    UPROPERTY()
    float ChunkUpdateTimer;

    // Internal Functions
    void GenerateTerrainForChunk(FIntPoint ChunkCoordinates, FChunkData& ChunkData);
    void GenerateFoliageForChunk(FIntPoint ChunkCoordinates, FChunkData& ChunkData);
    void GenerateStructuresForChunk(FIntPoint ChunkCoordinates, FChunkData& ChunkData);
    void ApplyConsciousnessEffects(FIntPoint ChunkCoordinates, FChunkData& ChunkData);
    
    float GenerateNoise(float X, float Y, int32 Octaves = 4);
    float GetBiomeWeight(EBiomeType BiomeType, FVector Location, float ConsciousnessLevel);
    void CleanupOldChunks();
    void UpdateChunkLOD(FIntPoint ChunkCoordinates, float DistanceToPlayer);
    
    // Async Generation
    void StartAsyncChunkGeneration(FIntPoint ChunkCoordinates);
    void OnAsyncGenerationComplete(FIntPoint ChunkCoordinates, TArray<AActor*> GeneratedActors);
};