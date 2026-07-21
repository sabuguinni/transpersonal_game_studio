#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "DinosaurAudioSystem.generated.h"

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
enum class EAudio_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    EAudio_DinosaurSpecies Species = EAudio_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float GroundShakeThreshold = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    bool bCausesGroundShake = false;
};

USTRUCT(BlueprintType)
struct FAudio_EnvironmentZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float MusicIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    bool bIsNightZone = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float DangerLevel = 0.0f;
};

/**
 * ADinosaurAudioSystem
 * Manages spatial audio for all dinosaur species in the world.
 * Handles footstep rumble, roars, ambient calls, and ground shake feedback.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAudioSystem : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurAudioSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Core Audio Control ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurRoar(EAudio_DinosaurSpecies Species, FVector Location, float VolumeScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurFootstep(EAudio_DinosaurSpecies Species, FVector Location, float FootMass = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerGroundShake(FVector EpicenterLocation, float Intensity, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void SetDinosaurState(EAudio_DinosaurSpecies Species, EAudio_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    EAudio_DinosaurState GetDinosaurState(EAudio_DinosaurSpecies Species) const;

    // --- Environment Audio ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetEnvironmentZone(const FAudio_EnvironmentZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void FadeAmbientToNight(float FadeDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void FadeAmbientToDay(float FadeDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetDangerMusicIntensity(float Intensity);

    // --- Player Feedback ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void PlayPlayerDamageSound(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void PlayPlayerFootstep(FString SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void PlayCraftingSound(FString ItemCrafted);

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void PlayFireCrackle(FVector FireLocation, float Intensity = 1.0f);

    // --- Diagnostic ---

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void LogAudioSystemStatus();

    UFUNCTION(BlueprintPure, Category = "Audio|Dinosaur")
    float GetDistanceToNearestPredator(FVector PlayerLocation) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float GlobalVolumeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicFadeSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    bool bEnableGroundShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float GroundShakeDecayRate = 0.95f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
        meta = (AllowPrivateAccess = "true"))
    float CurrentDangerIntensity = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
        meta = (AllowPrivateAccess = "true"))
    float CurrentMusicIntensity = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
        meta = (AllowPrivateAccess = "true"))
    bool bIsNightTime = false;

private:
    TMap<EAudio_DinosaurSpecies, EAudio_DinosaurState> DinosaurStateMap;
    TMap<EAudio_DinosaurSpecies, FVector> DinosaurPositionCache;

    void InitializeDinosaurProfiles();
    void UpdateMusicIntensity(float DeltaTime);
    float CalculateDangerFromState(EAudio_DinosaurState State) const;
    FAudio_DinosaurSoundProfile* FindProfile(EAudio_DinosaurSpecies Species);
};
