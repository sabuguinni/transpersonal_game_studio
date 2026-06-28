#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystem.generated.h"

// ============================================================
// Enums — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EAudio_AmbientZone : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    OpenPlain   UMETA(DisplayName = "Open Plain"),
    Camp        UMETA(DisplayName = "Camp"),
    Danger      UMETA(DisplayName = "Danger Zone")
};

UENUM(BlueprintType)
enum class EAudio_DinoSoundType : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roar        UMETA(DisplayName = "Roar"),
    Footstep    UMETA(DisplayName = "Footstep"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death"),
    Herd        UMETA(DisplayName = "Herd Call")
};

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Exploration UMETA(DisplayName = "Exploration"),
    Tension     UMETA(DisplayName = "Tension"),
    Combat      UMETA(DisplayName = "Combat"),
    Camp        UMETA(DisplayName = "Camp / Safe"),
    Night       UMETA(DisplayName = "Night"),
    Discovery   UMETA(DisplayName = "Discovery")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientZone Zone = EAudio_AmbientZone::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString SoundURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bIsLooping = true;

    FAudio_AmbientLayer() {}
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FName DinoSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    EAudio_DinoSoundType SoundType = EAudio_DinoSoundType::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FString SoundURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float AttenuationRadius = 3000.0f;

    FAudio_DinoSoundEntry() {}
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FName CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString SubtitleText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float EstimatedDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bHasBeenPlayed = false;

    FAudio_VoiceLine() {}
};

USTRUCT(BlueprintType)
struct FAudio_FreesoundRef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    int32 FreesoundID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    FString PreviewURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    FString Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    float Duration = 0.0f;

    FAudio_FreesoundRef() {}
};

// ============================================================
// UAudio_SystemComponent — main audio manager component
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Audio System Component")
class TRANSPERSONALGAME_API UAudio_SystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemComponent();

    // ---- Ambient Layers ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientZone CurrentZone = EAudio_AmbientZone::Forest;

    // ---- Dino Sounds ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    TArray<FAudio_DinoSoundEntry> DinoSoundRegistry;

    // ---- Voice Lines ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> VoiceLineRegistry;

    // ---- Music State ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_MusicState CurrentMusicState = EAudio_MusicState::Exploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionTime = 3.0f;

    // ---- Freesound References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    TArray<FAudio_FreesoundRef> FreesoundRefs;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    float CurrentMusicVolume = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    bool bNightModeActive = false;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetAmbientZone(EAudio_AmbientZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    FAudio_DinoSoundEntry GetDinoSound(FName Species, EAudio_DinoSoundType SoundType) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    FAudio_VoiceLine GetNextUnplayedVoiceLine(FName CharacterID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void MarkVoiceLinePlayed(FName CharacterID, int32 LineIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetNightMode(bool bIsNight);

    UFUNCTION(BlueprintCallable, Category = "Audio|Freesound")
    void RegisterFreesoundRef(int32 SoundID, const FString& Name, const FString& PreviewURL, const FString& Tags, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio|Freesound")
    TArray<FAudio_FreesoundRef> GetFreesoundRefsByTag(const FString& Tag) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void LoadDefaultAmbientLayers();
    void LoadDefaultDinoSounds();
    void LoadDefaultVoiceLines();
    void LoadFreesoundRefs();
};
