#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryState : uint8
{
    Introduction,
    FirstHunt,
    PackEncounter,
    ResourceScarcity,
    TribalConflict,
    SeasonalMigration,
    FinalSurvival
};

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Narration,
    Warning,
    Instruction,
    Observation,
    Emergency
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceClip;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Narrator");
        DialogueText = TEXT("");
        DialogueType = ENarr_DialogueType::Narration;
        Duration = 5.0f;
        VoiceClip = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryState RequiredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerRadius;

    FNarr_StoryEvent()
    {
        EventName = TEXT("DefaultEvent");
        RequiredState = ENarr_StoryState::Introduction;
        bIsTriggered = false;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_StoryState CurrentStoryState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DialogueTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsPlayingDialogue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* VoiceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float StoryProgressionTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentDialogueIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_StoryEvent* CurrentStoryEvent;

public:
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryState();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckPlayerProximityToEvents();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryState GetCurrentStoryState() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeStoryEvents();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnStoryStateChanged(ENarr_StoryState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueStarted(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueFinished();
};