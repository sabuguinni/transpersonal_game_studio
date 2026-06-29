#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "DinosaurAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding")
};

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenPlains  UMETA(DisplayName = "Open Plains"),
    DenseForest UMETA(DisplayName = "Dense Forest"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Cliffside   UMETA(DisplayName = "Cliffside"),
    DangerZone  UMETA(DisplayName = "Danger Zone")
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float IdleVocalFrequency = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float AlertVocalFrequency = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    bool bCausesScreenShake = false;
};

USTRUCT(BlueprintType)
struct FAudio_AmbientZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DayVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float NightVolume = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DangerMultiplier = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float TensionFadeSpeed = 2.0f;
};

/**
 * ADinosaurAudioManager
 * Manages adaptive audio for dinosaur species — idle vocalizations, footstep weight,
 * roar propagation, and ambient zone transitions based on player proximity and danger level.
 * Follows Walter Murch principle: silence before the roar is more powerful than the roar itself.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAudioManager : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurAudioManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Dinosaur Audio ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinosaurRoar(AActor* DinosaurActor, FName SpeciesName, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void UpdateDinosaurState(AActor* DinosaurActor, EAudio_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayFootstepImpact(AActor* DinosaurActor, float FootWeight, FVector ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerSilenceEvent(float SilenceDuration, float RadiusMeters);

    // --- Ambient Zone Audio ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetPlayerBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetDangerLevel(float DangerLevel01);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(float TimeNormalized01);

    // --- Screen Shake ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerFootstepScreenShake(float Intensity, FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageAudioFeedback(float DamageAmount);

    // --- State Queries ---

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    float GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_BiomeZone GetCurrentBiomeZone() const { return CurrentBiomeZone; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    bool IsInSilenceEvent() const { return bInSilenceEvent; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* PrimaryAmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* DangerStingerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientZoneConfig> AmbientZoneConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float GlobalAudioScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SilenceEventFadeSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MaxRoarScreenShakeRadius = 2000.0f;

private:
    float CurrentDangerLevel = 0.0f;
    float CurrentTimeOfDay = 0.5f;
    float SilenceTimer = 0.0f;
    bool bInSilenceEvent = false;
    EAudio_BiomeZone CurrentBiomeZone = EAudio_BiomeZone::OpenPlains;

    void UpdateAmbientVolumes(float DeltaTime);
    void TickSilenceEvent(float DeltaTime);
    FAudio_DinosaurSoundProfile* FindProfileForSpecies(FName SpeciesName);
};
