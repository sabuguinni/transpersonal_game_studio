#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EnvArt_CretaceousRockFormationLibrary.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_RockFormationType : uint8
{
    WeatheredOutcrop     UMETA(DisplayName = "Weathered Outcrop"),
    MossyCoveredBoulder  UMETA(DisplayName = "Mossy Covered Boulder"),
    StratifiedCliff      UMETA(DisplayName = "Stratified Cliff"),
    ErodedPillar         UMETA(DisplayName = "Eroded Pillar"),
    CaveEntrance         UMETA(DisplayName = "Cave Entrance"),
    RiverRock            UMETA(DisplayName = "River Rock"),
    VolcanicRock         UMETA(DisplayName = "Volcanic Rock")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_RockFormationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    FString FormationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    EEnvArt_RockFormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    FVector BaseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float MossCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    float WeatheringIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    bool bHasVegetationGrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation")
    FLinearColor BaseRockColor;

    FEnvArt_RockFormationData()
    {
        FormationName = TEXT("Default Rock Formation");
        FormationType = EEnvArt_RockFormationType::WeatheredOutcrop;
        BaseScale = FVector(1.0f, 1.0f, 1.0f);
        MossCoverage = 0.3f;
        WeatheringIntensity = 0.5f;
        bHasVegetationGrowth = true;
        BaseRockColor = FLinearColor(0.4f, 0.35f, 0.3f, 1.0f);
    }
};

/**
 * Cretaceous Rock Formation Library - Environmental Asset Management
 * Manages procedural rock formations for authentic Cretaceous period environments
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousRockFormationLibrary : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousRockFormationLibrary();

protected:
    virtual void BeginPlay() override;

public:
    // Core rock formation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation Config")
    FEnvArt_RockFormationData FormationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation Config")
    TArray<UStaticMesh*> RockMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Formation Config")
    TArray<UMaterialInterface*> RockMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    bool bIsLandmark;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    FString StorytellingDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    bool bHasDinosaurMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    bool bIsClimbable;

    // Atmospheric integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    bool bCastsDramaticShadows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    float ShadowIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    bool bInteractsWithFog;

    // Performance optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDynamicLOD;

public:
    // Rock formation management functions
    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void InitializeRockFormation(const FEnvArt_RockFormationData& InFormationData);

    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void ApplyWeatheringEffects(float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void SetMossCoverage(float MossLevel);

    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void AddVegetationGrowth();

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void SetAsLandmark(const FString& LandmarkName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void AddDinosaurMarkings(bool bScratchMarks, bool bNestingSite);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Integration")
    void ConfigureAtmosphericInteraction(bool bEnableFogInteraction, float ShadowMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForDistance(float ViewDistance);

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Rock Formation")
    FVector GetFormationBounds() const;

    UFUNCTION(BlueprintPure, Category = "Rock Formation")
    bool IsFormationStable() const;

    UFUNCTION(BlueprintCallable, Category = "Rock Formation")
    void RandomizeFormationAppearance();

protected:
    // Internal utility functions
    void UpdateMaterialParameters();
    void ConfigureLODSettings();
    void SetupCollisionGeometry();
    void ApplyAtmosphericShaderParameters();

private:
    // Internal state tracking
    bool bIsInitialized;
    float CurrentWeatheringLevel;
    float CurrentMossLevel;
    int32 VegetationComplexity;
};