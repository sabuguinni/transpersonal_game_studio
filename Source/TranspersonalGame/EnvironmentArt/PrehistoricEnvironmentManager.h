#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "../SharedTypes.h"
#include "PrehistoricEnvironmentManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeVegetationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<class UStaticMesh*> TreeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<class UStaticMesh*> ShrubMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<class UStaticMesh*> GrassMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float ScaleVariation = 0.2f;

    FEnvArt_BiomeVegetationData()
    {
        VegetationDensity = 1.0f;
        ScaleVariation = 0.2f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_EnvironmentProps
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<class UStaticMesh*> RockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<class UStaticMesh*> FallenLogMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<class UStaticMesh*> BoneMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    float PropDensity = 0.5f;

    FEnvArt_EnvironmentProps()
    {
        PropDensity = 0.5f;
    }
};

/**
 * Manages prehistoric environment art placement and biome-specific vegetation
 * Handles the visual storytelling through environmental details
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricEnvironmentManager : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricEnvironmentManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core environment data per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FEnvArt_BiomeVegetationData> BiomeVegetationMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FEnvArt_EnvironmentProps> BiomePropsMap;

    // Instanced mesh components for performance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TArray<class UInstancedStaticMeshComponent*> VegetationComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TArray<class UInstancedStaticMeshComponent*> PropComponents;

    // Environment storytelling parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    float NarrativeDetailDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (AllowPrivateAccess = "true"))
    bool bEnableEnvironmentalStorytelling = true;

    // Material management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    class UMaterialInterface* TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, class UMaterialInterface*> BiomeMaterials;

public:
    // Environment population functions
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateBiomeVegetation(EBiomeType BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PlaceEnvironmentalProps(EBiomeType BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void CreateNarrativeDetails(const FVector& Location, const FString& StoryContext);

    // Biome transition management
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void BlendBiomeVegetation(EBiomeType FromBiome, EBiomeType ToBiome, const FVector& TransitionCenter, float BlendRadius);

    // Material application
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyBiomeMaterials(EBiomeType BiomeType, class ALandscape* TargetLandscape);

    // Environmental storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceStorytellingElements(const FVector& Location, const FString& NarrativeHint);

    // Cleanup and optimization
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void OptimizeVegetationLODs();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ClearEnvironmentArea(const FVector& Center, float Radius);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugShowBiomeData();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateEnvironmentSetup();

private:
    // Internal helper functions
    void InitializeBiomeData();
    void SetupInstancedComponents();
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    FRotator GetRandomRotation();
    FVector GetRandomScale(float BaseScale, float Variation);
    bool IsValidPlacementLocation(const FVector& Location);
    void PlaceNarrativeDetail(const FVector& Location, const FString& DetailType);
};