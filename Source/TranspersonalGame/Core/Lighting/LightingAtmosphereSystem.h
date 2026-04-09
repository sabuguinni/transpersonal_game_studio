#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "LightingAtmosphereSystem.generated.h"

UENUM(BlueprintType)
enum class EAtmospherePreset : uint8
{
    EarthLike       UMETA(DisplayName = "Earth-like"),
    Alien           UMETA(DisplayName = "Alien"),
    Prehistoric     UMETA(DisplayName = "Prehistoric"),
    Mystical        UMETA(DisplayName = "Mystical"),
    Dangerous       UMETA(DisplayName = "Dangerous"),
    Peaceful        UMETA(DisplayName = "Peaceful")
};

USTRUCT(BlueprintType)
struct FAtmosphereSettings
{
    GENERATED_BODY()

    // Rayleigh Scattering (blue sky)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rayleigh")
    FLinearColor RayleighScattering = FLinearColor(0.005802f, 0.013558f, 0.033100f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rayleigh")
    float RayleighScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rayleigh")
    float RayleighExponentialDistribution = 8.0f;

    // Mie Scattering (haze, pollution)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie")
    FLinearColor MieScattering = FLinearColor(0.003996f, 0.003996f, 0.003996f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie")
    float MieScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie")
    FLinearColor MieAbsorption = FLinearColor(0.000444f, 0.000444f, 0.000444f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie")
    float MieAbsorptionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie")
    float MieAnisotropy = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mie")
    float MieExponentialDistribution = 1.2f;

    // Absorption (ozone layer)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Absorption")
    FLinearColor Absorption = FLinearColor(0.000650f, 0.001881f, 0.000085f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Absorption")
    float AbsorptionScale = 1.0f;

    // Planet properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float GroundRadius = 6360.0f; // km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float AtmosphereHeight = 60.0f; // km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    FLinearColor GroundAlbedo = FLinearColor(0.3f, 0.3f, 0.3f);

    // Art direction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Art Direction")
    float SkyLuminanceFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Art Direction")
    float AerialPerspectiveDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Art Direction")
    float HeightFogContribution = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Art Direction")
    float TransmittanceMinLightElevationAngle = -90.0f;
};

USTRUCT(BlueprintType)
struct FCloudSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coverage")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coverage")
    float CloudType = 0.5f; // 0=cumulus, 1=stratus

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float CloudDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float CloudOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altitude")
    float CloudLayerBottomAltitude = 1.5f; // km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altitude")
    float CloudLayerHeight = 4.0f; // km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector CloudWindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float CloudWindSpeed = 10.0f; // km/h

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float CloudScatteringLuminanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor CloudScatteringLuminanceColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FFogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.638f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    FLinearColor DirectionalInscatteringColor = FLinearColor(1.0f, 0.9f, 0.7f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Directional")
    float DirectionalInscatteringExponent = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Directional")
    float DirectionalInscatteringStartDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
    float FogMaxOpacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
    float StartDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
    float FogCutoffDistance = 0.0f; // 0 = infinite

    // Volumetric fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FLinearColor VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FLinearColor VolumetricFogEmissive = FLinearColor(0.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricFogExtinctionScale = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAtmosphereChanged, EAtmospherePreset, NewPreset);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULightingAtmosphereSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULightingAtmosphereSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EAtmospherePreset CurrentPreset = EAtmospherePreset::EarthLike;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FAtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    FCloudSettings CloudSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FFogSettings FogSettings;

    // Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EAtmospherePreset, FAtmosphereSettings> AtmospherePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EAtmospherePreset, FCloudSettings> CloudPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EAtmospherePreset, FFogSettings> FogPresets;

    // Component references
    UPROPERTY()
    class USkyAtmosphereComponent* SkyAtmosphereComponent;

    UPROPERTY()
    class UVolumetricCloudComponent* VolumetricCloudComponent;

    UPROPERTY()
    class AExponentialHeightFog* HeightFogActor;

    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bSmoothTransitions = true;

    // Internal state
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    FAtmosphereSettings SourceAtmosphere;
    FAtmosphereSettings TargetAtmosphere;
    FCloudSettings SourceClouds;
    FCloudSettings TargetClouds;
    FFogSettings SourceFog;
    FFogSettings TargetFog;

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAtmosphereChanged OnAtmosphereChanged;

    // Public API
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmospherePreset(EAtmospherePreset NewPreset, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetCustomAtmosphereSettings(const FAtmosphereSettings& NewSettings, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Clouds")
    void SetCloudSettings(const FCloudSettings& NewSettings, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void SetFogSettings(const FFogSettings& NewSettings, bool bImmediate = false);

    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    EAtmospherePreset GetCurrentPreset() const { return CurrentPreset; }

    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    FAtmosphereSettings GetCurrentAtmosphereSettings() const { return AtmosphereSettings; }

    UFUNCTION(BlueprintPure, Category = "Clouds")
    FCloudSettings GetCurrentCloudSettings() const { return CloudSettings; }

    UFUNCTION(BlueprintPure, Category = "Fog")
    FFogSettings GetCurrentFogSettings() const { return FogSettings; }

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ForceUpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SaveCurrentSettingsAsPreset(EAtmospherePreset PresetSlot);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ResetToDefaultPresets();

protected:
    // Internal methods
    void InitializeComponents();
    void SetupDefaultPresets();
    void UpdateAtmosphereTransition(float DeltaTime);
    void ApplyAtmosphereSettings(const FAtmosphereSettings& Settings);
    void ApplyCloudSettings(const FCloudSettings& Settings);
    void ApplyFogSettings(const FFogSettings& Settings);

    // Blending utilities
    FAtmosphereSettings BlendAtmosphereSettings(const FAtmosphereSettings& A, const FAtmosphereSettings& B, float Alpha);
    FCloudSettings BlendCloudSettings(const FCloudSettings& A, const FCloudSettings& B, float Alpha);
    FFogSettings BlendFogSettings(const FFogSettings& A, const FFogSettings& B, float Alpha);
    FLinearColor BlendColors(const FLinearColor& A, const FLinearColor& B, float Alpha);
    float BlendFloats(float A, float B, float Alpha);
    FVector BlendVectors(const FVector& A, const FVector& B, float Alpha);

    // Component finding
    void FindOrCreateSkyAtmosphere();
    void FindOrCreateVolumetricClouds();
    void FindOrCreateHeightFog();
};