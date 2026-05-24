#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeProxy.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_PerformanceTerrainController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxLandscapeActors = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TerrainLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableTerrainCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TerrainResolution = 1024;

    FWorld_TerrainPerformanceSettings()
    {
        MaxLandscapeActors = 16;
        TerrainLODDistance = 5000.0f;
        bEnableTerrainCulling = true;
        CullingDistance = 15000.0f;
        TerrainResolution = 1024;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkSize = FVector(2000.0f, 2000.0f, 1000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType BiomeType = EBiomeType::Temperate_Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainComplexity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bNeedsGeneration = true;
};

/**
 * Performance-optimized terrain controller for massive world generation
 * Manages landscape streaming, LOD, and biome integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PerformanceTerrainController : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PerformanceTerrainController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Core terrain management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    FWorld_TerrainPerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    TArray<FWorld_TerrainChunk> TerrainChunks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    TArray<ALandscapeProxy*> ActiveLandscapes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain State")
    int32 ActiveChunkCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastPerformanceCheck = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 16.67f;

public:
    // Terrain generation and management
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void InitializeTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainChunk(const FVector& Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void UpdateTerrainLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void OptimizeTerrainPerformance();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CheckTerrainPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustTerrainQuality(float TargetFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentTerrainLoad() const;

    // Chunk management
    UFUNCTION(BlueprintCallable, Category = "Chunk Management")
    void ActivateTerrainChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "Chunk Management")
    void DeactivateTerrainChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "Chunk Management")
    void StreamTerrainChunks(const FVector& PlayerLocation);

    // Biome integration
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void ApplyBiomeToTerrain(ALandscapeProxy* Landscape, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void UpdateTerrainMaterials(ALandscapeProxy* Landscape, EBiomeType BiomeType);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector GetNearestChunkLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    int32 GetChunkIndexAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsChunkInRange(const FVector& ChunkLocation, const FVector& PlayerLocation) const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void EditorGenerateTestTerrain();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void EditorOptimizeAllTerrain();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void EditorValidateTerrainSystem();

private:
    // Internal terrain management
    void CreateLandscapeActor(const FVector& Location, const FVector& Scale);
    void UpdateChunkStates(const FVector& PlayerLocation);
    void ManageTerrainMemory();
    void ApplyPerformanceOptimizations();

    // Performance tracking
    float FrameTimeHistory[60];
    int32 FrameTimeIndex = 0;
    bool bPerformanceOptimizationEnabled = true;
};