#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "Audio_NarrativeAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_CharacterVoiceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString VoiceURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    EAudio_PersonalityTrait PersonalityTrait;

    FAudio_CharacterVoiceData()
    {
        CharacterName = TEXT("");
        VoiceURL = TEXT("");
        Duration = 0.0f;
        PersonalityTrait = EAudio_PersonalityTrait::Neutral;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FAudio_CharacterVoiceData> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsOneShot;

    FAudio_NarrativeEvent()
    {
        EventName = TEXT("");
        TriggerRadius = 500.0f;
        bIsOneShot = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_NarrativeAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_NarrativeAudioManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TArray<FAudio_NarrativeEvent> NarrativeEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TMap<FString, FAudio_CharacterVoiceData> CharacterVoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterNarrativeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VoicelineAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudibleDistance;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayCharacterVoiceLine(const FString& CharacterName, const FString& VoiceURL);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void TriggerNarrativeEvent(const FString& EventName, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void RegisterCharacterVoice(const FString& CharacterName, const FString& VoiceURL, float Duration, EAudio_PersonalityTrait Trait);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    bool IsVoiceLineActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopCurrentVoiceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetNarrativeVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    FAudio_CharacterVoiceData GetCharacterVoiceData(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    TArray<FString> GetAvailableCharacterVoices() const;

private:
    bool bIsPlayingVoiceLine;
    float CurrentVoiceLineTimer;
    FString CurrentCharacterSpeaking;

    void UpdateVoiceLinePlayback(float DeltaTime);
    void OnVoiceLineComplete();
    bool CanPlayVoiceLine(const FString& CharacterName) const;
};