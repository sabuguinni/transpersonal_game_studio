#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_AtmosphericManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunTemperature = 5600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 135.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AerosolDensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector RayleighScattering = FVector(0.33f, 0.135f, 0.058f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float BloomIntensity = 0.8f;

    FLight_AtmosphericPreset()
    {
        // Default constructor with Cretaceous period values
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Lighting components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AExponentialHeightFog* VolumetricFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyAtmosphere* Atmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class APostProcessVolume* PostProcessVolume;

    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeHours = 14.0f; // 2 PM default

    // Atmospheric presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    TMap<ELight_TimeOfDay, FLight_AtmosphericPreset> TimePresets;

    // Cretaceous period settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    bool bUseCretaceousAtmosphere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float AtmosphericDensityMultiplier = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Settings")
    float CO2LevelMultiplier = 4.0f; // Higher CO2 in Cretaceous

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void SetTimeOfDayByHours(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void ApplyAtmosphericPreset(const FLight_AtmosphericPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void InitializeCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void UpdateSunPosition(float TimeHours);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Manager")
    FLight_AtmosphericPreset GetCurrentPreset() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Atmospheric Manager")
    void SetupCretaceousLighting();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Atmospheric Manager")
    void FindLightingActors();

protected:
    void InitializeTimePresets();
    void InterpolatePresets(const FLight_AtmosphericPreset& PresetA, const FLight_AtmosphericPreset& PresetB, float Alpha, FLight_AtmosphericPreset& OutPreset);
    void ApplyCretaceousModifications(FLight_AtmosphericPreset& Preset);

private:
    float LastUpdateTime = 0.0f;
    bool bLightingActorsFound = false;
};