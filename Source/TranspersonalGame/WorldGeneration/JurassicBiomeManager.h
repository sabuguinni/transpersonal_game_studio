#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Engine/DataAsset.h"
#include "JurassicBiomeManager.generated.h"

UENUM(BlueprintType)
enum class EJurassicBiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverValley     UMETA(DisplayName = "River Valley"), 
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyOutcrops   UMETA(DisplayName = "Rocky Outcrops"),
    SwampLands      UMETA(DisplayName = "Swamp Lands"),
    Coastline       UMETA(DisplayName = "Coastline"),
    Highlands       UMETA(DisplayName = "Highlands"),
    CanyonWalls     UMETA(DisplayName = "Canyon Walls")
};

USTRUCT(BlueprintType)
struct FJurassicBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EJurassicBiomeType BiomeType = EJurassicBiomeType::DenseForest;

    // Environmental conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float ElevationMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float ElevationMax = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float SlopeMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float SlopeMax = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MoistureMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MoistureMax = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TemperatureMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TemperatureMax = 1.0f;

    // Water proximity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bRequiresWaterProximity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float MaxWaterDistance = 1000.0f;

    // Biome blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float BlendRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendStrength = 0.5f;

    // PCG Generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    class UPCGGraph* BiomePCGGraph = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "0", ClampMax = "10"))
    int32 GenerationPriority = 5;
};

/**
 * Data Asset that defines biome configurations for the Jurassic world
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UJurassicBiomeDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FJurassicBiomeData> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalTemperature = 0.75f; // Warm Jurassic climate

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalMoisture = 0.65f; // High humidity

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float SeaLevel = 0.0f; // Reference sea level
};

/**
 * Manages biome distribution and environmental conditions across the Jurassic world
 * Works with PCG Framework to create realistic biome transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AJurassicBiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* BiomePCGComponent;

    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    class UJurassicBiomeDataAsset* BiomeDataAsset;

    // Environmental maps (textures for biome distribution)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Maps")
    class UTexture2D* ElevationMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Maps")
    class UTexture2D* MoistureMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Maps")
    class UTexture2D* TemperatureMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Maps")
    class UTexture2D* BiomeOverrideMap; // Manual biome painting

    // World bounds for biome calculations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Bounds")
    FVector WorldOrigin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Bounds")
    FVector WorldSize = FVector(201600.0f, 201600.0f, 25600.0f); // 2016m x 2016m x 256m

public:
    // Biome query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    EJurassicBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    FJurassicBiomeData GetBiomeDataAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    TArray<EJurassicBiomeType> GetBiomesInRadius(FVector WorldLocation, float Radius) const;

    // Environmental query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
    float GetMoistureAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
    float GetElevationAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environment")
    float GetSlopeAtLocation(FVector WorldLocation) const;

    // Biome generation
    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void RefreshBiomeGeneration();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void ClearBiomeGeneration();

private:
    // Internal calculation helpers
    FVector2D WorldToUV(FVector WorldLocation) const;
    float SampleTexture(UTexture2D* Texture, FVector2D UV) const;
    float CalculateBiomeSuitability(const FJurassicBiomeData& BiomeData, FVector WorldLocation) const;
    EJurassicBiomeType DetermineBiomeType(FVector WorldLocation) const;
    void SetupPCGBiomeGeneration();
};