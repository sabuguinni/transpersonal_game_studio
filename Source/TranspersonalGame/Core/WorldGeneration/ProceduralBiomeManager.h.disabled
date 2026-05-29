#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "Landscape/Landscape.h"
#include "../SharedTypes.h"
#include "ProceduralBiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor DebugColor;

    FWorld_BiomeZone()
    {
        BiomeType = EWorld_BiomeType::Forest;
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        Temperature = 25.0f;
        Humidity = 0.5f;
        DebugColor = FLinearColor::Green;
    }
};

USTRUCT(BlueprintType)
struct FWorld_VegetationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float TreeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float BushDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float GrassDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> TreeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> BushMeshes;

    FWorld_VegetationSettings()
    {
        TreeDensity = 0.3f;
        BushDensity = 0.5f;
        GrassDensity = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProceduralBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AProceduralBiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core biome management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EWorld_BiomeType, FWorld_VegetationSettings> VegetationSettings;

    // World generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSizeX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSizeY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float HeightScale;

    // Debug and visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowBiomeDebugSpheres;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowVegetationDebug;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Public methods
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void GenerateInitialBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void PlaceVegetationInBiomes();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ToggleBiomeDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ClearAllVegetation();

protected:
    // Internal methods
    void InitializeDefaultBiomes();
    void InitializeVegetationSettings();
    void CreateBiomeDebugSpheres();
    void SpawnVegetationForBiome(const FWorld_BiomeZone& BiomeZone);
    FVector GenerateRandomLocationInBiome(const FWorld_BiomeZone& BiomeZone) const;
    bool IsLocationValidForVegetation(const FVector& Location) const;

private:
    // Internal state
    TArray<AActor*> DebugSphereActors;
    TArray<AActor*> VegetationActors;
    bool bBiomesInitialized;
};