#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "Light_VolumetricLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Dusty       UMETA(DisplayName = "Dusty"),
    Humid       UMETA(DisplayName = "Humid")
};

USTRUCT(BlueprintType)
struct FLight_VolumetricSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float HeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FColor InscatteringColor = FColor(180, 200, 255, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float ScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FColor Albedo = FColor(200, 220, 255, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bVolumetricFogEnabled = true;
};

USTRUCT(BlueprintType)
struct FLight_CaveLightData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float Intensity = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FColor LightColor = FColor(255, 180, 120, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float AttenuationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float VolumetricScatteringIntensity = 2.0f;
};

/**
 * Manages volumetric lighting effects, atmospheric fog, and cave lighting systems
 * Provides persistent atmosphere fixes and dynamic weather lighting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolumetricLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolumetricLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === CORE COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // === VOLUMETRIC SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Lighting")
    FLight_VolumetricSettings VolumetricSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    TArray<FLight_CaveLightData> CaveLightConfigurations;

    // === ATMOSPHERE PERSISTENCE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bFixPersistentAtmosphere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FColor CretaceousLightColor = FColor(255, 240, 200, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float CretaceousLightIntensity = 5.0f;

    // === LUMEN OPTIMIZATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 MaxLightBounces = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 FinalGatherQuality = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    int32 VolumetricFogGridPixelSize = 8;

public:
    // === LIGHTING MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Volumetric Lighting")
    void ApplyVolumetricSettings(const FLight_VolumetricSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupCaveLighting(const TArray<FLight_CaveLightData>& CaveConfigs);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FixPersistentAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void OptimizeLumenSettings();

    // === DYNAMIC EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void UpdateVolumetricFogForWeather();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void AnimateCaveLightFlicker(float FlickerIntensity = 0.1f);

    // === PERSISTENCE ===
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Persistence")
    void SaveLightingConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateVolumetricLightingSetup();

private:
    // === INTERNAL MANAGEMENT ===
    void InitializeVolumetricFog();
    void CleanupOldAtmosphereComponents();
    void ConfigureDirectionalLight();
    void SpawnCaveLights();
    void ApplyLumenConsoleCommands();

    // === RUNTIME TRACKING ===
    UPROPERTY()
    TArray<class APointLight*> SpawnedCaveLights;

    UPROPERTY()
    class AExponentialHeightFog* VolumetricFogActor;

    UPROPERTY()
    class ADirectionalLight* MainDirectionalLight;

    float WeatherTransitionTimer = 0.0f;
    float CaveFlickerTimer = 0.0f;
};