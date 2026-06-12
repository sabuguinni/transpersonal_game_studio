#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_AtmosphericSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Temperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float BloomIntensity = 0.675f;

    FLight_AtmosphericSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        Temperature = 5500.0f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogInscatteringColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        BloomIntensity = 0.675f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UDirectionalLightComponent* DirectionalLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPostProcessComponent* PostProcess;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLight_AtmosphericSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLight_AtmosphericSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLight_AtmosphericSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLight_AtmosphericSettings NightSettings;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureVolumetricFog();

private:
    void UpdateLighting();
    void InterpolateSettings(const FLight_AtmosphericSettings& SettingsA, const FLight_AtmosphericSettings& SettingsB, float Alpha, FLight_AtmosphericSettings& OutSettings);
    void FindAndConfigureWorldActors();

    UPROPERTY()
    class ADirectionalLight* WorldDirectionalLight;

    UPROPERTY()
    class ASkyAtmosphere* WorldSkyAtmosphere;

    UPROPERTY()
    class AExponentialHeightFog* WorldHeightFog;

    UPROPERTY()
    class APostProcessVolume* WorldPostProcessVolume;
};