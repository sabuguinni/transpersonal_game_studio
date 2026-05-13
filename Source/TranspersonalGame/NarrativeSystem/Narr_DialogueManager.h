#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Tribal_Elder        UMETA(DisplayName = "Tribal Elder"),
    Hunt_Leader         UMETA(DisplayName = "Hunt Leader"),
    War_Chief           UMETA(DisplayName = "War Chief"),
    Water_Seeker        UMETA(DisplayName = "Water Seeker"),
    Survivor_Whisper    UMETA(DisplayName = "Survivor Whisper"),
    Warning_Shout       UMETA(DisplayName = "Warning Shout"),
    Discovery_Call      UMETA(DisplayName = "Discovery Call")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Player_Approach     UMETA(DisplayName = "Player Approach"),
    Dinosaur_Spotted    UMETA(DisplayName = "Dinosaur Spotted"),
    Resource_Found      UMETA(DisplayName = "Resource Found"),
    Danger_Detected     UMETA(DisplayName = "Danger Detected"),
    Quest_Start         UMETA(DisplayName = "Quest Start"),
    Quest_Complete      UMETA(DisplayName = "Quest Complete"),
    Time_Based          UMETA(DisplayName = "Time Based"),
    Location_Based      UMETA(DisplayName = "Location Based")
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
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanInterrupt;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        DialogueType = ENarr_DialogueType::Survivor_Whisper;
        TriggerType = ENarr_DialogueTrigger::Player_Approach;
        Priority = 1.0f;
        CooldownTime = 5.0f;
        bCanInterrupt = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float SequenceCooldown;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("DefaultSequence");
        bLooping = false;
        SequenceCooldown = 30.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, const FNarr_DialogueLine&, DialogueLine, AActor*, TriggeringActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, const FString&, SequenceID);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType, AActor* TriggeringActor = nullptr, const FString& ContextData = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueSequence(const FString& SequenceID, AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueVolume(float Volume);

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueTriggered OnDialogueTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueCompleted OnDialogueCompleted;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Database")
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue State")
    TMap<ENarr_DialogueTrigger, float> TriggerCooldowns;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsDialoguePlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentSequenceID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Settings")
    float GlobalDialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Settings")
    float DefaultCooldownTime;

private:
    void ProcessDialogueQueue();
    void PlayNextDialogueLine();
    bool CanTriggerDialogue(ENarr_DialogueTrigger TriggerType) const;
    void UpdateCooldowns(float DeltaTime);
    
    FTimerHandle DialogueTimerHandle;
    TArray<FNarr_DialogueLine> DialogueQueue;
};