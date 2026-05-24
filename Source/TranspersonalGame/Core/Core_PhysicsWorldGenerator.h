#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Materials/MaterialInterface.h"
#include "Core_PhysicsWorldGenerator.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Plains      UMETA(DisplayName = "Plains"),
    Hills       UMETA(DisplayName = "Hills"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Valleys     UMETA(DisplayName = "Valleys"),
    Canyons     UMETA(DisplayName = "Canyons"),
    Plateaus    UMETA(DisplayName = "Plateaus")
};

UENUM(BlueprintType)
enum class ECore_BiomeType : uint8
{
    Tropical    UMETA(DisplayName = "Tropical Forest"),
    Temperate   UMETA(DisplayName = "Temperate Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Coastal     UMETA(DisplayName = "Coastal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ECore_TerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ECore_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ElevationVariance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bHasWaterFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<FVector> ResourceNodes;

    FCore_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = FVector(1000.0f, 1000.0f, 500.0f);
        TerrainType = ECore_TerrainType::Plains;
        BiomeType = ECore_BiomeType::Temperate;
        ElevationVariance = 100.0f;
        VegetationDensity = 0.5f;
        bHasWaterFeatures = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsTerrainProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GroundFriction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GroundRestitution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float SlopeDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float WaterDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector WindDirection;

    FCore_PhysicsTerrainProperties()
    {
        GroundFriction = 0.7f;
        GroundRestitution = 0.1f;
        SlopeDamping = 0.8f;
        WaterDensity = 1000.0f;
        WindStrength = 0.3f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
    }
};

/**
 * Core Physics World Generator - Generates physically accurate terrain with realistic Cretaceous period environments
 * Handles procedural terrain generation with proper physics properties for dinosaur survival gameplay
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsWorldGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsWorldGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === TERRAIN GENERATION ===
    
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrainChunk(const FVector& ChunkCenter, ECore_TerrainType TerrainType, ECore_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateFullTerrain(int32 ChunkCountX, int32 ChunkCountY);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ApplyPhysicsPropertiesToTerrain(const FCore_PhysicsTerrainProperties& Properties);

    // === BIOME SYSTEMS ===
    
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateTropicalForest(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateSavanna(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateWetlands(const FVector& Location, float Radius);

    // === WATER FEATURES ===
    
    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void GenerateRiver(const TArray<FVector>& RiverPath, float RiverWidth);

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void GenerateLake(const FVector& Center, float Radius, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void GenerateWaterfall(const FVector& TopLocation, const FVector& BottomLocation);

    // === RESOURCE PLACEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Resource Generation")
    void PlaceResourceNodes(ECore_BiomeType BiomeType, int32 NodeCount);

    UFUNCTION(BlueprintCallable, Category = "Resource Generation")
    void GenerateStoneOutcrops(const FVector& Location, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Resource Generation")
    void GenerateWoodResources(const FVector& Location, int32 TreeCount);

    // === PHYSICS INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetupTerrainCollision(AActor* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyWeatherEffects(float WindStrength, const FVector& WindDirection);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void UpdateTerrainPhysics(float DeltaTime);

    // === VALIDATION & TESTING ===
    
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void TestTerrainGeneration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void ClearGeneratedTerrain();

protected:
    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    FVector WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    FVector ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 MaxChunksX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 MaxChunksY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Properties")
    FCore_PhysicsTerrainProperties DefaultTerrainProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    float BiomeTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    float VegetationDensityMultiplier;

    // === RUNTIME DATA ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<FCore_TerrainChunk> GeneratedChunks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> TerrainActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> VegetationActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> WaterActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bIsGenerating;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float GenerationProgress;

private:
    // === INTERNAL METHODS ===
    
    void GenerateHeightmap(const FVector& ChunkCenter, const FVector& ChunkSize, ECore_TerrainType TerrainType);
    void ApplyBiomeVegetation(const FVector& Location, ECore_BiomeType BiomeType, float Density);
    void CreateWaterBody(const FVector& Location, const FVector& Size, float Depth);
    void SetupPhysicsMaterials();
    
    FVector CalculateTerrainNormal(const FVector& Location);
    float GetElevationAtLocation(const FVector& Location);
    bool IsLocationSuitableForVegetation(const FVector& Location, ECore_BiomeType BiomeType);
    
    // === PHYSICS MATERIALS ===
    
    UPROPERTY()
    class UPhysicalMaterial* GrassPhysicsMaterial;

    UPROPERTY()
    class UPhysicalMaterial* StonePhysicsMaterial;

    UPROPERTY()
    class UPhysicalMaterial* MudPhysicsMaterial;

    UPROPERTY()
    class UPhysicalMaterial* SandPhysicsMaterial;

    UPROPERTY()
    class UPhysicalMaterial* WaterPhysicsMaterial;
};