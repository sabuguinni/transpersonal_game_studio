#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "SharedTypes.h"
#include "EnvArt_VolumetricFogSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_FogDensityLevel : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Light       UMETA(DisplayName = "Light Mist"),
    Medium      UMETA(DisplayName = "Medium Fog"),
    Heavy       UMETA(DisplayName = "Heavy Fog"),
    Dense       UMETA(DisplayName = "Dense Fog")
};

UENUM(BlueprintType)
enum class EEnvArt_FogType : uint8
{
    Valley      UMETA(DisplayName = "Valley Fog"),
    Forest      UMETA(DisplayName = "Forest Mist"),
    River       UMETA(DisplayName = "River Fog"),
    Mountain    UMETA(DisplayName = "Mountain Clouds"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float FogMaxOpacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float StartDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.638f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    FLinearColor DirectionalInscatteringColor = FLinearColor(1.0f, 0.9f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float DirectionalInscatteringExponent = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float DirectionalInscatteringStartDistance = 11000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float VolumetricFogAlbedo = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float VolumetricFogEmissive = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    float VolumetricFogExtinctionScale = 1.0f;

    FEnvArt_FogSettings()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogMaxOpacity = 1.0f;
        StartDistance = 0.0f;
        FogInscatteringColor = FLinearColor(0.447f, 0.638f, 1.0f);
        DirectionalInscatteringColor = FLinearColor(1.0f, 0.9f, 0.8f);
        DirectionalInscatteringExponent = 4.0f;
        DirectionalInscatteringStartDistance = 11000.0f;
        VolumetricFogScatteringDistribution = 0.2f;
        VolumetricFogAlbedo = 0.9f;
        VolumetricFogEmissive = 0.0f;
        VolumetricFogExtinctionScale = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_VolumetricFogSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_VolumetricFogSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Fog Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Control")
    EEnvArt_FogDensityLevel CurrentFogLevel = EEnvArt_FogDensityLevel::Light;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Control")
    EEnvArt_FogType FogType = EEnvArt_FogType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Control")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Control")
    bool bEnableDynamicFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Control")
    float FogTransitionSpeed = 1.0f;

    // Fog Settings for Different Types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FEnvArt_FogSettings ValleyFogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FEnvArt_FogSettings ForestFogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FEnvArt_FogSettings RiverFogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FEnvArt_FogSettings MountainFogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FEnvArt_FogSettings VolcanicFogSettings;

    // Dynamic Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Fog")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Fog")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Fog")
    float FogPulseSpeed = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Fog")
    float FogPulseAmplitude = 0.2f;

    // References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UExponentialHeightFogComponent* FogComponent;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Fog Control")
    void SetFogLevel(EEnvArt_FogDensityLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Fog Control")
    void SetFogType(EEnvArt_FogType NewType);

    UFUNCTION(BlueprintCallable, Category = "Fog Control")
    void ApplyFogSettings(const FEnvArt_FogSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Fog Control")
    void TransitionToFogSettings(const FEnvArt_FogSettings& TargetSettings, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Fog Control")
    FEnvArt_FogSettings GetCurrentFogSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Fog Control")
    void CreateFogActor();

    UFUNCTION(BlueprintCallable, Category = "Fog Control")
    void FindExistingFogActor();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Fog")
    void UpdateDynamicFog(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environmental Storytelling")
    void CreateAtmosphericMood(const FString& MoodName);

private:
    // Internal state
    FEnvArt_FogSettings CurrentSettings;
    FEnvArt_FogSettings TargetSettings;
    bool bIsTransitioning;
    float TransitionProgress;
    float TransitionDuration;
    float FogPulseTime;

    // Helper functions
    FEnvArt_FogSettings GetPresetForType(EEnvArt_FogType Type) const;
    FEnvArt_FogSettings GetSettingsForLevel(EEnvArt_FogDensityLevel Level) const;
    void InitializePresets();
    void LerpFogSettings(const FEnvArt_FogSettings& From, const FEnvArt_FogSettings& To, float Alpha);
};