#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    DustImpact      UMETA(DisplayName = "Dust Impact"),
    Rain            UMETA(DisplayName = "Rain"),
    Fog             UMETA(DisplayName = "Fog"),
    Wind            UMETA(DisplayName = "Wind Particles"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    Sparks          UMETA(DisplayName = "Sparks"),
    Smoke           UMETA(DisplayName = "Smoke")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::Campfire;
        SpawnLocation = FVector::ZeroVector;
        Intensity = 1.0f;
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable, Category = "Transpersonal VFX")
class TRANSPERSONALGAME_API AVFX_ParticleManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    // Niagara particle systems for different effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* CampfireSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* DustImpactSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* FogSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* WindSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* BloodSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* SparksSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    UNiagaraSystem* SmokeSystem;

    // Active particle components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveParticles;

    // VFX configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float GlobalIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bEnableWeatherEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bEnableCombatEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    int32 MaxActiveParticles = 50;

public:
    virtual void Tick(float DeltaTime) override;

    // Core VFX methods
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    UNiagaraComponent* SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, float Intensity = 1.0f, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnParticleFromConfig(const FVFX_ParticleConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAllParticles();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopParticlesByType(EVFX_ParticleType ParticleType);

    // Prehistoric-specific VFX
    UFUNCTION(BlueprintCallable, Category = "Prehistoric VFX")
    void CreateCampfire(FVector Location, float FireIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric VFX")
    void CreateDinosaurFootstepDust(FVector ImpactLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric VFX")
    void CreateWeatherEffect(EVFX_ParticleType WeatherType, FVector CenterLocation, float Coverage = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void CreateBloodSplatter(FVector HitLocation, FVector HitDirection, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Crafting VFX")
    void CreateCraftingSparks(FVector CraftLocation, float Duration = 2.0f);

    // VFX management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupExpiredParticles();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveParticleCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetGlobalIntensity(float NewIntensity);

    // Editor functions
    UFUNCTION(CallInEditor, Category = "VFX Testing")
    void TestAllParticleTypes();

    UFUNCTION(CallInEditor, Category = "VFX Testing")
    void TestCampfireEffect();

    UFUNCTION(CallInEditor, Category = "VFX Testing")
    void TestDustImpactEffect();

private:
    // Helper methods
    UNiagaraSystem* GetParticleSystemForType(EVFX_ParticleType ParticleType);
    void LoadDefaultParticleSystems();
    void RemoveExpiredParticles();

    // Timers
    FTimerHandle CleanupTimerHandle;
};