#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/PostProcessVolume.h"
#include "../SharedTypes.h"
#include "BiomeLightingManager.generated.h"

USTRUCT(BlueprintType)
struct FLight_BiomeLightingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieAbsorption = 0.4f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_BiomeLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_BiomeLightingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Componentes principais de iluminação
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDirectionalLightComponent* SunLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkyLightComponent* SkyLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UExponentialHeightFogComponent* MainFogComponent;

    // Configurações por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TMap<EBiomeType, FLight_BiomeLightingConfig> BiomeLightingConfigs;

    // Sistema de ciclo dia/noite
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationInMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    UCurveFloat* SunIntensityCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    UCurveLinearColor* SunColorCurve;

    // Configurações de Lumen
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 MaxLightBounces = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float ReflectionMaxRoughness = 0.8f;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetBiomeLighting(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreateBiomeSpecificLights();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentSunIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FRotator GetCurrentSunRotation() const;

protected:
    // Funções internas
    void InitializeLightingComponents();
    void SetupBiomeConfigs();
    void ApplyLightingConfig(const FLight_BiomeLightingConfig& Config);
    void UpdateSunPosition();
    void UpdateFogSettings();
    void UpdateAtmosphereSettings();

private:
    // Estado interno
    float DayNightTimer = 0.0f;
    EBiomeType CurrentBiome = EBiomeType::Savana;
    bool bLightingInitialized = false;

    // Cache de referências
    TArray<UPointLightComponent*> BiomePointLights;
    APostProcessVolume* GlobalPostProcessVolume;
};