#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Adaptive ambient audio, dinosaur proximity audio,
// screen shake triggers, and dialogue audio routing.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    DenseJungle     UMETA(DisplayName = "Dense Jungle"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveInterior    UMETA(DisplayName = "Cave Interior"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    NightCamp       UMETA(DisplayName = "Night Camp"),
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Danger          UMETA(DisplayName = "Danger"),
    Panic           UMETA(DisplayName = "Panic"),
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenSavanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ThreatLevel MaxThreatLevel = EAudio_ThreatLevel::Panic;
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName DinoSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> RoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepShakeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepShakeMagnitude = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RoarAudibleRadius = 5000.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DisplayDuration = 4.0f;
};

// ============================================================
// UAudio_ProximityComponent — attaches to dinosaur actors
// Plays footstep/roar audio and triggers camera shake
// when player is within range.
// ============================================================
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FAudio_DinoSoundProfile SoundProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float ProximityCheckInterval = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dino", meta = (AllowPrivateAccess = "true"))
    float CurrentDistanceToPlayer = 99999.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerFootstep();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    float GetDistanceToPlayer() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TimeSinceLastCheck = 0.0f;
    float TimeSinceLastFootstep = 0.0f;

    UPROPERTY()
    UAudioComponent* RoarAudioComp = nullptr;

    UPROPERTY()
    UAudioComponent* FootstepAudioComp = nullptr;

    UPROPERTY()
    UAudioComponent* BreathingAudioComp = nullptr;

    void UpdateProximityAudio(float DeltaTime);
    void ApplyCameraShake(float Magnitude, float Radius);
};

// ============================================================
// AAudio_AmbientZoneActor — placed in level to define
// biome audio zones. Fades ambient layers in/out as
// player enters/exits the zone radius.
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AmbientZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZoneActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenSavanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendWeight = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    bool IsPlayerInZone() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    void UpdateBlendWeight(float DeltaTime);
    void SyncLayersToThreat();
};

// ============================================================
// UAudio_SystemManagerSubsystem — GameInstance subsystem
// Central coordinator for all audio in the game.
// Manages biome transitions, threat level, dialogue routing.
// ============================================================
UCLASS()
class TRANSPERSONALGAME_API UAudio_SystemManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void SetGlobalThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void SetActiveBiome(EAudio_BiomeZone NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    EAudio_ThreatLevel GetGlobalThreatLevel() const { return GlobalThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    EAudio_BiomeZone GetActiveBiome() const { return ActiveBiome; }

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void PlayDialogueLine(const FAudio_DialogueLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    bool IsDialoguePlaying() const { return bDialoguePlaying; }

    // TTS Audio URL registry — maps speaker name to Supabase URL
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    TMap<FString, FString> TTSAudioURLRegistry;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void RegisterTTSAudioURL(const FString& SpeakerName, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    FString GetTTSAudioURL(const FString& SpeakerName) const;

private:
    EAudio_ThreatLevel GlobalThreatLevel = EAudio_ThreatLevel::Safe;
    EAudio_BiomeZone ActiveBiome = EAudio_BiomeZone::OpenSavanna;
    bool bDialoguePlaying = false;

    UPROPERTY()
    UAudioComponent* DialogueAudioComp = nullptr;

    void InitializeTTSRegistry();
};
