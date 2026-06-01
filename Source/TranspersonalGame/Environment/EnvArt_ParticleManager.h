#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "EnvArt_ParticleManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_ParticleEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    float ParticleLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    int32 MaxParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effect")
    FLinearColor ParticleColor;

    FEnvArt_ParticleEffect()
    {
        EffectName = TEXT("DefaultEffect");
        SpawnLocation = FVector::ZeroVector;
        SpawnRadius = 1000.0f;
        ParticleLifetime = 5.0f;
        MaxParticles = 100;
        ParticleColor = FLinearColor::White;
    }
};

UENUM(BlueprintType)
enum class EEnvArt_ParticleType : uint8
{
    Dust        UMETA(DisplayName = "Dust"),
    Pollen      UMETA(DisplayName = "Pollen"),
    Mist        UMETA(DisplayName = "Mist"),
    Sand        UMETA(DisplayName = "Sand"),
    Snow        UMETA(DisplayName = "Snow"),
    Ash         UMETA(DisplayName = "Ash"),
    Leaves      UMETA(DisplayName = "Leaves"),
    Fireflies   UMETA(DisplayName = "Fireflies")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_ParticleManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_ParticleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core particle system component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* ParticleSystemComponent;

    // Particle effects for each biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Particles")
    TMap<EBiomeType, FEnvArt_ParticleEffect> BiomeParticleEffects;

    // Active particle effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<UParticleSystemComponent*> ActiveParticleComponents;

    // Particle management settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Management")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Management")
    float MaxParticleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Management")
    int32 MaxActiveParticleSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Management")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Management")
    bool bEnableBiomeSpecificParticles;

    // Wind and environmental factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Factors")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Factors")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Factors")
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Factors")
    float WeatherIntensity;

private:
    float LastUpdateTime;
    class APawn* CachedPlayerPawn;

public:
    // Particle management functions
    UFUNCTION(BlueprintCallable, Category = "Particle Management")
    void CreateBiomeParticles(EBiomeType BiomeType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Particle Management")
    void CreateAtmosphericEffect(EEnvArt_ParticleType ParticleType, const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Particle Management")
    void UpdateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Particle Management")
    void CleanupDistantParticles();

    UFUNCTION(BlueprintCallable, Category = "Particle Management")
    void SetWindParameters(const FVector& NewWindDirection, float NewWindStrength);

    UFUNCTION(BlueprintCallable, Category = "Particle Management")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Particle Management")
    void SetWeatherIntensity(float NewWeatherIntensity);

    // Biome-specific particle creation
    UFUNCTION(BlueprintCallable, Category = "Biome Particles")
    void CreateSavannaParticles(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Particles")
    void CreateForestParticles(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Particles")
    void CreateSwampParticles(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Particles")
    void CreateDesertParticles(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Particles")
    void CreateMountainParticles(const FVector& Location);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    EBiomeType GetBiomeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetDistanceToPlayer(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool ShouldCullParticleSystem(UParticleSystemComponent* ParticleComponent);

    // Debug functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSpawnTestParticles();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugClearAllParticles();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogParticleSystemStatus();
};

#include "EnvArt_ParticleManager.generated.h"