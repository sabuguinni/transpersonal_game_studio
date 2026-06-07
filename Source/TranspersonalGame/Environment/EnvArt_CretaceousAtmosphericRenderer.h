#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Materials/MaterialParameterCollection.h"
#include "EnvArt_CretaceousAtmosphericRenderer.generated.h"

/**
 * Cretaceous Atmospheric Renderer
 * Manages atmospheric lighting, volumetric fog, and environmental effects
 * for authentic Cretaceous period atmosphere with performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousAtmosphericRenderer : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousAtmosphericRenderer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Atmospheric Lighting Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Lighting")
    FLinearColor GoldenHourTint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Lighting")
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Lighting")
    bool bEnableDynamicTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Lighting")
    float AtmosphericTransitionSpeed;

    // Volumetric Fog Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    bool bEnableVolumetricFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float AtmosphericDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float FogHeightFalloff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FLinearColor FogInscatteringColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    bool bEnableDynamicAtmosphere;

    // Performance Optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bOptimizeForPerformance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance;

    // Material Parameter Collection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialParameterCollection* AtmosphericMaterialCollection;

    // Actor References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* AtmosphericFog;

private:
    // Initialization Methods
    void InitializeAtmosphericLighting();
    void InitializeVolumetricFog();
    void SetupMaterialParameterCollection();

    // Update Methods
    void UpdateAtmosphericParameters(float DeltaTime);
    void UpdateLightingTransition(float DeltaTime);
    void UpdateFogDensity(float DeltaTime);

    // Performance Optimization
    void OptimizeAtmosphericRendering();

public:
    // Blueprint Callable Functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetAtmosphericDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void EnableGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableVolumetricFog(bool bEnable) { bEnableVolumetricFog = bEnable; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceOptimization(bool bEnable) { bOptimizeForPerformance = bEnable; }

    // Getters
    UFUNCTION(BlueprintPure, Category = "Atmospheric State")
    float GetCurrentTimeOfDay() const { return TimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric State")
    float GetAtmosphericDensity() const { return AtmosphericDensity; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric State")
    FLinearColor GetCurrentSunColor() const { return GoldenHourTint; }
};