#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "NarrativeTypes.h"
#include "DialogueManager.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, DialogueID, const FString&, SpeakerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueLineChanged, const FNarr_DialogueLine&, CurrentLine, int32, LineIndex, int32, TotalLines);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChoicesPresented, const TArray<FNarr_DialogueChoice>&, Choices);

/**
 * Manages dialogue system and conversation flow
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Narrative))
class TRANSPERSONALGAME_API ADialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ADialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Dialogue Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueLineChanged OnDialogueLineChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnChoicesPresented OnChoicesPresented;

    // Dialogue Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    UDataTable* CharacterDataTable;

    // Current Dialogue State
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FNarr_DialogueNode CurrentDialogueNode;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    TArray<FString> PlayerFlags;

    // Dialogue Control Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

    // Flag Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue Flags")
    void SetPlayerFlag(const FString& FlagName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Flags")
    bool HasPlayerFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Flags")
    void RemovePlayerFlag(const FString& FlagName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Flags")
    void ClearAllFlags();

    // Dialogue Utilities
    UFUNCTION(BlueprintCallable, Category = "Dialogue Utilities")
    bool IsDialogueAvailable(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Utilities")
    bool AreRequirementsMet(const TArray<FString>& RequiredFlags) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Utilities")
    FNarr_CharacterLore GetCharacterLore(const FString& CharacterID) const;

    // Auto-advance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Advance")
    bool bAutoAdvanceEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Advance")
    float AutoAdvanceDelay;

private:
    // Internal state
    float AutoAdvanceTimer;
    bool bWaitingForChoice;

    // Internal functions
    void ProcessCurrentLine();
    void ApplyChoiceEffects(const FNarr_DialogueChoice& Choice);
    FNarr_DialogueNode* GetDialogueNode(const FString& DialogueID) const;
    void ResetDialogueState();
};

/**
 * Component for actors that can participate in dialogue
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueParticipantComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueParticipantComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Participant Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Participant")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Participant")
    FString DefaultDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Participant")
    TArray<FString> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Participant")
    bool bCanInitiateDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Participant")
    float InteractionDistance;

    // References
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue Participant")
    ADialogueManager* DialogueManager;

    // Dialogue Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue Participant")
    bool StartDialogueWithPlayer();

    UFUNCTION(BlueprintCallable, Category = "Dialogue Participant")
    bool CanStartDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Participant")
    FString GetBestAvailableDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Participant")
    void AddAvailableDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Participant")
    void RemoveAvailableDialogue(const FString& DialogueID);

private:
    void FindDialogueManager();
};