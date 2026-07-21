#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "EnvArt_AtmosphericLightingSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float RayleighScatteringScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float MieScatteringScale = 1.5f;

    FEnvArt_LightingPreset()
    {
        // Default golden hour preset
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericLightingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingPreset> LightingPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<ADirectionalLight> SunLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<AExponentialHeightFog> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSoftObjectPtr<ASkyAtmosphere> SkyAtmosphereActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bAnimateLighting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float LightingTransitionSpeed = 1.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FEnvArt_LightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindAndAssignLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreateDefaultLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FEnvArt_LightingPreset GetCurrentLightingPreset() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ApplyMorningLighting();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ApplyNightLighting();

protected:
    void InitializeLightingPresets();
    void UpdateSunLighting(const FEnvArt_LightingPreset& Preset);
    void UpdateFogSettings(const FEnvArt_LightingPreset& Preset);
    void UpdateSkyAtmosphere(const FEnvArt_LightingPreset& Preset);

private:
    FEnvArt_LightingPreset CurrentPreset;
    FEnvArt_LightingPreset TargetPreset;
    float TransitionProgress = 0.0f;
    bool bIsTransitioning = false;
};