#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "VFXSystemManager.generated.h"

// ============================================================
// ENUMS — VFX categories (VFX_ prefix to avoid name collision)
// ============================================================

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    DinosaurImpact  UMETA(DisplayName = "DinosaurImpact"),
    Combat          UMETA(DisplayName = "Combat"),
    Weather         UMETA(DisplayName = "Weather"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Ambient         UMETA(DisplayName = "Ambient")
};

UENUM(BlueprintType)
enum class EVFX_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    AshFall     UMETA(DisplayName = "AshFall"),
    Fog         UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class EVFX_DinoSize : uint8
{
    Small   UMETA(DisplayName = "Small"),    // Raptor, Compy
    Medium  UMETA(DisplayName = "Medium"),   // Triceratops, Iguanodon
    Large   UMETA(DisplayName = "Large"),    // T-Rex, Brachiosaurus
    Massive UMETA(DisplayName = "Massive")   // Titanosaur
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FVFX_EmitterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName EmitterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectCategory Category = EVFX_EffectCategory::Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float MaxSpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LifetimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD_Distance_Low = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD_Distance_Off = 5000.0f;
};

USTRUCT(BlueprintType)
struct FVFX_DinoImpactProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_DinoSize Size = EVFX_DinoSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float FootstepDustRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float FootstepDustLifetime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float RoarDistortionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float BreathVaporRadius = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bHasBreathVapor = false;
};

USTRUCT(BlueprintType)
struct FVFX_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_WeatherState State = EVFX_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ParticleSpawnRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float FogDensityTarget = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor FogColorTarget = FLinearColor(0.6f, 0.65f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float TransitionDuration = 5.0f;
};

USTRUCT(BlueprintType)
struct FVFX_CampfireState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float FuelRemaining = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float IntensityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bActive = false;
};

// ============================================================
// UVFX_WeatherController — Component that drives weather VFX
// ============================================================

UCLASS(ClassGroup = "VFX", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_WeatherController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_WeatherController();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(EVFX_WeatherState NewState, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    EVFX_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void TriggerVolcanicAshfall(float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    float GetWeatherTransitionAlpha() const { return WeatherTransitionAlpha; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Weather")
    EVFX_WeatherState CurrentWeatherState = EVFX_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Weather")
    EVFX_WeatherState TargetWeatherState = EVFX_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WeatherTransitionAlpha = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WeatherTransitionSpeed = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float AshfallTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TArray<FVFX_WeatherConfig> WeatherPresets;

private:
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyWeatherToFog(const FVFX_WeatherConfig& Config, float Alpha);
};

// ============================================================
// AVFX_SystemManager — Main VFX manager actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Dinosaur Impact VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnFootstepDust(FVector Location, EVFX_DinoSize DinoSize);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnRoarDistortion(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnBreathVapor(FVector Location, FRotator Direction);

    // --- Combat VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodSplatter(FVector Location, FVector ImpactNormal, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnSpearImpact(FVector Location, FVector SurfaceNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnPlayerDamageFlash(float DamageAmount);

    // --- Environment VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnCampfireEffect(FVector Location, float FuelAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void UpdateCampfireIntensity(int32 CampfireIndex, float NewFuel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnWaterfallSpray(FVector BaseLocation, float FlowRate);

    // --- Crafting VFX ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Crafting")
    void SpawnFlintKnappingSparks(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Crafting")
    void SpawnCookingSmoke(FVector Location, bool bIsActive);

    // --- Weather ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeather(EVFX_WeatherState NewWeather);

    UFUNCTION(BlueprintPure, Category = "VFX|Weather")
    EVFX_WeatherState GetCurrentWeather() const;

    // --- LOD ---
    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    void UpdateVFXLOD(FVector PlayerLocation);

    // --- Registry ---
    UFUNCTION(BlueprintCallable, Category = "VFX|Registry")
    void RegisterDinoImpactProfile(FVFX_DinoImpactProfile Profile);

    UFUNCTION(BlueprintPure, Category = "VFX|Registry")
    bool GetDinoProfile(FName SpeciesName, FVFX_DinoImpactProfile& OutProfile) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Components")
    UVFX_WeatherController* WeatherController;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TArray<FVFX_DinoImpactProfile> DinoImpactProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
    TArray<FVFX_CampfireState> ActiveCampfires;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_HighQualityRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_MediumQualityRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_CullRadius = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Debug")
    bool bDebugVFXSpawnPoints = false;

private:
    void InitializeDefaultDinoProfiles();
    void InitializeWeatherPresets();
    float GetDustScaleForDinoSize(EVFX_DinoSize Size) const;
    void SpawnDebugSphere(FVector Location, float Radius, FColor Color);
};
