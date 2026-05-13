#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "World_LandscapeCreationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString Description;

    FWorld_BiomeZoneDefinition()
    {
        BiomeName = TEXT("DefaultBiome");
        Location = FVector::ZeroVector;
        BiomeColor = FLinearColor::White;
        BiomeRadius = 2500.0f;
        BiomeType = EBiomeType::Temperate;
        Description = TEXT("Default biome zone");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandscapeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 QuadsPerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float QuadSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FString LandscapeName;

    FWorld_LandscapeConfiguration()
    {
        LandscapeSize = FVector(1000000, 1000000, 1000); // 10km x 10km
        ComponentCountX = 8;
        ComponentCountY = 8;
        QuadsPerComponent = 63;
        QuadSize = 200.0f; // 2m per quad for 10km total
        LandscapeName = TEXT("MainLandscape_Cretaceous");
    }
};

/**
 * World Landscape Creation Manager
 * Handles creation and management of massive landscapes for the Cretaceous world
 * Implements the critical landscape creation requirements for asset purchase criteria
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_LandscapeCreationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UWorld_LandscapeCreationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core landscape creation functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool CreateMainLandscape(const FWorld_LandscapeConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool CreateBiomeZones(const TArray<FWorld_BiomeZoneDefinition>& BiomeZones);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    ALandscape* GetMainLandscape() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<AActor*> GetBiomeMarkers() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool ValidateLandscapeRequirements() const;

    // Biome zone management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeZoneDefinition GetBiomeZoneAt(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const;

    // Asset purchase criteria validation
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool MeetsAssetPurchaseCriteria() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FString GetLandscapeStatusReport() const;

protected:
    // Landscape management
    UPROPERTY(BlueprintReadOnly, Category = "Landscape", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<ALandscape> MainLandscape;

    UPROPERTY(BlueprintReadOnly, Category = "Landscape", meta = (AllowPrivateAccess = "true"))
    FWorld_LandscapeConfiguration CurrentLandscapeConfig;

    // Biome zone management
    UPROPERTY(BlueprintReadOnly, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeZoneDefinition> BiomeZones;

    UPROPERTY(BlueprintReadOnly, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    TArray<TSoftObjectPtr<AActor>> BiomeMarkerActors;

    UPROPERTY(BlueprintReadOnly, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    TArray<TSoftObjectPtr<ATriggerVolume>> BiomeTriggerVolumes;

    // Internal helper functions
    ALandscape* CreateLandscapeActor(const FWorld_LandscapeConfiguration& Config);
    AActor* CreateBiomeMarker(const FWorld_BiomeZoneDefinition& BiomeZone);
    ATriggerVolume* CreateBiomeTrigger(const FWorld_BiomeZoneDefinition& BiomeZone);
    
    void InitializeDefaultBiomeZones();
    bool ValidateLandscapeDimensions(ALandscape* Landscape) const;
    bool ValidateBiomeGeographicSeparation() const;

private:
    // Status tracking
    bool bLandscapeCreated;
    bool bBiomeZonesCreated;
    bool bMeetsAssetCriteria;
    
    // Constants for asset purchase criteria
    static constexpr float MIN_LANDSCAPE_SIZE = 1000000.0f; // 10km in cm
    static constexpr int32 REQUIRED_BIOME_COUNT = 5;
    static constexpr float MIN_BIOME_SEPARATION = 2000.0f; // 20m minimum separation
};