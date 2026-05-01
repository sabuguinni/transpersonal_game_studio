#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

// Dialogue line data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextLineIndices;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        DisplayDuration = 3.0f;
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
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 StartingLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    FNarr_DialogueConversation()
    {
        ConversationID = TEXT("DefaultConversation");
        StartingLineIndex = 0;
        bIsRepeatable = true;
    }
};

// Narrative context for survival situations
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_SurvivalContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SurvivalPriority CurrentPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> NearbyThreats;

    FNarr_SurvivalContext()
    {
        CurrentPriority = ENarr_SurvivalPriority::Water;
        TimeOfDay = 12.0f;
        CurrentBiome = ENarr_BiomeType::Forest;
        ThreatLevel = 0.5f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueStarted, const FString&, ConversationID, const FNarr_DialogueLine&, FirstLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded, const FString&, ConversationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueLineChanged, int32, LineIndex, const FNarr_DialogueLine&, CurrentLine);

/**
 * Dialogue System Component for narrative interactions
 * Handles contextual survival dialogue based on player state and environment
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(const FString& ConversationID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue(int32 ChoiceIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterConversation(const FNarr_DialogueConversation& Conversation);

    // Context-aware dialogue
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetContextualDialogue(const FNarr_SurvivalContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UpdateSurvivalContext(const FNarr_SurvivalContext& NewContext);

    // Audio playback
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueAudio();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsInConversation() const { return bIsInConversation; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    TArray<FNarr_DialogueLine> GetCurrentChoices() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FNarr_OnDialogueLineChanged OnDialogueLineChanged;

protected:
    // Current conversation state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bIsInConversation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString CurrentConversationID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    // Conversation database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<FString, FNarr_DialogueConversation> RegisteredConversations;

    // Survival context
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    FNarr_SurvivalContext CurrentSurvivalContext;

    // Audio component for voice playback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* DialogueAudioComponent;

private:
    // Helper functions
    bool IsValidLineIndex(int32 LineIndex) const;
    void InitializeDefaultConversations();
    FString GenerateContextualResponse(const FNarr_SurvivalContext& Context);
    void BroadcastDialogueEvents(const FNarr_DialogueLine& Line);
};