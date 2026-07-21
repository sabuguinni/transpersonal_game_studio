#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Light_ShelterLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_ShelterType : uint8
{
    Cave_MainFire        UMETA(DisplayName = "Cave Main Fire"),
    Cave_SecondaryFire   UMETA(DisplayName = "Cave Secondary Fire"),
    Cave_Entrance        UMETA(DisplayName = "Cave Entrance"),
    Forest_Ambient       UMETA(DisplayName = "Forest Ambient"),
    Rock_Overhang        UMETA(DisplayName = "Rock Overhang")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FireLightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float BaseIntensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    FLinearColor FireColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float AttenuationRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float SourceRadius = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FlickerIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Lighting")
    float FlickerSpeed = 2.5f;

    FLight_FireLightConfig()
    {
        BaseIntensity = 800.0f;
        FireColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
        AttenuationRadius = 1200.0f;
        SourceRadius = 25.0f;
        FlickerIntensity = 0.3f;
        FlickerSpeed = 2.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_ShelterLightingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Lighting")
    ELight_ShelterType ShelterType = ELight_ShelterType::Cave_MainFire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Lighting")
    FLight_FireLightConfig FireConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Lighting")
    bool bEnableFireFlicker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Lighting")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Lighting")
    float VolumetricFogDensity = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Lighting")
    FLinearColor VolumetricFogColor = FLinearColor(0.8f, 0.6f, 0.4f, 0.3f);

    FLight_ShelterLightingData()
    {
        ShelterType = ELight_ShelterType::Cave_MainFire;
        bEnableFireFlicker = true;
        bEnableVolumetricFog = true;
        VolumetricFogDensity = 0.15f;
        VolumetricFogColor = FLinearColor(0.8f, 0.6f, 0.4f, 0.3f);
    }
};

/**
 * Prehistoric Shelter Interior Lighting Manager
 * Handles dynamic fire lighting, cave entrance transitions, and atmospheric effects
 * for Stone Age dwelling interiors with realistic fire mechanics integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_ShelterLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_ShelterLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UPointLightComponent* MainFireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UPointLightComponent* SecondaryFireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class USpotLightComponent* EntranceLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class URectLightComponent* AmbientLight;

    // Shelter lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FLight_ShelterLightingData ShelterLightingData;

    // Fire flicker system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Effects")
    bool bEnableFireFlicker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Effects")
    float FlickerTimeAccumulator = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Effects")
    float CurrentFlickerOffset = 0.0f;

    // Atmospheric integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bIntegrateWithWeatherSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WeatherInfluenceStrength = 0.4f;

    // Time of day integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bAdjustForTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DaytimeIntensityMultiplier = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float NighttimeIntensityMultiplier = 1.2f;

public:
    // Lighting control functions
    UFUNCTION(BlueprintCallable, Category = "Shelter Lighting")
    void SetShelterType(ELight_ShelterType NewShelterType);

    UFUNCTION(BlueprintCallable, Category = "Shelter Lighting")
    void UpdateFireLighting(const FLight_FireLightConfig& NewFireConfig);

    UFUNCTION(BlueprintCallable, Category = "Shelter Lighting")
    void SetFireFlickerEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Shelter Lighting")
    void AdjustLightingForTimeOfDay(float TimeOfDayNormalized);

    UFUNCTION(BlueprintCallable, Category = "Shelter Lighting")
    void AdjustLightingForWeather(float WeatherIntensity, const FLinearColor& WeatherTint);

    // Fire mechanics integration
    UFUNCTION(BlueprintCallable, Category = "Fire Integration")
    void OnFireLit();

    UFUNCTION(BlueprintCallable, Category = "Fire Integration")
    void OnFireExtinguished();

    UFUNCTION(BlueprintCallable, Category = "Fire Integration")
    void OnFireIntensityChanged(float NewIntensity);

    // Atmospheric effects
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void EnableVolumetricFog(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void SetVolumetricFogProperties(float Density, const FLinearColor& Color);

protected:
    // Internal lighting update functions
    void UpdateFireFlicker(float DeltaTime);
    void ApplyShelterTypePreset();
    void ConfigureLightingComponents();
    void UpdateAtmosphericEffects();

    // Helper functions
    float CalculateFlickerValue(float Time, float Speed, float Intensity);
    FLinearColor BlendLightColors(const FLinearColor& BaseColor, const FLinearColor& InfluenceColor, float BlendStrength);
};