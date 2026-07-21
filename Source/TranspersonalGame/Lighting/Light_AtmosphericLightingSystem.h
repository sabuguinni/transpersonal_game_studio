#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Light_AtmosphericLightingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunAngle = -35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunAzimuth = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHeight = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.008f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_RimLightSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rim Lighting")
    float RimLightIntensity = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rim Lighting")
    FLinearColor RimLightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rim Lighting")
    float RimLightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rim Lighting")
    FVector RimLightOffset = FVector(-800.0f, 400.0f, 600.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericLightingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting System")
    FLight_TimeOfDaySettings TimeOfDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting System")
    FLight_RimLightSettings RimLightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting System")
    bool bEnableDynamicTimeOfDay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting System")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting System")
    bool bEnableAutomaticRimLighting = true;

private:
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    AExponentialHeightFog* AtmosphericFog;

    UPROPERTY()
    ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY()
    TArray<class APointLight*> RimLights;

    float CurrentTimeOfDay = 14.0f; // 2 PM default (late afternoon)

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void ApplyCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(float HourOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void CreateRimLightingForActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void UpdateAtmosphericSettings(const FLight_TimeOfDaySettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting System")
    void FindAndConfigureLightingActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting System")
    void ApplyPrehistoricAtmosphere();

private:
    void UpdateSunPosition();
    void UpdateAtmosphericFog();
    void UpdateSkyAtmosphere();
    void CleanupRimLights();
    FLinearColor CalculateSunColorFromTime(float HourOfDay);
    float CalculateSunIntensityFromTime(float HourOfDay);
};