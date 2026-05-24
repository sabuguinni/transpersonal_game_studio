#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "../SharedTypes.h"
#include "BiomeAtmosphericLightingSystem.generated.h"

USTRUCT(BlueprintType)
struct FLight_BiomeAtmosphericSettings
{
    GENERATED_BODY()

    // Fog settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor::White;

    // Ambient lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientLightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientLightIntensity = 1.0f;

    // Directional light modulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunlightColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunlightIntensityMultiplier = 1.0f;

    // Atmospheric point lights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FVector> AtmosphericLightPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FLinearColor> AtmosphericLightColors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<float> AtmosphericLightIntensities;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeAtmosphericLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeAtmosphericLightingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core atmospheric settings per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<EBiomeType, FLight_BiomeAtmosphericSettings> BiomeAtmosphericSettings;

    // Current active biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    EBiomeType CurrentBiome = EBiomeType::Savanna;

    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float AtmosphericTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bEnableAtmosphericTransitions = true;

    // References to lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ADirectionalLight> MainSun;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ASkyLight> MainSkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TMap<EBiomeType, TObjectPtr<AExponentialHeightFog>> BiomeFogActors;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<TObjectPtr<APointLight>> AtmosphericPointLights;

    // System methods
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void InitializeBiomeAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetCurrentBiome(EBiomeType NewBiome, bool bInstantTransition = false);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateAtmosphericLighting(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void CreateBiomeFogActors();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void CreateAtmosphericPointLights();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyBiomeAtmosphericSettings(const FLight_BiomeAtmosphericSettings& Settings, float BlendWeight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    FLight_BiomeAtmosphericSettings GetCurrentAtmosphericSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void RefreshLightingReferences();

    // Biome detection
    UFUNCTION(BlueprintCallable, Category = "Biome Detection")
    EBiomeType DetectBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Detection")
    void UpdateBiomeBasedOnPlayerLocation();

private:
    // Internal state
    FLight_BiomeAtmosphericSettings CurrentSettings;
    FLight_BiomeAtmosphericSettings TargetSettings;
    float TransitionProgress = 1.0f;

    // Helper methods
    void InitializeDefaultBiomeSettings();
    void BlendAtmosphericSettings(const FLight_BiomeAtmosphericSettings& From, const FLight_BiomeAtmosphericSettings& To, float Alpha, FLight_BiomeAtmosphericSettings& Result);
    void ApplyFogSettings(const FLight_BiomeAtmosphericSettings& Settings);
    void ApplyAmbientLighting(const FLight_BiomeAtmosphericSettings& Settings);
    void ApplySunlightModulation(const FLight_BiomeAtmosphericSettings& Settings);
    void UpdateAtmosphericPointLights(const FLight_BiomeAtmosphericSettings& Settings);
};