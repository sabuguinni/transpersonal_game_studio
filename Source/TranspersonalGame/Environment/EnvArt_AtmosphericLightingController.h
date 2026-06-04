#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericLightingController.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
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
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    FEnvArt_LightingPreset()
    {
        // Default golden hour preset
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericLightingController : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericLightingController();

protected:
    virtual void BeginPlay() override;

    // === LIGHTING COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // === LIGHTING REFERENCES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* MainSunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyLight* SkyLightActor;

    // === TIME OF DAY SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 16.0f; // 4 PM golden hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    EEnvArt_TimeOfDay CurrentTimePreset = EEnvArt_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoUpdateLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeUpdateSpeed = 1.0f;

    // === LIGHTING PRESETS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingPreset> LightingPresets;

    // === ATMOSPHERIC EFFECTS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableAtmosphericScattering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericHazeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor AtmosphericTint = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

public:
    // === LIGHTING CONTROL FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimePreset(EEnvArt_TimeOfDay NewPreset);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyLightingPreset(const FEnvArt_LightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateSunLighting(const FEnvArt_LightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateFogSettings(const FEnvArt_LightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateSkyLighting(const FEnvArt_LightingPreset& Preset);

    // === ATMOSPHERIC FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableGoldenHourAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphericHaze(float Intensity, FLinearColor Tint);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ToggleVolumetricFog(bool bEnable);

    // === REFERENCE FINDING ===
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void InitializeLightingPresets();

    // === EDITOR FUNCTIONS ===
    UFUNCTION(CallInEditor, Category = "Editor")
    void ApplyGoldenHourPreset();

    UFUNCTION(CallInEditor, Category = "Editor")
    void RefreshLightingReferences();

public:
    virtual void Tick(float DeltaTime) override;

private:
    void UpdateLightingBasedOnTime();
    FEnvArt_LightingPreset InterpolateLightingPresets(const FEnvArt_LightingPreset& A, const FEnvArt_LightingPreset& B, float Alpha);
};