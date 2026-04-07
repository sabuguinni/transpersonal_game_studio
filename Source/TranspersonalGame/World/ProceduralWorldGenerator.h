#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Core/ConsciousnessSystem.h"
#include "ProceduralWorldGenerator.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Meditation      UMETA(DisplayName = "Meditation Garden"),
    Memory          UMETA(DisplayName = "Memory Palace"),
    Dream           UMETA(DisplayName = "Dream Landscape"),
    Shadow          UMETA(DisplayName = "Shadow Realm"),
    Light           UMETA(DisplayName = "Light Dimension"),
    Void            UMETA(DisplayName = "Void Space"),
    Unity           UMETA(DisplayName = "Unity Field")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeConfiguration : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    ERealityLayer AssociatedLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EConsciousnessState RequiredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float TerrainScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float TerrainHeight = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 NoiseOctaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float NoiseFrequency = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> EnvironmentMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<UMaterialInterface> SkyboxMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<class USoundBase> AmbientSound;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChunkData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Chunk")
    FVector2D ChunkCoordinates;

    UPROPERTY(BlueprintReadOnly, Category = "Chunk")
    EBiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Chunk")
    bool bIsGenerated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Chunk")
    bool bIsLoaded = false;

    TArray<FVector> GeneratedVertices;
    TArray<int32> GeneratedTriangles;
    TArray<FVector> GeneratedNormals;
    TArray<FVector2D> GeneratedUVs;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeChanged, EBiomeType, OldBiome, EBiomeType, NewBiome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChunkGenerated, FVector2D, ChunkCoordinates);

/**
 * Procedural world generator that creates consciousness-responsive environments
 * Generates different biomes based on player's consciousness state and reality layer
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProceduralWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    AProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // World Generation
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorldAroundPlayer(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void RegenerateCurrentBiome();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void TransitionToBiome(EBiomeType NewBiome);

    // Chunk Management
    UFUNCTION(BlueprintCallable, Category = "Chunks")
    void LoadChunk(FVector2D ChunkCoordinates);

    UFUNCTION(BlueprintCallable, Category = "Chunks")
    void UnloadChunk(FVector2D ChunkCoordinates);

    UFUNCTION(BlueprintPure, Category = "Chunks")
    FVector2D GetChunkCoordinatesFromWorldLocation(const FVector& WorldLocation) const;

    // Biome Management
    UFUNCTION(BlueprintPure, Category = "Biomes")
    EBiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType DetermineBiomeFromConsciousness(EConsciousnessState State, ERealityLayer Layer) const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void LoadBiomeConfiguration(UDataTable* ConfigTable);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBiomeChanged OnBiomeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnChunkGenerated OnChunkGenerated;

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInstancedStaticMeshComponent> TerrainMeshComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TObjectPtr<UDataTable> BiomeConfigurationTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ChunkSize = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 RenderDistance = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxLoadedChunks = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BiomeTransitionDuration = 5.0f;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TransitionProgress = 0.0f;

private:
    // Consciousness System Reference
    UPROPERTY()
    TObjectPtr<UConsciousnessSystem> ConsciousnessSystem;

    // Chunk Management
    TMap<FVector2D, FChunkData> LoadedChunks;
    TQueue<FVector2D> ChunkGenerationQueue;

    // Generation Methods
    void GenerateChunk(FVector2D ChunkCoordinates, EBiomeType BiomeType);
    void GenerateTerrainMesh(FChunkData& ChunkData, const FBiomeConfiguration& Config);
    void PopulateChunkWithObjects(FChunkData& ChunkData, const FBiomeConfiguration& Config);
    
    // Noise Generation
    float GeneratePerlinNoise(float X, float Y, int32 Octaves, float Frequency, float Amplitude) const;
    float PerlinNoise2D(float X, float Y) const;
    
    // Consciousness Integration
    UFUNCTION()
    void OnConsciousnessStateChanged(EConsciousnessState OldState, EConsciousnessState NewState);
    
    UFUNCTION()
    void OnRealityLayerChanged(ERealityLayer OldLayer, ERealityLayer NewLayer);

    // Utility
    FBiomeConfiguration* GetBiomeConfiguration(EBiomeType BiomeType);
    void UpdateEnvironmentLighting(const FBiomeConfiguration& Config);
    void UpdateAmbientAudio(const FBiomeConfiguration& Config);
    
    // Transition Management
    void StartBiomeTransition(EBiomeType NewBiome);
    void UpdateBiomeTransition(float DeltaTime);
    void CompleteBiomeTransition();
    
    FTimerHandle TransitionTimer;
    EBiomeType TargetBiome;
};