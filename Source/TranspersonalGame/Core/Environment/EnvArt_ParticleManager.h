#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "EnvArt_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_ParticleType : uint8
{
    Dust        UMETA(DisplayName = "Dust Motes"),
    Pollen      UMETA(DisplayName = "Pollen"),
    Leaves      UMETA(DisplayName = "Falling Leaves"),
    Insects     UMETA(DisplayName = "Flying Insects"),
    Spores      UMETA(DisplayName = "Spores"),
    Ash         UMETA(DisplayName = "Volcanic Ash"),
    Mist        UMETA(DisplayName = "Forest Mist"),
    Fireflies   UMETA(DisplayName = "Fireflies")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_ParticleZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneExtent = FVector(1000.0f, 1000.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EEnvArt_ParticleType ParticleType = EEnvArt_ParticleType::Dust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float ParticleDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bActiveInDaylight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bActiveAtNight = false;

    FEnvArt_ParticleZone()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneExtent = FVector(1000.0f, 1000.0f, 500.0f);
        ParticleType = EEnvArt_ParticleType::Dust;
        ParticleDensity = 1.0f;
        bActiveInDaylight = true;
        bActiveAtNight = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_ParticleManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_ParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Zones")
    TArray<FEnvArt_ParticleZone> ParticleZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxViewDistance = 5000.0f;

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void CreateForestDustZone(FVector Location, FVector Extent);

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void CreatePollenZone(FVector Location, FVector Extent);

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void CreateFireflyZone(FVector Location, FVector Extent);

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void UpdateParticleZones();

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void SetWindParameters(FVector NewDirection, float NewStrength);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAmbientForestSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDistantDinosaurRoars();

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveParticleSystems;

    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* DinosaurAudioComponent;

    float LastUpdateTime;

    void SpawnParticleSystem(const FEnvArt_ParticleZone& Zone);
    void UpdateParticleSystemParameters(UNiagaraComponent* ParticleSystem, const FEnvArt_ParticleZone& Zone);
    bool IsZoneActiveForTimeOfDay(const FEnvArt_ParticleZone& Zone) const;
    float GetCurrentTimeOfDay() const;
};