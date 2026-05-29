#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Sound/SoundCue.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Extreme     UMETA(DisplayName = "Extreme Danger")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<UMetaSoundSource> AmbientMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> BiomeMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundWave>> CreatureSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeDistance = 5000.0f;

    FAudio_BiomeAudioProfile()
    {
        BiomeType = EAudio_BiomeType::Savanna;
        BaseVolume = 0.7f;
        FadeDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_DialogueAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundWave> VoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bHasSubtitles = true;

    FAudio_DialogueAudioData()
    {
        Duration = 0.0f;
        bHasSubtitles = true;
    }
};

/**
 * MetaSound-based audio manager for adaptive biome soundscapes and dialogue integration
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateThreatLevel(EAudio_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Intensity = 1.0f);

    // Dialogue system integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueLine(const FString& NPCName, const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetDialogueVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsDialoguePlaying() const;

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEnvironmentalEffect(const FString& EffectName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientVolume(float Volume);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Configuration")
    TArray<FAudio_BiomeAudioProfile> BiomeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Configuration")
    TMap<FString, FAudio_DialogueAudioData> DialogueDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY()
    UAudioComponent* BiomeAudioComponent;

    UPROPERTY()
    UAudioComponent* DialogueAudioComponent;

    UPROPERTY()
    UAudioComponent* EffectsAudioComponent;

private:
    void InitializeBiomeProfiles();
    void InitializeDialogueDatabase();
    void UpdateBiomeAudio();
    void CrossfadeBiomes(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome);

    float MasterVolume = 1.0f;
    float AmbientVolume = 0.8f;
    float DialogueVolume = 1.0f;
    bool bDialoguePlaying = false;
};