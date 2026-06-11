#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "AtmosphericController.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn UMETA(DisplayName = "Dawn"),
    Morning UMETA(DisplayName = "Morning"),
    Noon UMETA(DisplayName = "Noon"),
    Afternoon UMETA(DisplayName = "Afternoon"),
    Dusk UMETA(DisplayName = "Dusk"),
    Night UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float RayleighScatteringScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float MieScatteringScale = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float AerosolDensity = 0.4f;

    FEnvArt_AtmosphericSettings()
    {
        // Default Cretaceous atmospheric values
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphericController : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphericController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === ATMOSPHERIC COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // === LIGHTING REFERENCES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (AllowPrivateAccess = "true"))
    AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere", meta = (AllowPrivateAccess = "true"))
    ASkyAtmosphere* SkyAtmosphere;

    // === TIME SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (AllowPrivateAccess = "true"))
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (AllowPrivateAccess = "true"))
    float TimeOfDayProgress = 0.6f; // 0.0 = start of period, 1.0 = end

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (AllowPrivateAccess = "true"))
    float DayDurationMinutes = 20.0f; // Real-time minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (AllowPrivateAccess = "true"))
    bool bEnableDynamicTimeOfDay = false;

    // === ATMOSPHERIC SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FEnvArt_AtmosphericSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FEnvArt_AtmosphericSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FEnvArt_AtmosphericSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FEnvArt_AtmosphericSettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FEnvArt_AtmosphericSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FEnvArt_AtmosphericSettings NightSettings;

    // === ATMOSPHERIC CONTROL METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay, float Progress = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSunLighting(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericFog(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSkyAtmosphere(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FindAtmosphericActors();

    UFUNCTION(BlueprintCallable, Category = "Time")
    void AdvanceTimeOfDay(float DeltaMinutes);

    UFUNCTION(BlueprintCallable, Category = "Time")
    FEnvArt_AtmosphericSettings GetCurrentAtmosphericSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Time")
    FEnvArt_AtmosphericSettings InterpolateSettings(const FEnvArt_AtmosphericSettings& A, const FEnvArt_AtmosphericSettings& B, float Alpha) const;

private:
    // === INTERNAL STATE ===
    float AccumulatedTime = 0.0f;
    bool bAtmosphericActorsFound = false;

    void InitializeDefaultSettings();
    void ValidateAtmosphericReferences();
    FEnvArt_AtmosphericSettings GetSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay) const;
};