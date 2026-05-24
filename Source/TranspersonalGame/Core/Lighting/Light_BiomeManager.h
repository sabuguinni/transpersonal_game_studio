#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PointLightComponent.h"
#include "../SharedTypes.h"
#include "Light_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor DirectionalColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float DirectionalIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Temperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Tint = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Saturation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Contrast = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureCompensation = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Configurações de iluminação por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TMap<EBiomeType, FLight_BiomeSettings> BiomeSettings;

    // Referências para componentes de iluminação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ADirectionalLight* MainDirectionalLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ASkyLight* MainSkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    AExponentialHeightFog* MainFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ASkyAtmosphere* MainAtmosphere;

    // Arrays de luzes pontuais por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lights")
    TMap<EBiomeType, TArray<APointLight*>> BiomePointLights;

    // Arrays de Post Process Volumes por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    TMap<EBiomeType, APostProcessVolume*> BiomePostProcessVolumes;

    // Configuração do ciclo dia/noite
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeOfDay = 12.0f; // 0-24 horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDuration = 1200.0f; // Duração do dia em segundos (20 minutos)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    // Métodos públicos
    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void SetBiomeLighting(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting Setup")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting Setup")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting Setup")
    void CreateBiomePointLights();

    UFUNCTION(BlueprintCallable, Category = "Lighting Setup")
    void CreateBiomePostProcessVolumes();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateDirectionalLight();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateSkyLight();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateAtmosphere();

private:
    void InitializeBiomeSettings();
    FLight_BiomeSettings GetCurrentBiomeSettings();
    float CalculateSunAngle();
    FLinearColor CalculateSunColor(float SunAngle);
    float CalculateSunIntensity(float SunAngle);
};