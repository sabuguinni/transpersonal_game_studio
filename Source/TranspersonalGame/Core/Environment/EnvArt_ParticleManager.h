#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "TranspersonalGame/SharedTypes.h"
#include "EnvArt_ParticleManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEnvArtParticles, Log, All);

/**
 * Atmospheric particle configuration for different environments
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Config")
    EBiomeType BiomeType = EBiomeType::Savanna;

    // Dust particles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dust")
    bool bEnableDust = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dust")
    float DustSpawnRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dust")
    FVector DustVelocity = FVector(10.0f, 0.0f, 5.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dust")
    FLinearColor DustColor = FLinearColor(0.8f, 0.7f, 0.5f, 0.3f);

    // Pollen particles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pollen")
    bool bEnablePollen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pollen")
    float PollenSpawnRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pollen")
    FVector PollenVelocity = FVector(5.0f, 0.0f, 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pollen")
    FLinearColor PollenColor = FLinearColor(1.0f, 1.0f, 0.8f, 0.4f);

    // Mist/fog particles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mist")
    bool bEnableMist = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mist")
    float MistSpawnRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mist")
    float MistLifetime = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mist")
    FLinearColor MistColor = FLinearColor(0.9f, 0.9f, 1.0f, 0.2f);

    // Wind interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    bool bAffectedByWind = true;

    // Spawn area
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnAreaSize = FVector(2000.0f, 2000.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float ParticleLifetime = 10.0f;
};

/**
 * Manages atmospheric particle effects for environmental storytelling
 * Creates dust motes, pollen, mist, and other ambient particles
 */
UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_ParticleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_ParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PARTICLE SYSTEM CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Manager")
    float UpdateRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Manager")
    int32 MaxActiveParticleSystems = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Manager")
    bool bUseNiagaraSystem = true;

    // === BIOME PARTICLE CONFIGURATIONS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Particles")
    TMap<EBiomeType, FEnvArt_ParticleConfig> BiomeParticleConfigs;

    // === NIAGARA SYSTEM REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    UNiagaraSystem* DustParticleSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    UNiagaraSystem* PollenParticleSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    UNiagaraSystem* MistParticleSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    UNiagaraSystem* WindParticleSystem = nullptr;

    // === ENVIRONMENTAL METHODS ===

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void InitializeParticleManager();

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void UpdateParticlesForBiome(EBiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void SpawnDustParticles(const FVector& Location, const FEnvArt_ParticleConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void SpawnPollenParticles(const FVector& Location, const FEnvArt_ParticleConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void SpawnMistParticles(const FVector& Location, const FEnvArt_ParticleConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void UpdateWindEffects(const FVector& WindDirection, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void CreateGoldenHourParticles(const FVector& SunDirection);

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void CreateForestMistEffect(const FVector& ForestCenter, float MistRadius);

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void SetParticleIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Particle Manager")
    void CleanupDistantParticles(const FVector& PlayerLocation);

private:
    // === INTERNAL METHODS ===

    void InitializeBiomeParticleConfigs();
    void CreateNiagaraComponent(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation);
    void UpdateParticleParameters(UNiagaraComponent* Component, const FEnvArt_ParticleConfig& Config);
    bool IsLocationInRange(const FVector& Location, const FVector& PlayerLocation) const;
    void RemoveOldestParticleSystem();

    // === INTERNAL STATE ===

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveParticleSystems;

    UPROPERTY()
    EBiomeType CurrentBiome = EBiomeType::Savanna;

    FVector LastPlayerLocation = FVector::ZeroVector;
    float ParticleUpdateTimer = 0.0f;
    float ParticleUpdateInterval = 0.5f; // Update every 0.5 seconds

    // Wind state
    FVector CurrentWindDirection = FVector(1.0f, 0.0f, 0.0f);
    float CurrentWindStrength = 1.0f;

    // Performance tracking
    int32 ParticleSystemsSpawnedThisFrame = 0;
    int32 MaxParticleSystemsPerFrame = 3;
};