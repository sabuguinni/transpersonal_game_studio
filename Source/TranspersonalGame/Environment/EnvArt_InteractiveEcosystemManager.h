#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "Engine/AmbientSound.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnvArt_InteractiveEcosystemManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_EcosystemZoneType : uint8
{
    PrimaryForest UMETA(DisplayName = "Primary Forest"),
    RiverDelta UMETA(DisplayName = "River Delta"),
    VolcanicSlopes UMETA(DisplayName = "Volcanic Slopes"),
    OpenPlains UMETA(DisplayName = "Open Plains"),
    RockyOutcrops UMETA(DisplayName = "Rocky Outcrops")
};

UENUM(BlueprintType)
enum class EEnvArt_InteractivePropType : uint8
{
    FallenLog UMETA(DisplayName = "Fallen Log"),
    ClimbingRock UMETA(DisplayName = "Climbing Rock"),
    ShelterTree UMETA(DisplayName = "Shelter Tree"),
    CrystalNode UMETA(DisplayName = "Crystal Node"),
    BonePile UMETA(DisplayName = "Bone Pile")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_EcosystemZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    EEnvArt_EcosystemZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<FString> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float BiodiversityLevel;

    FEnvArt_EcosystemZone()
    {
        ZoneType = EEnvArt_EcosystemZoneType::PrimaryForest;
        Location = FVector::ZeroVector;
        Radius = 3000.0f;
        BiodiversityLevel = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_InteractiveProp
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    EEnvArt_InteractivePropType PropType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    bool bIsInteractable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    FString InteractionPrompt;

    FEnvArt_InteractiveProp()
    {
        PropType = EEnvArt_InteractivePropType::FallenLog;
        Location = FVector::ZeroVector;
        InteractionRadius = 300.0f;
        bIsInteractable = true;
        InteractionPrompt = TEXT("Interact");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_InteractiveEcosystemManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_InteractiveEcosystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === ECOSYSTEM ZONES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Zones")
    TArray<FEnvArt_EcosystemZone> EcosystemZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Zones")
    float ZoneTransitionDistance;

    // === INTERACTIVE PROPS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactive Props")
    TArray<FEnvArt_InteractiveProp> InteractiveProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactive Props")
    float PropSpawnRadius;

    // === AMBIENT AUDIO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<AAmbientSound*> AmbientSoundActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioBlendDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolumeMultiplier;

    // === ATMOSPHERIC EFFECTS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<UParticleSystemComponent*> AtmosphericParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float ParticleIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableDynamicWeather;

    // === ECOSYSTEM FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void InitializeEcosystemZones();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void SpawnInteractiveProps();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void SetupAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateAtmosphericEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    EEnvArt_EcosystemZoneType GetCurrentEcosystemZone(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void BlendAudioForZone(EEnvArt_EcosystemZoneType ZoneType, float BlendWeight);

    UFUNCTION(BlueprintCallable, Category = "Props")
    bool CanInteractWithProp(const FVector& PlayerLocation, int32 PropIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Props")
    void InteractWithProp(int32 PropIndex);

    // === ATMOSPHERIC LIGHTING ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateVolumetricFog(float Density, float HeightFalloff);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnAtmosphericParticles(const FVector& Location, const FString& ParticleType);

private:
    // === INTERNAL TRACKING ===
    UPROPERTY()
    EEnvArt_EcosystemZoneType CurrentPlayerZone;

    UPROPERTY()
    float ZoneBlendTimer;

    UPROPERTY()
    TArray<ATriggerBox*> ZoneTriggers;

    // === HELPER FUNCTIONS ===
    void CreateZoneTrigger(const FEnvArt_EcosystemZone& Zone);
    void UpdateAudioBlending(float DeltaTime);
    void ProcessEcosystemInteractions();
    float CalculateZoneInfluence(const FVector& Location, const FEnvArt_EcosystemZone& Zone) const;
};