#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/AmbientSound.h"
#include "DinosaurAudioSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pteranodon          UMETA(DisplayName = "Pteranodon"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    EAudio_DinosaurSpecies Species = EAudio_DinosaurSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* IdleVocalisation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* AlertRoar = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* AttackRoar = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* FootstepHeavy = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* FootstepLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepGroundShakeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float VocalisationRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepInterval = 0.8f;
};

USTRUCT(BlueprintType)
struct FAudio_EnvironmentZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    USoundCue* DayAmbience = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    USoundCue* NightAmbience = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    USoundCue* RainLayer = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float DayVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float NightVolume = 0.9f;
};

/**
 * UAudio_DinosaurAudioComponent
 * Manages spatial audio for dinosaur entities — vocalisations, footsteps,
 * ground shake propagation, and state-driven sound transitions.
 * Attaches to any dinosaur pawn in the MinPlayableMap.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Audio Component")
class TRANSPERSONALGAME_API UAudio_DinosaurAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DinosaurAudioComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FAudio_DinosaurSoundProfile SoundProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    EAudio_DinosaurState CurrentState = EAudio_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    bool bEnableGroundShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float GroundShakeIntensity = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void SetDinosaurState(EAudio_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayFootstep(bool bIsHeavyStep);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayVocalisation(EAudio_DinosaurState ForState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerGroundShakeAtLocation(FVector Location, float Intensity, float Radius);

    UFUNCTION(BlueprintPure, Category = "Audio|Dinosaur")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Dinosaur")
    bool IsPlayerInAlertRange() const;

private:
    float FootstepTimer = 0.0f;
    float StateTransitionCooldown = 0.0f;
    APawn* CachedPlayerPawn = nullptr;

    void UpdateFootstepLoop(float DeltaTime);
    void CachePlayerReference();
};

/**
 * UAudio_EnvironmentAudioManager
 * World-level ambient audio manager. Handles day/night crossfades,
 * biome zone blending, weather layers, and campfire proximity audio.
 * Placed as a singleton actor component on the GameMode or a dedicated manager actor.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Environment Audio Manager")
class TRANSPERSONALGAME_API UAudio_EnvironmentAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_EnvironmentAudioManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    TArray<FAudio_EnvironmentZone> EnvironmentZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float DayNightBlendSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    bool bIsNight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float CampfireAudioRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    USoundCue* CampfireLoop = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    USoundCue* ThunderStrike = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    USoundCue* RainLoop = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetDayNightState(bool bNight);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetWeatherState(bool bRaining);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void TriggerThunder(FVector StrikeLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void RegisterCampfireLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void UnregisterCampfireLocation(FVector Location);

    UFUNCTION(BlueprintPure, Category = "Audio|Environment")
    float GetCurrentAmbientVolume() const;

private:
    TArray<FVector> ActiveCampfireLocations;
    float CurrentBlendAlpha = 0.0f;
    float ThunderCooldown = 0.0f;

    void UpdateDayNightBlend(float DeltaTime);
    void UpdateCampfireProximityAudio();
    void UpdateWeatherAudio(float DeltaTime);
};
