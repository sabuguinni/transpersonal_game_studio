#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/NoExportTypes.h"
#include "../SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

/**
 * Biome Architecture System - Defines the technical foundation for all biome systems
 * This system ensures that all biome-related modules follow consistent patterns
 * and maintain proper separation of concerns.
 */

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Swamp = 0,      // SW quadrant - wetlands, murky water, dense vegetation
    Forest = 1,     // NW quadrant - dense trees, undergrowth, filtered light
    Savanna = 2,    // Center - grasslands, scattered trees, open spaces
    Desert = 3,     // E quadrant - arid, sparse vegetation, sand/rock
    Mountains = 4   // NE quadrant - rocky terrain, elevation, alpine vegetation
};

UENUM(BlueprintType)
enum class EEng_BiomeLayer : uint8
{
    Terrain = 0,        // Base landscape and elevation
    Vegetation = 1,     // Trees, grass, bushes, plants
    Water = 2,          // Rivers, lakes, wetlands
    Atmosphere = 3,     // Fog, lighting, weather effects
    Wildlife = 4,       // Dinosaurs and other creatures
    Resources = 5,      // Rocks, minerals, crafting materials
    Structures = 6      // Natural formations, caves, cliffs
};

UENUM(BlueprintType)
enum class EEng_BiomeQuality : uint8
{
    Placeholder = 0,    // Basic shapes, development only
    Low = 1,           // Simple meshes, basic materials
    Medium = 2,        // Detailed meshes, good materials
    High = 3,          // High-poly meshes, advanced materials
    Ultra = 4          // Photorealistic, commercial quality
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeRegion
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Region")
    EEng_BiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Region")
    FVector CenterLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Region")
    float Radius;

    UPROPERTY(BlueprintReadOnly, Category = "Region")
    FVector2D BoundingBox;

    UPROPERTY(BlueprintReadOnly, Category = "Region")
    float Temperature;

    UPROPERTY(BlueprintReadOnly, Category = "Region")
    float Humidity;

    UPROPERTY(BlueprintReadOnly, Category = "Region")
    float Elevation;

    FEng_BiomeRegion()
    {
        BiomeType = EEng_BiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        BoundingBox = FVector2D(2000.0f, 2000.0f);
        Temperature = 25.0f;
        Humidity = 50.0f;
        Elevation = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeLayer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Layer")
    EEng_BiomeLayer LayerType;

    UPROPERTY(BlueprintReadOnly, Category = "Layer")
    FString LayerName;

    UPROPERTY(BlueprintReadOnly, Category = "Layer")
    bool bEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Layer")
    float Density;

    UPROPERTY(BlueprintReadOnly, Category = "Layer")
    EEng_BiomeQuality Quality;

    UPROPERTY(BlueprintReadOnly, Category = "Layer")
    TArray<FString> AssetPaths;

    FEng_BiomeLayer()
    {
        LayerType = EEng_BiomeLayer::Terrain;
        LayerName = TEXT("UnnamedLayer");
        bEnabled = true;
        Density = 1.0f;
        Quality = EEng_BiomeQuality::Low;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    EEng_BiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    TArray<FEng_BiomeLayer> Layers;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    FLinearColor AmbientColor;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    float FogDensity;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    FLinearColor FogColor;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    float WindStrength;

    FEng_BiomeConfiguration()
    {
        BiomeType = EEng_BiomeType::Savanna;
        BiomeName = TEXT("DefaultBiome");
        AmbientColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        FogDensity = 0.1f;
        FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
        WindStrength = 1.0f;
    }
};

/**
 * Core biome architecture component that manages biome system integration
 * and ensures proper layering and quality control.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_BiomeArchitecture : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_BiomeArchitecture();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Biome system management
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool RegisterBiomeRegion(const FEng_BiomeRegion& Region);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool UnregisterBiomeRegion(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void ValidateBiomeArchitecture();

    // Biome configuration
    UFUNCTION(BlueprintCallable, Category = "Biome Configuration")
    void SetBiomeConfiguration(const FEng_BiomeConfiguration& Configuration);

    UFUNCTION(BlueprintCallable, Category = "Biome Configuration")
    FEng_BiomeConfiguration GetBiomeConfiguration(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Configuration")
    void UpdateBiomeLayer(EEng_BiomeType BiomeType, const FEng_BiomeLayer& Layer);

    // Quality management
    UFUNCTION(BlueprintCallable, Category = "Quality")
    void SetGlobalBiomeQuality(EEng_BiomeQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Quality")
    void SetBiomeQuality(EEng_BiomeType BiomeType, EEng_BiomeQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Quality")
    EEng_BiomeQuality GetBiomeQuality(EEng_BiomeType BiomeType) const;

    // Spatial queries
    UFUNCTION(BlueprintCallable, Category = "Spatial")
    EEng_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Spatial")
    FEng_BiomeRegion GetBiomeRegion(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Spatial")
    TArray<FEng_BiomeRegion> GetAllBiomeRegions() const;

    UFUNCTION(BlueprintCallable, Category = "Spatial")
    float GetDistanceToBiome(const FVector& Location, EEng_BiomeType BiomeType) const;

    // Editor utilities
    UFUNCTION(CallInEditor, Category = "Debug")
    void GenerateBiomeReport();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidateAllBiomes();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ResetBiomeSystem();

    UFUNCTION(CallInEditor, Category = "Setup")
    void CreateDefaultBiomeLayout();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    TArray<FEng_BiomeRegion> RegisteredRegions;

    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    TArray<FEng_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(BlueprintReadOnly, Category = "Quality")
    EEng_BiomeQuality GlobalQuality;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bBiomeSystemInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float BiomeUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastBiomeUpdate;

private:
    void InitializeDefaultBiomes();
    void UpdateBiomeStates();
    bool IsBiomeRegistered(EEng_BiomeType BiomeType) const;
    void CreateBiomeConfiguration(EEng_BiomeType BiomeType);
    void ValidateBiomeOverlaps();
    FEng_BiomeLayer CreateDefaultLayer(EEng_BiomeLayer LayerType);
};

/**
 * Biome Manager Actor - Central coordinator for all biome systems
 * This actor should be placed once in the level to manage all biome operations.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    UEng_BiomeArchitecture* GetBiomeArchitecture() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    bool IsWorldBiomeSystemReady() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    void InitializeWorldBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    void UpdateAllBiomes();

    UFUNCTION(CallInEditor, Category = "World Setup")
    void SetupMinPlayableMapBiomes();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidateWorldBiomes();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome System")
    UEng_BiomeArchitecture* BiomeArchitecture;

    UPROPERTY(BlueprintReadOnly, Category = "World State")
    bool bWorldBiomesInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "World State")
    float WorldSize;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveBiomeRegions;
};