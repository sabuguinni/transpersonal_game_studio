#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "VFXManager.generated.h"

// === VFX CATEGORY ENUM ===
UENUM(BlueprintType)
enum class EVFX_Category : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Combat          UMETA(DisplayName = "Combat"),
    Weather         UMETA(DisplayName = "Weather"),
    World           UMETA(DisplayName = "World"),
    Crafting        UMETA(DisplayName = "Crafting"),
};

// === VFX LOD LEVEL ===
UENUM(BlueprintType)
enum class EVFX_LODLevel : uint8
{
    High    UMETA(DisplayName = "High (0-15m)"),
    Medium  UMETA(DisplayName = "Medium (15-40m)"),
    Low     UMETA(DisplayName = "Low (40-80m)"),
    Culled  UMETA(DisplayName = "Culled (>80m)"),
};

// === VFX SYSTEM ENTRY ===
USTRUCT(BlueprintType)
struct FVFX_SystemEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_Category Category = EVFX_Category::Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float CullDistanceHigh = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float CullDistanceMedium = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float CullDistanceLow = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultScale = 1.0f;
};

// === VFX SPAWN PARAMS ===
USTRUCT(BlueprintType)
struct FVFX_SpawnParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    AActor* AttachTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName AttachSocketName = NAME_None;
};

/**
 * UVFX_Manager — Central VFX registry and spawn controller.
 * Manages all Niagara particle systems for the prehistoric survival game.
 * Categories: Environment (campfire, embers), Dinosaur (footstep dust, breath vapor, roar distortion),
 *             Combat (blood impact, weapon impact), Weather (rain, snow, fog),
 *             World (volcanic ash, water splash, pollen), Crafting (sparks, smoke).
 * LOD chain: High (0-15m), Medium (15-40m), Low (40-80m), Culled (>80m).
 */
UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_Manager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_Manager();

    // === NIAGARA ASSET REFERENCES ===
    // Environment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_CampfireSmoke;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
    TSoftObjectPtr<UNiagaraSystem> NS_Fire_CampfireEmbers;

    // Dinosaur
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_BreathVapor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_RoarDistortion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    TSoftObjectPtr<UNiagaraSystem> NS_Dino_BloodImpact;

    // Weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Rain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Snow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_Fog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    TSoftObjectPtr<UNiagaraSystem> NS_Weather_DustStorm;

    // Combat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_BloodSplatter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_WeaponImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
    TSoftObjectPtr<UNiagaraSystem> NS_Combat_RockImpact;

    // World
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_VolcanicAsh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_WaterSplash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_PollenDrift;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|World")
    TSoftObjectPtr<UNiagaraSystem> NS_World_InsectSwarm;

    // Crafting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Crafting")
    TSoftObjectPtr<UNiagaraSystem> NS_Crafting_FlintSparks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Crafting")
    TSoftObjectPtr<UNiagaraSystem> NS_Crafting_CookingSmoke;

    // === LOD SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_HighMaxDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_MediumMaxDistance = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_LowMaxDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    int32 MaxActiveVFXSystems = 32;

    // === SPAWN FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX_Campfire(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX_DinoFootstep(const FVector& Location, float DinoMassKg = 6000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX_BloodImpact(const FVector& Location, const FVector& ImpactNormal, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX_WeaponImpact(const FVector& Location, const FVector& ImpactNormal, bool bOnDinosaur);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetWeatherVFX(EVFX_Category WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllWeatherVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    EVFX_LODLevel GetLODLevelForDistance(float DistanceCm) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ClearAllVFX();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveWeatherComponents;

    void CleanupFinishedVFX();
    UNiagaraComponent* SpawnNiagaraAtLocation(TSoftObjectPtr<UNiagaraSystem> NiagaraAsset, const FVector& Location, const FRotator& Rotation, float Scale, bool bAutoDestroy);
};
