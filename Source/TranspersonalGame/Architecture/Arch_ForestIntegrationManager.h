#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "Arch_ForestIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ForestStructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float ForestIntegrationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bNaturalCamouflage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<FString> AttachedVegetation;

    FArch_ForestStructureData()
    {
        StructureName = TEXT("DefaultStructure");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        ForestIntegrationLevel = 0.5f;
        bNaturalCamouflage = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_NaturalShelterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EBiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float MinTreeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float MaxSlopeAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bRequireWaterAccess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WindProtectionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    TArray<FString> PreferredMaterials;

    FArch_NaturalShelterConfig()
    {
        TargetBiome = EBiomeType::Forest;
        MinTreeDensity = 0.3f;
        MaxSlopeAngle = 25.0f;
        bRequireWaterAccess = true;
        WindProtectionLevel = 0.7f;
        PreferredMaterials = {TEXT("Wood"), TEXT("Stone"), TEXT("Leaves")};
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_CamouflageSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camouflage")
    float VegetationCoveragePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camouflage")
    bool bUseNaturalColors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camouflage")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camouflage")
    TArray<FLinearColor> NaturalColorPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camouflage")
    float VisibilityFromDistance;

    FArch_CamouflageSettings()
    {
        VegetationCoveragePercent = 60.0f;
        bUseNaturalColors = true;
        WeatheringLevel = 0.8f;
        NaturalColorPalette = {
            FLinearColor(0.4f, 0.3f, 0.2f, 1.0f), // Brown
            FLinearColor(0.2f, 0.4f, 0.1f, 1.0f), // Green
            FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)  // Grey
        };
        VisibilityFromDistance = 50.0f;
    }
};

/**
 * Forest Integration Manager for Architecture Systems
 * Manages how architectural structures blend with Cretaceous forest environments
 * Handles natural camouflage, vegetation integration, and environmental adaptation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_ForestIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_ForestIntegrationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Forest Integration")
    void InitializeForestIntegration();

    UFUNCTION(BlueprintCallable, Category = "Forest Integration")
    bool IntegrateStructureWithForest(const FArch_ForestStructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Forest Integration")
    void ApplyNaturalCamouflage(AActor* StructureActor, const FArch_CamouflageSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Forest Integration")
    TArray<FVector> FindOptimalShelterLocations(const FArch_NaturalShelterConfig& Config, int32 MaxLocations = 10);

    // Vegetation Integration
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void AttachVegetationToStructure(AActor* StructureActor, const TArray<FString>& VegetationTypes);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void CreateVineOvergrowth(AActor* StructureActor, float CoveragePercent);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PlaceMossAndLichen(AActor* StructureActor, float DensityLevel);

    // Environmental Adaptation
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void AdaptToForestLighting(AActor* StructureActor);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyWeatheringEffects(AActor* StructureActor, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float CalculateForestDensityAtLocation(const FVector& Location, float SampleRadius = 1000.0f);

    // Stealth and Visibility
    UFUNCTION(BlueprintCallable, Category = "Stealth")
    float CalculateStructureVisibility(AActor* StructureActor, const FVector& ObserverLocation);

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    void OptimizeForStealth(AActor* StructureActor);

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    bool IsStructureHiddenFromPredators(AActor* StructureActor);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FArch_NaturalShelterConfig DefaultShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FArch_CamouflageSettings DefaultCamouflageSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ForestIntegrationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxVegetationInstancesPerStructure;

    // Runtime Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<FArch_ForestStructureData> IntegratedStructures;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TMap<AActor*, float> StructureVisibilityMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bForestIntegrationActive;

    // Performance Monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 ActiveIntegrationCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastIntegrationTime;

private:
    // Helper Functions
    bool ValidateForestLocation(const FVector& Location, const FArch_NaturalShelterConfig& Config);
    void UpdateStructureVisibility();
    void OptimizePerformance();
    TArray<AActor*> GetNearbyVegetation(const FVector& Location, float Radius);
    void ApplyMaterialWeathering(UStaticMeshComponent* MeshComponent, float WeatheringLevel);
};