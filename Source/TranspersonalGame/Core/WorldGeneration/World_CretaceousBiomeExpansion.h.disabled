#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_CretaceousBiomeExpansion.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FLinearColor BiomeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    bool bIsActive;

    FWorld_BiomeZoneData()
    {
        BiomeName = TEXT("DefaultBiome");
        ZoneLocation = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        BiomeType = EBiomeType::Forest;
        BiomeColor = FLinearColor::White;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_CretaceousLandscapeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentSizeQuads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 SectionsPerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float HeightmapScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    bool bCreateCollision;

    FWorld_CretaceousLandscapeConfig()
    {
        LandscapeSize = FVector(10000, 10000, 1000);
        ComponentSizeQuads = 63;
        SectionsPerComponent = 2;
        HeightmapScale = 100.0f;
        bCreateCollision = true;
    }
};

/**
 * Cretaceous Biome Expansion Manager
 * Handles the creation and management of the 10km x 10km landscape with 5 distinct biomes
 * Required for asset purchase criteria fulfillment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CretaceousBiomeExpansion : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CretaceousBiomeExpansion();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core landscape management
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Landscape Creation")
    void CreateCretaceousLandscape();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Management")
    void CreateBiomeZones();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Water Systems")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool ValidateLandscapeSize();

    // Biome zone management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void AddBiomeZone(const FWorld_BiomeZoneData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RemoveBiomeZone(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeZoneData GetBiomeZoneData(const FString& BiomeName);

    // Landscape expansion
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void ExpandLandscapeToSize(const FVector& TargetSize);

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void SetLandscapeConfiguration(const FWorld_CretaceousLandscapeConfig& Config);

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Landscape configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Configuration")
    FWorld_CretaceousLandscapeConfig LandscapeConfig;

    // Biome zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeZoneData> BiomeZones;

    // Landscape reference
    UPROPERTY(BlueprintReadOnly, Category = "Landscape")
    class ALandscape* MainLandscape;

    // Biome markers
    UPROPERTY(BlueprintReadOnly, Category = "Biome Markers")
    TArray<class APointLight*> BiomeMarkers;

    // Water bodies
    UPROPERTY(BlueprintReadOnly, Category = "Water Bodies")
    TArray<class AStaticMeshActor*> WaterBodies;

private:
    // Internal landscape creation
    void CreateLandscapeInternal();
    void SetupBiomeMarkersInternal();
    void CreateWaterBodiesInternal();
    
    // Validation
    bool ValidateWorldSize() const;
    bool ValidateBiomeConfiguration() const;
    
    // Purchase criteria validation
    bool MeetsPurchaseCriteria() const;
    void LogPurchaseCriteriaStatus() const;
};