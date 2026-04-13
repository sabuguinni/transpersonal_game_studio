#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "WorldGenerationTypes.h"
#include "WorldGenerationManager.generated.h"

class UWorld_NoiseGenerator;
class UWorld_BiomeManager;
class UWorld_RiverSystem;
class ALandscape;

/**
 * Main manager for procedural world generation
 * Coordinates terrain generation, biome placement, and river systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorldGenerationManager : public AActor
{
    GENERATED_BODY()

public:
    AWorldGenerationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // World generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    FWorld_GenerationSettings GenerationSettings;

    // Generation components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWorld_NoiseGenerator> NoiseGenerator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWorld_BiomeManager> BiomeManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWorld_RiverSystem> RiverSystem;

    // Generation functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRivers();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearWorld();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetHeightAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector GetNearestRiver(FVector WorldLocation) const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "World Generation")
    void RegenerateWorld();

    UFUNCTION(CallInEditor, Category = "World Generation")
    void PreviewGeneration();

protected:
    // Internal generation state
    UPROPERTY()
    bool bIsGenerating = false;

    UPROPERTY()
    bool bWorldGenerated = false;

    // Generated landscape reference
    UPROPERTY()
    TObjectPtr<ALandscape> GeneratedLandscape;

    // Chunk management for streaming
    UPROPERTY()
    TArray<FVector2D> LoadedChunks;

    UPROPERTY()
    TArray<FVector2D> PendingChunks;

    // Internal generation functions
    void InitializeComponents();
    void SetupWorldPartition();
    void GenerateChunk(FVector2D ChunkCoordinate);
    void UnloadChunk(FVector2D ChunkCoordinate);
    
    // Noise-based terrain generation
    TArray<float> GenerateHeightmap(int32 Size, FVector2D Offset);
    void ApplyBiomeInfluence(TArray<float>& Heightmap, int32 Size, FVector2D Offset);
    void CarveRivers(TArray<float>& Heightmap, int32 Size, FVector2D Offset);

private:
    // Performance monitoring
    float LastGenerationTime = 0.0f;
    int32 GeneratedChunkCount = 0;
};