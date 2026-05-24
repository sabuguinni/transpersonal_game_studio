#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeProxy.h"
#include "LandscapeEditorObject.h"
#include "SharedTypes.h"
#include "World_CretaceousLandscapeBuilder.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandscapeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentSizeQuads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 SectionsPerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 QuadsPerSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<EBiomeType> BiomeLayout;

    FWorld_LandscapeConfiguration()
    {
        LandscapeScale = FVector(100.0f, 100.0f, 100.0f);
        ComponentSizeQuads = 63;
        SectionsPerComponent = 2;
        QuadsPerSection = 63;
        BiomeLayout = {EBiomeType::Forest, EBiomeType::Plains, EBiomeType::Desert, EBiomeType::Mountains, EBiomeType::Swamp};
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightVariation;

    FWorld_BiomeZoneData()
    {
        BiomeType = EBiomeType::Forest;
        ZoneCenter = FVector2D::ZeroVector;
        ZoneRadius = 2000.0f;
        HeightVariation = 500.0f;
    }
};

/**
 * Cretaceous Landscape Builder - Creates and manages the main 10km x 10km landscape
 * with proper biome distribution for the prehistoric survival game.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CretaceousLandscapeBuilder : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CretaceousLandscapeBuilder();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Configuration")
    FWorld_LandscapeConfiguration LandscapeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    TArray<FWorld_BiomeZoneData> BiomeZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    class ALandscape* MainLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Generation")
    float BaseTerrainHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Generation")
    float NoiseIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    TArray<FVector2D> RiverPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    float RiverWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    float RiverDepth;

public:
    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    bool CreateCretaceousLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    bool GenerateHeightmapData(TArray<uint16>& HeightData, int32 Width, int32 Height);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void SetupBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    EBiomeType GetBiomeAtLocation(FVector2D Location) const;

    UFUNCTION(BlueprintCallable, Category = "Height Generation")
    float CalculateHeightAtLocation(FVector2D Location) const;

    UFUNCTION(BlueprintCallable, Category = "Rivers")
    void GenerateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "Rivers")
    bool IsLocationNearRiver(FVector2D Location, float Threshold = 200.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Utilities")
    FVector GetLandscapeSize() const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Utilities")
    bool IsLandscapeValid() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void BuildLandscapeInEditor();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RegenerateBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void ValidateLandscapeSetup();

private:
    float GeneratePerlinNoise(float X, float Y, float Scale) const;
    float SmoothStep(float Edge0, float Edge1, float X) const;
    uint16 FloatToHeightmapValue(float Height) const;
    void ApplyRiverToHeightmap(TArray<uint16>& HeightData, int32 Width, int32 Height);
};