#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_VoiceActedCharacterSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    TSoftObjectPtr<USoundCue> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    ESurvivalContext TriggerContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    int32 Priority;

    FNarr_VoiceLine()
    {
        LineID = TEXT("");
        DialogueText = FText::GetEmpty();
        Duration = 0.0f;
        TriggerContext = ESurvivalContext::Normal;
        Priority = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterVoiceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    TArray<FNarr_VoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    float BasePitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    bool bCanInterrupt;

    FNarr_CharacterVoiceProfile()
    {
        CharacterName = TEXT("Unknown");
        BaseVolume = 1.0f;
        BasePitch = 1.0f;
        bCanInterrupt = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_VoiceActedCharacterSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_VoiceActedCharacterSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VoiceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    FNarr_CharacterVoiceProfile VoiceProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    TMap<FString, FNarr_VoiceLine> VoiceLineDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    float MaxDialogueDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    bool bAutoTriggerContextualLines;

    UPROPERTY(BlueprintReadOnly, Category = "Voice System")
    bool bIsCurrentlySpeaking;

    UPROPERTY(BlueprintReadOnly, Category = "Voice System")
    FString CurrentLineID;

    UPROPERTY(BlueprintReadOnly, Category = "Voice System")
    float CurrentLineTimeRemaining;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    bool PlayVoiceLine(const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    bool PlayContextualLine(ESurvivalContext Context);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void StopCurrentVoiceLine();

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    bool IsPlayerInRange() const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void AddVoiceLine(const FNarr_VoiceLine& NewLine);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void RemoveVoiceLine(const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    TArray<FString> GetAvailableLines() const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    FNarr_VoiceLine GetVoiceLine(const FString& LineID) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice System")
    void OnVoiceLineStarted(const FString& LineID, const FText& DialogueText);

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice System")
    void OnVoiceLineFinished(const FString& LineID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice System")
    void OnVoiceLineInterrupted(const FString& LineID);

protected:
    UFUNCTION()
    void OnAudioFinished();

    void UpdateVoiceLineTimer(float DeltaTime);
    void CheckContextualTriggers();
    FNarr_VoiceLine* FindBestContextualLine(ESurvivalContext Context);

private:
    FTimerHandle VoiceLineTimer;
    float VoiceLineStartTime;
    bool bVoiceLineTimerActive;
};