#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Quest_VoiceActingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_VoiceLineData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    FString VoiceLineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    bool bIsQuestCritical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    FString QuestID;

    FQuest_VoiceLineData()
    {
        CharacterName = TEXT("");
        VoiceLineText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        bIsQuestCritical = false;
        QuestID = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_VoiceActingManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_VoiceActingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    TArray<FQuest_VoiceLineData> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    bool bAutoPlayOnTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    float VoiceLineDelay;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void PlayVoiceLine(const FString& CharacterName, const FString& QuestID = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void StopCurrentVoiceLine();

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void AddVoiceLine(const FQuest_VoiceLineData& NewVoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    bool IsVoiceLinePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    FQuest_VoiceLineData GetVoiceLineByCharacter(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    TArray<FQuest_VoiceLineData> GetVoiceLinesByQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice Acting")
    void OnVoiceLineStarted(const FQuest_VoiceLineData& VoiceLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice Acting")
    void OnVoiceLineCompleted(const FQuest_VoiceLineData& VoiceLine);

private:
    FQuest_VoiceLineData CurrentVoiceLine;
    bool bIsPlaying;
    float CurrentPlayTime;

    void InitializeVoiceLines();
    void LoadAudioFromURL(const FString& AudioURL);
};