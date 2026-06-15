#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_WeatheredBoulderAsset.generated.h"

/**
 * Weathered Boulder Asset System for Cretaceous Environment
 * Creates realistic prehistoric rock formations with dynamic weathering states
 * Supports moss growth, lichen patterns, and environmental storytelling
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_WeatheredBoulderAsset : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AEnvArt_WeatheredBoulderAsset();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === BOULDER PROPERTIES ===
    
    /** Current weathering state of the boulder */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    EEnvArt_WeatheringState WeatheringState;

    /** Size category of the boulder */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    EEnvArt_BoulderSize BoulderSize;

    /** Rock type affecting appearance and properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    EEnvArt_RockType RockType;

    /** Moss coverage percentage (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MossCoverage;

    /** Lichen growth intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LichenIntensity;

    /** Age of the boulder in years (affects weathering) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boulder Properties")
    int32 BoulderAge;

    // === ENVIRONMENTAL STORYTELLING ===
    
    /** Whether this boulder has ancient markings or scratches */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    bool bHasAncientMarkings;

    /** Whether animals use this boulder for territory marking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    bool bIsAnimalTerritory;

    /** Whether this boulder provides shelter opportunities */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    bool bProvidesShel ter;

    /** Narrative context for this specific boulder */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Storytelling")
    FString NarrativeContext;

    // === MATERIAL SYSTEM ===
    
    /** Base rock material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* BaseMaterial;

    /** Moss overlay material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossMaterial;

    /** Lichen overlay material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* LichenMaterial;

    /** Weathered surface material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredMaterial;

    // === MESH VARIANTS ===
    
    /** Array of mesh variants for different boulder shapes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Variants")
    TArray<UStaticMesh*> BoulderMeshVariants;

    /** Current mesh variant index */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Variants")
    int32 CurrentMeshVariant;

    // === GAMEPLAY INTEGRATION ===
    
    /** Whether players can climb this boulder */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bIsClimbable;

    /** Whether this boulder can be used for crafting materials */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bCanHarvestMaterials;

    /** Stone material yield when harvested */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 StoneYield;

    // === FUNCTIONS ===
    
    /** Initialize boulder with random properties */
    UFUNCTION(BlueprintCallable, Category = "Boulder System")
    void InitializeRandomBoulder();

    /** Apply weathering state to materials and appearance */
    UFUNCTION(BlueprintCallable, Category = "Boulder System")
    void ApplyWeatheringState();

    /** Update moss and lichen growth based on environmental factors */
    UFUNCTION(BlueprintCallable, Category = "Boulder System")
    void UpdateOrganicGrowth(float Humidity, float Temperature, float TimeMultiplier);

    /** Set boulder size and scale appropriately */
    UFUNCTION(BlueprintCallable, Category = "Boulder System")
    void SetBoulderSize(EEnvArt_BoulderSize NewSize);

    /** Apply rock type specific properties */
    UFUNCTION(BlueprintCallable, Category = "Boulder System")
    void ApplyRockTypeProperties();

    /** Get environmental story description */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environmental Storytelling")
    FString GetEnvironmentalStory() const;

    /** Check if boulder provides specific gameplay function */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gameplay")
    bool CanProvideFunction(EEnvArt_BoulderFunction Function) const;

protected:
    /** Internal function to randomize mesh selection */
    void SelectRandomMeshVariant();

    /** Internal function to calculate weathering effects */
    void CalculateWeatheringEffects();

    /** Internal function to apply material blending */
    void BlendMaterials();

private:
    /** Timer for organic growth updates */
    float OrganicGrowthTimer;

    /** Cached environmental factors */
    float CachedHumidity;
    float CachedTemperature;
};