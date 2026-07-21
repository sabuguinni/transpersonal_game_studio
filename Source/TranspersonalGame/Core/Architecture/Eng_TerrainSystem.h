#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "SharedTypes.h"
#include "Eng_TerrainSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString LayerName = "Base";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    class UMaterialInterface* LayerMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float BlendWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MinElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxElevation = 1000.0f;

    FEng_TerrainLayer()
    {
        LayerName = "Base";
        LayerMaterial = nullptr;
        BlendWeight = 1.0f;
        NoiseScale = 0.001f;
        MinElevation = 0.0f;
        MaxElevation = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ChunkSize = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsGenerated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType DominantBiome = EBiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<FEng_TerrainLayer> TerrainLayers;

    FEng_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = 1024;
        bIsGenerated = false;
        DominantBiome = EBiomeType::Plains;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TerrainManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TerrainManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void GenerateTerrainChunk(const FVector& ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    float GetHeightAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    FVector GetNormalAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    bool IsLocationWalkable(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void CreateLandscapeActor(const FVector& Location, int32 SizeX = 1024, int32 SizeY = 1024);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void ApplyBiomeToTerrain(const FVector& Location, EBiomeType BiomeType, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    TArray<FEng_TerrainChunk> GetActiveChunks();

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void SetTerrainQuality(ETerrainQuality Quality);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    TMap<FVector, FEng_TerrainChunk> TerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    int32 ChunkSize = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    float MaxHeight = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    float NoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    ETerrainQuality TerrainQuality = ETerrainQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    TArray<FEng_TerrainLayer> DefaultLayers;

private:
    void InitializeDefaultLayers();
    float GenerateHeightNoise(const FVector2D& Location);
    void UpdateChunkLOD(FEng_TerrainChunk& Chunk, float DistanceToPlayer);
    class ALandscape* FindOrCreateLandscape();
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_TerrainComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_TerrainComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Terrain Component")
    void UpdateTerrainInfo();

    UFUNCTION(BlueprintCallable, Category = "Terrain Component")
    float GetGroundHeight() const { return GroundHeight; }

    UFUNCTION(BlueprintCallable, Category = "Terrain Component")
    FVector GetGroundNormal() const { return GroundNormal; }

    UFUNCTION(BlueprintCallable, Category = "Terrain Component")
    bool IsOnSteepSlope() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Component")
    ETerrainType GetTerrainType() const { return CurrentTerrainType; }

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerrainChanged, ETerrainType, OldTerrain, ETerrainType, NewTerrain);
    UPROPERTY(BlueprintAssignable, Category = "Terrain Component")
    FOnTerrainChanged OnTerrainChanged;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Component")
    float GroundHeight = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Component")
    FVector GroundNormal = FVector::UpVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Component")
    ETerrainType CurrentTerrainType = ETerrainType::Grass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Component")
    float SteepSlopeThreshold = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Component")
    float UpdateInterval = 0.1f;

private:
    float TimeSinceLastUpdate = 0.0f;
    UEng_TerrainManager* TerrainManager = nullptr;
    ETerrainType DetermineTerrainType(const FVector& Location);
};