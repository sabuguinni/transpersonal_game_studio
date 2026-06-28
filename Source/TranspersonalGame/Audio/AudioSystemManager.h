#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Manages ambient audio zones, dialogue audio wiring,
// and survival-context adaptive audio for MinPlayableMap.
// All types prefixed Audio_ to avoid cross-agent collisions.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_AmbientZone : uint8
{
    Camp        UMETA(DisplayName = "Camp"),
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    OpenPlain   UMETA(DisplayName = "Open Plain"),
    Cave        UMETA(DisplayName = "Cave"),
    DangerZone  UMETA(DisplayName = "Danger Zone"),
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical"),
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientZone Zone = EAudio_AmbientZone::Camp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;

    // URL reference for TTS/Freesound assets loaded at runtime
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString ExternalAudioURL;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString DialogueText;

    // Supabase TTS URL from ElevenLabs generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    TSoftObjectPtr<USoundBase> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    bool bHasBeenPlayed = false;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AmbientZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AmbientZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_AmbientZone ZoneType = EAudio_AmbientZone::Camp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeToZone(EAudio_AmbientZone NewZone, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    FString GetZoneName() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    float ThreatTransitionTimer = 0.0f;
    bool bTransitioning = false;
};

UCLASS()
class TRANSPERSONALGAME_API AAudio_AmbientSourceActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientSourceActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudio_AmbientZoneComponent* AmbientZoneComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudioComponent* PrimaryAudioComp;

    // NPC dialogue entries wired from TTS generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    TArray<FAudio_DialogueEntry> DialogueEntries;

    // Camp audio URLs from cycle 013 TTS generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString ScoutWarningURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782606451163_ScoutWarning.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString ElderSurvivorURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782606472094_ElderSurvivor.mp3");

    // Previous cycle TTS URLs wired in
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString ElderTrackerURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782606319109_ElderTracker.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString ChiefHunterURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782606339713_ChiefHunter.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString CraftmasterURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782606350360_Craftmaster.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString ScoutRangerURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782606353494_ScoutRanger.mp3");

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueEntry(int32 EntryIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PopulateDefaultDialogueEntries();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    int32 GetDialogueCount() const { return DialogueEntries.Num(); }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    int32 CurrentDialogueIndex = 0;
    float DialogueTimer = 0.0f;
    bool bDialoguePlaying = false;
};
