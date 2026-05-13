#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "LandscapeEditorObject.h"
#include "LandscapeEditorModule.h"
#include "LandscapeSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "World_MassiveLandscapeCreator.generated.h"

UENUM(BlueprintType)
enum class EWorld_LandscapeCreationMethod : uint8
{
    NativeLandscape     UMETA(DisplayName = "Native Landscape Actor"),
    StaticMeshTerrain   UMETA(DisplayName = "Static Mesh Terrain Grid"),
    ProceduralMesh      UMETA(DisplayName = "Procedural Mesh Terrain"),
    HybridApproach      UMETA(DisplayName = "Hybrid Landscape + Mesh")
};

UENUM(BlueprintType)
enum class EWorld_BiomeZoneType : uint8
{
    SwampSouthwest      UMETA(DisplayName = "Swamp (Southwest)"),
    ForestNorthwest     UMETA(DisplayName = "Forest (Northwest)"),
    SavannaCenter       UMETA(DisplayName = "Savanna (Center)"),
    DesertEast          UMETA(DisplayName = "Desert (East)"),
    MountainNortheast   UMETA(DisplayName = "Mountain (Northeast)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    EWorld_BiomeZoneType BiomeType = EWorld_BiomeZoneType::SavannaCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float RadiusKilometers = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float MinElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float MaxElevation = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FLinearColor BiomeColor = FLinearColor::White;

    FWorld_BiomeZoneConfig()
    {
        BiomeType = EWorld_BiomeZoneType::SavannaCenter;
        CenterLocation = FVector::ZeroVector;
        RadiusKilometers = 2.0f;
        MinElevation = 0.0f;
        MaxElevation = 1000.0f;
        BiomeColor = FLinearColor::White;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandscapeCreationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    EWorld_LandscapeCreationMethod CreationMethod = EWorld_LandscapeCreationMethod::HybridApproach;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    float TotalSizeKilometers = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    int32 ComponentCountX = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    int32 ComponentCountY = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    int32 QuadsPerComponent = 63;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    FVector LandscapeScale = FVector(100.0f, 100.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Creation")
    TArray<FWorld_BiomeZoneConfig> BiomeZones;

    FWorld_LandscapeCreationConfig()
    {
        CreationMethod = EWorld_LandscapeCreationMethod::HybridApproach;
        TotalSizeKilometers = 10.0f;
        ComponentCountX = 32;
        ComponentCountY = 32;
        QuadsPerComponent = 63;
        LandscapeScale = FVector(100.0f, 100.0f, 100.0f);
    }
};

/**
 * Massive Landscape Creator - Creates 10km x 10km landscapes for asset purchase criteria
 * Implements multiple creation methods and biome zone management
 * Critical for meeting asset purchase requirements and world expansion
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_MassiveLandscapeCreator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UWorld_MassiveLandscapeCreator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Landscape Creation Functions
    UFUNCTION(BlueprintCallable, Category = "Massive Landscape", CallInEditor = true)
    bool CreateMassiveLandscape(const FWorld_LandscapeCreationConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Massive Landscape", CallInEditor = true)
    bool CreateNativeLandscape(const FWorld_LandscapeCreationConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Massive Landscape", CallInEditor = true)
    bool CreateStaticMeshTerrain(const FWorld_LandscapeCreationConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Massive Landscape", CallInEditor = true)
    bool CreateHybridLandscape(const FWorld_LandscapeCreationConfig& Config);

    // Biome Zone Management
    UFUNCTION(BlueprintCallable, Category = "Biome Zones", CallInEditor = true)
    void CreateBiomeZones(const TArray<FWorld_BiomeZoneConfig>& BiomeConfigs);

    UFUNCTION(BlueprintCallable, Category = "Biome Zones", CallInEditor = true)
    void CreateDefaultBiomeLayout();

    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    EWorld_BiomeZoneType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    // Landscape Validation and Metrics
    UFUNCTION(BlueprintCallable, Category = "Landscape Validation")
    bool ValidateLandscapeSize() const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Validation")
    float GetLandscapeSizeKilometers() const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Validation")
    int32 GetLandscapeActorCount() const;

    // Performance and Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLandscapePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLandscapeLODSettings(int32 MaxLOD, float LODDistanceScale);

protected:
    // Internal helper functions
    TArray<uint16> GenerateHeightmapData(int32 SizeX, int32 SizeY, const FWorld_LandscapeCreationConfig& Config);
    void ApplyBiomeHeightVariation(TArray<uint16>& HeightData, int32 SizeX, int32 SizeY, const FWorld_BiomeZoneConfig& BiomeConfig);
    ALandscape* FindExistingLandscape() const;
    void CleanupOldLandscapes();

private:
    UPROPERTY()
    TArray<FWorld_BiomeZoneConfig> ActiveBiomeZones;

    UPROPERTY()
    ALandscape* MainLandscapeActor;

    UPROPERTY()
    TArray<AActor*> TerrainActors;

    // Configuration cache
    FWorld_LandscapeCreationConfig LastCreationConfig;
    
    // Performance tracking
    float LastCreationTime;
    int32 CreationAttempts;
    bool bLandscapeCreationSuccessful;
};