#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "../SharedTypes.h"
#include "NarrativeEventManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsOneTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    USoundBase* VoiceAudio;

    FNarr_NarrativeEvent()
    {
        EventID = TEXT("");
        DialogueText = TEXT("");
        CharacterName = TEXT("");
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        bIsOneTime = true;
        bHasBeenTriggered = false;
        VoiceAudio = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float SurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 DinosaurEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 SuccessfulHunts;

    FNarr_StoryProgress()
    {
        CurrentChapter = 1;
        SurvivalDays = 0.0f;
        DinosaurEncounters = 0;
        SuccessfulHunts = 0;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeEventManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID, AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CheckEventTrigger(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeAudio(const FNarr_NarrativeEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateStoryProgress(const FString& EventType, int32 Value = 1);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_StoryProgress GetStoryProgress() const { return CurrentStoryProgress; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeDefaultNarrativeEvents();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_NarrativeEvent> GetActiveEvents() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_StoryProgress CurrentStoryProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    UAudioComponent* NarrativeAudioComponent;

    void CreateSurvivalNarrativeEvents();
    void CreatePredatorWarningEvents();
    void CreateDiscoveryEvents();
};