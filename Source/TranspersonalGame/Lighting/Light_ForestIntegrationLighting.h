#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/Color.h"
#include "SharedTypes.h"
#include "Light_ForestIntegrationLighting.generated.h"

// Forest lighting time periods for atmospheric changes
UENUM(BlueprintType)
enum class ELight_ForestTimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"), 
    Midday     UMETA(DisplayName = "Midday"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night")
};

// Forest canopy density levels affecting light penetration
UENUM(BlueprintType)
enum class ELight_CanopyDensity : uint8
{
    Sparse     UMETA(DisplayName = "Sparse Canopy"),
    Medium     UMETA(DisplayName = "Medium Canopy"),
    Dense      UMETA(DisplayName = "Dense Canopy"),
    Thick      UMETA(DisplayName = "Thick Canopy")
};

// Forest structure lighting configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_ForestStructureLighting
{
    GENERATED_BODY()

    // Interior lighting intensity for shelters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Lighting")
    float InteriorLightIntensity = 800.0f;

    // Attenuation radius for structure lights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Lighting")
    float AttenuationRadius = 1500.0f;

    // Light color for warm shelter ambiance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Lighting")
    FLinearColor ShelterLightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    // Enable shadow casting from structure lights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Lighting")
    bool bCastShadows = true;

    // Use inverse squared falloff for realistic lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Lighting")
    bool bUseInverseSquaredFalloff = true;

    FLight_ForestStructureLighting()
    {
        InteriorLightIntensity = 800.0f;
        AttenuationRadius = 1500.0f;
        ShelterLightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        bCastShadows = true;
        bUseInverseSquaredFalloff = true;
    }
};

// Forest atmospheric lighting settings
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_ForestAtmosphere
{
    GENERATED_BODY()

    // Volumetric fog density for forest depth
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogDensity = 0.05f;

    // Volumetric scattering intensity for god rays
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricScatteringIntensity = 1.5f;

    // Light shaft bloom scale for forest rays
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float LightShaftBloomScale = 0.8f;

    // Forest undercanopy sky light intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float UndercanopyIntensity = 0.3f;

    // Forest floor ambient color (green tint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor ForestAmbientColor = FLinearColor(0.6f, 0.8f, 0.4f, 1.0f);

    FLight_ForestAtmosphere()
    {
        VolumetricFogDensity = 0.05f;
        VolumetricScatteringIntensity = 1.5f;
        LightShaftBloomScale = 0.8f;
        UndercanopyIntensity = 0.3f;
        ForestAmbientColor = FLinearColor(0.6f, 0.8f, 0.4f, 1.0f);
    }
};

// Forest mood lighting configuration for different times
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_ForestMoodLighting
{
    GENERATED_BODY()

    // Dawn lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood Lighting")
    FLinearColor DawnColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood Lighting")
    float DawnIntensity = 1200.0f;

    // Midday lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood Lighting")
    FLinearColor MiddayColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood Lighting")
    float MiddayIntensity = 2000.0f;

    // Dusk lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood Lighting")
    FLinearColor DuskColor = FLinearColor(0.8f, 0.4f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood Lighting")
    float DuskIntensity = 800.0f;

    FLight_ForestMoodLighting()
    {
        DawnColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
        DawnIntensity = 1200.0f;
        MiddayColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        MiddayIntensity = 2000.0f;
        DuskColor = FLinearColor(0.8f, 0.4f, 0.9f, 1.0f);
        DuskIntensity = 800.0f;
    }
};

/**
 * Forest Integration Lighting System
 * 
 * Manages dynamic lighting for forest environments and architectural structures
 * that blend seamlessly with the Cretaceous forest ecosystem. Provides
 * atmospheric lighting, canopy filtering, volumetric effects, and structure
 * integration lighting that enhances the natural camouflage and mood.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_ForestIntegrationLighting : public AActor
{
    GENERATED_BODY()

public:
    ALight_ForestIntegrationLighting();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE LIGHTING MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Forest Lighting")
    void InitializeForestLighting();

    UFUNCTION(BlueprintCallable, Category = "Forest Lighting")
    void UpdateTimeOfDay(ELight_ForestTimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Forest Lighting")
    void SetCanopyDensity(ELight_CanopyDensity Density);

    // === STRUCTURE INTEGRATION ===

    UFUNCTION(BlueprintCallable, Category = "Structure Lighting")
    void ConfigureStructureLighting(AActor* Structure, const FLight_ForestStructureLighting& Config);

    UFUNCTION(BlueprintCallable, Category = "Structure Lighting")
    void CreateInteriorLighting(const FVector& Location, const FString& StructureName);

    UFUNCTION(BlueprintCallable, Category = "Structure Lighting")
    void UpdateStructureVisibility(AActor* Structure, float VisibilityFactor);

    // === ATMOSPHERIC EFFECTS ===

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureVolumetricFog(const FLight_ForestAtmosphere& AtmosphereSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateCanopyLighting(float SunAngle, float CanopyDensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateGodRayEffects(const FVector& SunDirection);

    // === MOOD LIGHTING ===

    UFUNCTION(BlueprintCallable, Category = "Mood Lighting")
    void ApplyMoodLighting(ELight_ForestTimeOfDay TimeOfDay, const FLight_ForestMoodLighting& MoodConfig);

    UFUNCTION(BlueprintCallable, Category = "Mood Lighting")
    void TransitionToTimeOfDay(ELight_ForestTimeOfDay TargetTime, float TransitionDuration);

    // === LUMEN INTEGRATION ===

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void ConfigureLumenForForest();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void OptimizeLumenForVegetation();

    // === PROPERTIES ===

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    ELight_ForestTimeOfDay CurrentTimeOfDay = ELight_ForestTimeOfDay::Midday;

    // Current canopy density
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    ELight_CanopyDensity CurrentCanopyDensity = ELight_CanopyDensity::Medium;

    // Structure lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Lighting")
    FLight_ForestStructureLighting StructureLightingConfig;

    // Atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_ForestAtmosphere AtmosphereConfig;

    // Mood lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mood Lighting")
    FLight_ForestMoodLighting MoodLightingConfig;

    // Enable automatic time progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    bool bAutoProgressTime = true;

    // Time progression speed (hours per minute)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    float TimeProgressionSpeed = 24.0f;

    // Enable dynamic canopy response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    bool bDynamicCanopyResponse = true;

    // Maximum number of structure lights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxStructureLights = 20;

private:
    // === INTERNAL STATE ===

    // Current time accumulator
    float TimeAccumulator = 0.0f;

    // Active structure lights
    UPROPERTY()
    TArray<APointLight*> StructureLights;

    // Main directional light reference
    UPROPERTY()
    ADirectionalLight* MainDirectionalLight;

    // Sky light reference
    UPROPERTY()
    ASkyLight* ForestSkyLight;

    // Atmospheric fog reference
    UPROPERTY()
    AAtmosphericFog* ForestAtmosphericFog;

    // Mood lighting actors
    UPROPERTY()
    TMap<ELight_ForestTimeOfDay, APointLight*> MoodLights;

    // === INTERNAL METHODS ===

    void FindOrCreateLightingActors();
    void UpdateDirectionalLight();
    void UpdateSkyLight();
    void UpdateAtmosphericFog();
    void CleanupExcessLights();
    float CalculateCanopyLightPenetration(float SunAngle) const;
    FLinearColor GetTimeOfDayColor(ELight_ForestTimeOfDay TimeOfDay) const;
    float GetTimeOfDayIntensity(ELight_ForestTimeOfDay TimeOfDay) const;
};