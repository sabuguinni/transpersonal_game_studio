#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "VFXSystemManager.generated.h"

// ============================================================
// VFX System Manager — Agent #17 VFX
// Prehistoric survival game — realistic VFX only
// NO spiritual/mystical effects
// ============================================================

UENUM(BlueprintType)
enum class EVFX_EmitterType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    DinoImpact      UMETA(DisplayName = "Dinosaur Impact"),
    RainDrop        UMETA(DisplayName = "Rain Drop"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    EmberSpark      UMETA(DisplayName = "Ember Spark")
};

UENUM(BlueprintType)
enum class EVFX_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Ash         UMETA(DisplayName = "Volcanic Ash Fall")
};

UENUM(BlueprintType)
enum class EVFX_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FVFX_EmitterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    EVFX_EmitterType EmitterType = EVFX_EmitterType::Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float SpawnRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float ParticleLifetime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float EmitterRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    FLinearColor ParticleColor = FLinearColor(1.0f, 0.5f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float IntensityScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    EVFX_EmitterType ImpactType = EVFX_EmitterType::FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    float ImpactForce = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    float ImpactRadius = 100.0f;
};

// ============================================================
// AVFX_CampfireEmitter — campfire flame + ember VFX actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "VFX")
class TRANSPERSONALGAME_API AVFX_CampfireEmitter : public AActor
{
    GENERATED_BODY()

public:
    AVFX_CampfireEmitter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SetFireIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void ExtinguishFire();

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void IgniteFire();

    UFUNCTION(BlueprintPure, Category = "VFX|Campfire")
    bool IsFireActive() const { return bFireActive; }

    UFUNCTION(BlueprintPure, Category = "VFX|Campfire")
    float GetCurrentIntensity() const { return CurrentIntensity; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Components",
        meta = (AllowPrivateAccess = "true"))
    UPointLightComponent* FireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* EmitterZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FVFX_EmitterConfig FireConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FlickerSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FlickerAmplitude = 0.15f;

private:
    bool bFireActive = true;
    float CurrentIntensity = 1.0f;
    float FlickerTime = 0.0f;
    float BaseFireLightIntensity = 3000.0f;
};

// ============================================================
// AVFX_FootstepDustEmitter — footstep dust burst on impact
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "VFX")
class TRANSPERSONALGAME_API AVFX_FootstepDustEmitter : public AActor
{
    GENERATED_BODY()

public:
    AVFX_FootstepDustEmitter();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void TriggerFootstepDust(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void SetDinosaurScale(float Scale);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DinosaurMassScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    FVFX_EmitterConfig DustConfig;
};

// ============================================================
// AVFX_WeatherController — rain, fog, ash weather VFX
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "VFX")
class TRANSPERSONALGAME_API AVFX_WeatherController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_WeatherController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(EVFX_WeatherState NewState);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintPure, Category = "VFX|Weather")
    EVFX_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintPure, Category = "VFX|Weather")
    float GetWeatherIntensity() const { return WeatherIntensity; }

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void TransitionToWeather(EVFX_WeatherState TargetState, float TransitionDuration);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    EVFX_WeatherState CurrentWeatherState = EVFX_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float TransitionSpeed = 1.0f;

private:
    EVFX_WeatherState TargetWeatherState = EVFX_WeatherState::Clear;
    float TransitionAlpha = 0.0f;
    bool bTransitioning = false;
    float TransitionDurationSeconds = 5.0f;
};

// ============================================================
// AVFX_ImpactManager — handles all impact VFX spawning
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "VFX")
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void SpawnImpactEffect(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void SpawnBloodEffect(const FVector& Location, const FVector& Normal, float BloodAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void SpawnWeaponImpact(const FVector& Location, const FVector& Normal, float WeaponForce);

    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void SetDangerLevel(EVFX_DangerLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "VFX|Impact")
    EVFX_DangerLevel GetDangerLevel() const { return CurrentDangerLevel; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    EVFX_DangerLevel CurrentDangerLevel = EVFX_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    int32 MaxConcurrentEffects = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
    float EffectPoolRadius = 5000.0f;
};
