#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// Forward declarations
class UDialogueComponent;
class ANarrativeActor;

// Dialogue line data structure for DataTable
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextLineIndices;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioAssetPath = TEXT("");
        Duration = 3.0f;
        bIsPlayerChoice = false;
    }
};

// Dialogue conversation data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueConversation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ConversationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 StartLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_QuestType RequiredQuestType;

    FNarr_DialogueConversation()
    {
        ConversationID = TEXT("CONV_DEFAULT");
        ConversationName = TEXT("Default Conversation");
        StartLineIndex = 0;
        bIsRepeatable = true;
        RequiredQuestType = EEng_QuestType::Survival;
    }
};

// Dialogue trigger types
UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnInteract      UMETA(DisplayName = "On Interact"),
    OnProximity     UMETA(DisplayName = "On Proximity"),
    OnQuestStart    UMETA(DisplayName = "On Quest Start"),
    OnQuestComplete UMETA(DisplayName = "On Quest Complete"),
    OnDiscovery     UMETA(DisplayName = "On Discovery"),
    OnDanger        UMETA(DisplayName = "On Danger")
};

// Dialogue state
UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    WaitingInput    UMETA(DisplayName = "Waiting Input"),
    Completed       UMETA(DisplayName = "Completed"),
    Interrupted     UMETA(DisplayName = "Interrupted")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStateChanged, ENarr_DialogueState, NewState, const FNarr_DialogueLine&, CurrentLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, const FString&, ConversationID);

/**
 * Dialogue System Component - Manages narrative conversations and voice lines
 * Focuses on survival scenarios: field notes, danger alerts, discoveries
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(const FString& ConversationID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue(int32 ChoiceIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayVoiceLine(const FString& AudioPath, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerNarration(ENarr_DialogueTrigger TriggerType, const FString& Context = TEXT(""));

    // Survival-specific dialogue triggers
    UFUNCTION(BlueprintCallable, Category = "Survival Dialogue")
    void TriggerDangerAlert(EEng_DinosaurSpecies ThreatSpecies, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Survival Dialogue")
    void TriggerDiscoveryNarration(const FString& DiscoveryType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Dialogue")
    void TriggerWeatherWarning(EEng_WeatherType WeatherType, float Severity);

    UFUNCTION(BlueprintCallable, Category = "Survival Dialogue")
    void TriggerFieldNotes(const FString& ObservationText);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    ENarr_DialogueState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentLine() const { return CurrentLine; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return CurrentState == ENarr_DialogueState::Active || CurrentState == ENarr_DialogueState::WaitingInput; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStateChanged OnDialogueStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueCompleted OnDialogueCompleted;

protected:
    // Current dialogue state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    ENarr_DialogueState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    FNarr_DialogueLine CurrentLine;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    FNarr_DialogueConversation CurrentConversation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    float DialogueTimer;

    // Dialogue data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    TArray<FNarr_DialogueConversation> AvailableConversations;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* DefaultVoiceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VoiceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bAutoAdvanceDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AutoAdvanceDelay;

private:
    // Internal methods
    void ProcessCurrentLine();
    void SetDialogueState(ENarr_DialogueState NewState);
    FNarr_DialogueConversation* FindConversation(const FString& ConversationID);
    void LoadSurvivalDialogues();
    void CreateEmergencyDialogues();
};