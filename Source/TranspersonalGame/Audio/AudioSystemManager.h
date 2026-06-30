
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "AudioSystemManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;
};

USTRUCT(BlueprintType)
struct FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundCue* MusicCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float CrossfadeDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float Volume = 0.7f;
};

USTRUCT(BlueprintType)
struct FAudio_NPCVoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|NPC")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|NPC")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|NPC")
    FString SubtitleText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|NPC")
    float SpatialRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|NPC")
    float Volume = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FString SpeciesID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* IdleSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* AlertSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* ChargeSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundCue* FootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepGroundShakeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepGroundShakeIntensity = 1.0f;
};

// ─── Main Class ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    // ── Ambient System ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdateAmbientLayers();

    // ── Adaptive Music ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void CrossfadeToMusicState(const FAudio_MusicState& NewState);

    UFUNCTION(BlueprintPure, Category = "Audio|Music")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // ── NPC Dialogue Audio ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|NPC")
    void PlayNPCVoiceLine(const FAudio_NPCVoiceLine& Line, AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|NPC")
    void StopNPCVoiceLine();

    UFUNCTION(BlueprintPure, Category = "Audio|NPC")
    bool IsNPCLineActive() const { return bNPCLineActive; }

    // ── Dinosaur Audio ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurFootstep(const FAudio_DinosaurSoundProfile& Profile, FVector FootLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinosaurAlert(const FAudio_DinosaurSoundProfile& Profile, AActor* DinoActor);

    // ── Screen Feedback ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerCameraShake(float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageAudioFeedback(float DamageAmount);

    // ── State ────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_MusicState> MusicStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float DialogueVolume = 1.0f;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    UAudioComponent* ActiveMusicComponent = nullptr;

    UPROPERTY()
    UAudioComponent* ActiveAmbientComponent = nullptr;

    UPROPERTY()
    UAudioComponent* ActiveNPCComponent = nullptr;

    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Jungle;
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    bool bNPCLineActive = false;
    float ThreatTransitionTimer = 0.0f;

    void BlendAmbientToCurrentState();
    void EvaluateThreatTransition(float DeltaTime);
};
