#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Particles/ParticleSystemComponent.h"
#include "AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EAtmosphereType : uint8
{
    SacredDawn      UMETA(DisplayName = "Sacred Dawn"),
    MysticalTwilight UMETA(DisplayName = "Mystical Twilight"),
    UnityRadiance   UMETA(DisplayName = "Unity Radiance"),
    VoidStillness   UMETA(DisplayName = "Void Stillness"),
    CosmicInfinity  UMETA(DisplayName = "Cosmic Infinity")
};

USTRUCT(BlueprintType)
struct FAtmosphereProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    // Sky atmosphere settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HorizonColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AtmosphereHeight = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MieScattering = 0.004f;

    // Cloud settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableClouds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CloudOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor CloudColor = FLinearColor::White;

    // Particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableParticleEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ParticleCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ParticleLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ParticleColor = FLinearColor::White;

    // Dynamic effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBreathingLight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreathingRate = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntensityVariation = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnergyWaves = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaveSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaveFrequency = 0.5f;
};

UCLASS()
class TRANSPERSONALGAME_API AAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    UVolumetricCloudComponent* VolumetricClouds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    UParticleSystemComponent* AtmosphereParticles;

    // Atmosphere profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Profiles")
    TMap<EAtmosphereType, FAtmosphereProfile> AtmosphereProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Profiles")
    EAtmosphereType CurrentAtmosphere = EAtmosphereType::SacredDawn;

    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bIsTransitioning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionProgress = 0.0f;

    // Dynamic effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    float BreathingTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    float WaveTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    TArray<FLinearColor> EnergyWaveColors;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TransitionToAtmosphere(EAtmosphereType NewAtmosphere);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereProfile(EAtmosphereType AtmosphereType, const FAtmosphereProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FAtmosphereProfile GetCurrentAtmosphereProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableDynamicEffects(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBreathingEffect(bool bEnable, float Rate = 0.25f, float Variation = 0.15f);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetEnergyWaveEffect(bool bEnable, float Speed = 2.0f, float Frequency = 0.5f);

private:
    void InitializeAtmosphereProfiles();
    void UpdateTransition(float DeltaTime);
    void ApplyAtmosphereProfile(const FAtmosphereProfile& Profile);
    void InterpolateAtmosphere(const FAtmosphereProfile& From, const FAtmosphereProfile& To, float Alpha);
    void UpdateDynamicEffects(float DeltaTime);
    void UpdateBreathingEffect(float DeltaTime);
    void UpdateEnergyWaves(float DeltaTime);

    FAtmosphereProfile PreviousProfile;
    FAtmosphereProfile TargetProfile;
    float BaseIntensity;
};