#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PointLightComponent.h"
#include "VegetationLightingSystem.generated.h"

UENUM(BlueprintType)
enum class EVegetationDensity : uint8
{
    Sparse      UMETA(DisplayName = "Sparse"),
    Medium      UMETA(DisplayName = "Medium"),
    Dense       UMETA(DisplayName = "Dense"),
    VeryDense   UMETA(DisplayName = "Very Dense")
};

USTRUCT(BlueprintType)
struct FVegetationLightingSettings
{
    GENERATED_BODY()

    // Light filtering through canopy
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LightPenetration = 0.3f;
    
    // Shadow density under vegetation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShadowIntensity = 0.8f;
    
    // Ambient light reduction
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientOcclusion = 0.6f;
    
    // Color tint from foliage
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FoliageTint = FLinearColor(0.8f, 1.0f, 0.7f, 1.0f);
    
    // Subsurface scattering intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SubsurfaceScattering = 0.4f;
    
    // Wind movement effect on lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindLightVariation = 0.2f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVegetationLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVegetationLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Vegetation density analysis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Analysis")
    float AnalysisRadius = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Analysis")
    int32 SamplePoints = 16;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation Analysis")
    TArray<TSubclassOf<AStaticMeshActor>> VegetationClasses;

    // Dynamic lighting settings per density
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    TMap<EVegetationDensity, FVegetationLightingSettings> DensitySettings;
    
    // Atmospheric effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableDynamicFog = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensityMultiplier = 1.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricLighting = true;

    // Predator hiding spots
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat System")
    bool bEnableThreatLighting = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat System")
    float ThreatShadowIntensity = 0.9f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat System")
    FLinearColor ThreatShadowTint = FLinearColor(0.7f, 0.6f, 0.6f, 1.0f);

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Vegetation Lighting")
    EVegetationDensity AnalyzeVegetationDensity(FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "Vegetation Lighting")
    void UpdateLightingForDensity(EVegetationDensity Density, FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "Vegetation Lighting")
    float CalculateLightPenetration(FVector StartLocation, FVector EndLocation);
    
    UFUNCTION(BlueprintCallable, Category = "Vegetation Lighting")
    void CreateVolumetricLightShafts(FVector Location, FVector LightDirection);
    
    UFUNCTION(BlueprintPure, Category = "Vegetation Lighting")
    bool IsLocationInDenseVegetation(FVector Location) const;
    
    UFUNCTION(BlueprintCallable, Category = "Threat System")
    void HighlightPotentialThreatAreas(FVector PlayerLocation);

private:
    // Internal tracking
    TArray<FVector> VegetationSamplePoints;
    TMap<FVector, EVegetationDensity> CachedDensityMap;
    
    // Dynamic light components for vegetation effects
    UPROPERTY()
    TArray<UPointLightComponent*> DynamicVegetationLights;
    
    void InitializeVegetationAnalysis();
    void UpdateVegetationCache();
    void CreateDynamicLightShafts(FVector Location, const FVegetationLightingSettings& Settings);
    void UpdateFogDensityForVegetation(EVegetationDensity Density, FVector Location);
    
    // Threat detection
    TArray<FVector> DetectPotentialAmbushSpots(FVector PlayerLocation);
    void ApplyThreatLighting(const TArray<FVector>& ThreatLocations);
};