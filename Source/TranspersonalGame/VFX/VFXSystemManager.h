#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "VFXSystemManager.generated.h"

// ============================================================
// VFX System Manager — Agent #17
// Manages all Niagara particle effects for the prehistoric
// survival game. All effects are physically plausible and
// grounded in real prehistoric world phenomena.
// ============================================================

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),    // Fire, rain, fog, wind
    DinosaurImpact  UMETA(DisplayName = "DinosaurImpact"), // Footsteps, breath, blood
    CombatImpact    UMETA(DisplayName = "CombatImpact"),   // Weapon hits, wounds
    WorldAmbient    UMETA(DisplayName = "WorldAmbient"),   // Volcanic ash, pollen, insects
};

UENUM(BlueprintType)
enum class EVFX_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "LightRain"),
    HeavyRain   UMETA(DisplayName = "HeavyRain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Fog"),
    AshFall     UMETA(DisplayName = "AshFall"),
};

USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName EffectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectCategory Category = EVFX_EffectCategory::Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SpawnRate = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LifetimeSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD_Distance_High = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD_Distance_Medium = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LOD_Distance_Low = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bUseWorldSpaceSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor TintColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FVFX_ActiveEffect
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    FName EffectID;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float SpawnTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    float Duration = -1.0f; // -1 = infinite

    UPROPERTY(BlueprintReadOnly, Category = "VFX")
    bool bIsLooping = false;

    // Raw pointer — no UPROPERTY to avoid cross-module GC issues
    UNiagaraComponent* NiagaraComp = nullptr;
};

UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "VFX System Manager"))
class TRANSPERSONALGAME_API AVFXSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Environment VFX ──────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnCampfireEffect(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SetWeatherState(EVFX_WeatherState NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnRainEffect(FVector Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnFogPatch(FVector Location, float Density = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnVolcanicAsh(FVector Location, float IntensityMultiplier = 1.0f);

    // ── Dinosaur VFX ─────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnFootstepDust(FVector ImpactLocation, float DinoMassKg = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnBreathVapor(FVector MouthLocation, FRotator Direction, float AmbientTempC = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnRoarDistortion(FVector Origin, float Intensity = 1.0f);

    // ── Combat VFX ───────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodImpact(FVector HitLocation, FVector HitNormal, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnWeaponImpact(FVector HitLocation, FVector HitNormal, FName SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnSpearTrail(FVector StartLocation, FVector EndLocation);

    // ── World Ambient VFX ────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "VFX|Ambient")
    void SpawnPollenDrift(FVector Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Ambient")
    void SpawnInsectSwarm(FVector Location, int32 Density = 50);

    UFUNCTION(BlueprintCallable, Category = "VFX|Ambient")
    void SpawnWaterSplash(FVector WaterSurface, float ImpactForce = 1.0f);

    // ── Management ───────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void StopEffect(FName EffectID);

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void SetGlobalVFXQuality(int32 QualityLevel); // 0=Low, 1=Medium, 2=High

    // ── Niagara Asset References ──────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Rain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_BreathVapor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_BloodImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_World_VolcanicAsh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> NS_World_Pollen;

    // ── Config ───────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    int32 MaxConcurrentEffects = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    float GlobalVFXScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bEnableLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    EVFX_WeatherState CurrentWeather = EVFX_WeatherState::Clear;

private:
    UPROPERTY()
    TArray<FVFX_EffectConfig> RegisteredEffects;

    TArray<FVFX_ActiveEffect> ActiveEffects;

    int32 GlobalQualityLevel = 2;

    void CleanupExpiredEffects(float CurrentTime);
    void UpdateLOD(const FVector& CameraLocation);
    FName GenerateEffectID(const FString& Prefix);
    int32 EffectIDCounter = 0;
};
