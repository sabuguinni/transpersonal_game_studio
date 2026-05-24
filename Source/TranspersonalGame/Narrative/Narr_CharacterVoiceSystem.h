#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "Narr_CharacterVoiceSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    ESurvivalContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    int32 Priority;

    FNarr_VoiceLine()
    {
        CharacterName = TEXT("");
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Context = ESurvivalContext::Exploration;
        Duration = 0.0f;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString VoiceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FNarr_VoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BasePitch;

    FNarr_CharacterProfile()
    {
        CharacterName = TEXT("");
        VoiceID = TEXT("");
        BaseVolume = 1.0f;
        BasePitch = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_CharacterVoiceSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_CharacterVoiceSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Voice line management
    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void RegisterCharacterProfile(const FNarr_CharacterProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void PlayVoiceLine(const FString& CharacterName, const FString& DialogueText, ESurvivalContext Context);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void PlayContextualVoice(ESurvivalContext Context, int32 Priority = 5);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void StopCurrentVoice();

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    bool IsVoicePlaying() const;

    // Character management
    UFUNCTION(BlueprintCallable, Category = "Voice System")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    TArray<FString> GetRegisteredCharacters() const;

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    float GetMasterVolume() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    TMap<FString, FNarr_CharacterProfile> CharacterProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    TArray<FNarr_VoiceLine> QueuedVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    FNarr_VoiceLine CurrentVoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    bool bIsPlaying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    float VoiceTimer;

    // Audio component for playback
    UPROPERTY()
    class UAudioComponent* VoiceAudioComponent;

private:
    void ProcessVoiceQueue();
    void OnVoiceLineComplete();
    void InitializeDefaultCharacters();
    FNarr_VoiceLine FindBestVoiceLine(const FString& CharacterName, ESurvivalContext Context) const;
};