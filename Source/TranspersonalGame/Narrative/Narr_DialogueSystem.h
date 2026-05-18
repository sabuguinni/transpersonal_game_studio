#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Narration       UMETA(DisplayName = "Narration"),
    Warning         UMETA(DisplayName = "Warning"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Tutorial        UMETA(DisplayName = "Tutorial"),
    Ambient         UMETA(DisplayName = "Ambient")
};

UENUM(BlueprintType)
enum class ENarr_TriggerCondition : uint8
{
    OnEnter         UMETA(DisplayName = "On Enter"),
    OnExit          UMETA(DisplayName = "On Exit"),
    OnInteract      UMETA(DisplayName = "On Interact"),
    OnTimer         UMETA(DisplayName = "On Timer"),
    OnEvent         UMETA(DisplayName = "On Event")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class USoundBase* AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bSubtitlesEnabled;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT(""));
        AudioClip = nullptr;
        DialogueType = ENarr_DialogueType::Narration;
        Duration = 3.0f;
        bSubtitlesEnabled = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueLine> DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_TriggerCondition TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bOneTimeOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bHasBeenTriggered;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventName = TEXT("");
        TriggerCondition = ENarr_TriggerCondition::OnEnter;
        bOneTimeOnly = true;
        bHasBeenTriggered = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DefaultDialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bGlobalSubtitlesEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FNarr_DialogueLine CurrentDialogue;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddStoryEvent(const FNarr_StoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void SetSubtitlesEnabled(bool bEnabled);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue System")
    void OnDialogueStarted(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue System")
    void OnDialogueFinished(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue System")
    void OnStoryEventTriggered(const FNarr_StoryEvent& StoryEvent);

protected:
    UFUNCTION()
    void OnAudioFinished();

private:
    FTimerHandle DialogueTimerHandle;
    int32 CurrentDialogueIndex;
    TArray<FNarr_DialogueLine> CurrentDialogueSequence;

    void PlayNextDialogue();
    void FinishDialogueSequence();
};