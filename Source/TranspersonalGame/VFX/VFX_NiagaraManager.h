#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Dust_Footstep       UMETA(DisplayName = "Footstep Dust"),
    Dust_Impact         UMETA(DisplayName = "Impact Dust"),
    Blood_Splatter      UMETA(DisplayName = "Blood Splatter"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Breath_Steam        UMETA(DisplayName = "Breath Steam"),
    Roar_Distortion     UMETA(DisplayName = "Roar Air Distortion"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Combat_Sparks       UMETA(DisplayName = "Combat Sparks"),
    Environment_Pollen  UMETA(DisplayName = "Pollen Particles")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::Fire_Campfire;
        Duration = 5.0f;
        Intensity = 1.0f;
        Scale = FVector::OneVector;
        bAutoDestroy = true;
    }
};

/**
 * VFX Niagara Manager - Handles all particle effects for the prehistoric game
 * Manages fire, dust, blood, weather, and environmental particle systems
 * Designed for realistic prehistoric world without magical/spiritual effects
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // === PARTICLE SYSTEM SPAWNING ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Spawning")
    UNiagaraComponent* SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawning")
    UNiagaraComponent* SpawnParticleAttached(EVFX_ParticleType ParticleType, USceneComponent* AttachComponent, FName SocketName = NAME_None, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawning")
    void SpawnFootstepDust(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Spawning")
    void SpawnCombatImpact(FVector Location, FVector ImpactNormal, float Damage = 50.0f);

    // === ENVIRONMENTAL EFFECTS ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void StartCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void StopCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void StartWeatherEffect(EVFX_ParticleType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void StopWeatherEffect();

    // === DINOSAUR EFFECTS ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayDinosaurBreath(AActor* DinosaurActor, bool bColdWeather = false);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayRoarDistortion(FVector RoarLocation, float RoarPower = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayBloodEffect(FVector ImpactLocation, FVector BloodDirection, float BloodAmount = 1.0f);

    // === SYSTEM MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void SetGlobalVFXIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void EnableVFXCategory(EVFX_ParticleType CategoryType, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void CleanupExpiredEffects();

    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EVFX_ParticleType, FVFX_ParticleConfig> ParticleConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float GlobalVFXIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    int32 MaxActiveParticles = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float ParticleCleanupInterval = 10.0f;

protected:
    // === INTERNAL SYSTEMS ===
    
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveParticleComponents;

    UPROPERTY()
    UNiagaraComponent* CurrentWeatherEffect;

    UPROPERTY()
    TMap<FVector, UNiagaraComponent*> CampfireEffects;

    FTimerHandle CleanupTimerHandle;

    void InitializeParticleConfigs();
    void LoadNiagaraSystems();
    UNiagaraSystem* GetNiagaraSystemForType(EVFX_ParticleType ParticleType);
    void RegisterActiveComponent(UNiagaraComponent* Component);
    void UnregisterActiveComponent(UNiagaraComponent* Component);

private:
    void PerformCleanup();
    bool IsValidParticleType(EVFX_ParticleType ParticleType) const;
};