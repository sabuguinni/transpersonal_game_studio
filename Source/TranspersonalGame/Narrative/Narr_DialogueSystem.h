#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

// Dialogue state for tracking conversation progress
UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive,
    WaitingForPlayer,
    InProgress,
    Completed,
    Locked
};

// Dialogue trigger types for different narrative contexts
UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Tutorial,
    QuestGiver,
    StoryReveal,
    Warning,
    Lore
};

// Single dialogue line data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        DisplayDuration = 3.0f;
        AudioAssetPath = TEXT("");
    }
};

// Complete dialogue sequence
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanRepeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredQuestID;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("default");
        TriggerType = ENarr_DialogueTriggerType::Tutorial;
        bCanRepeat = false;
        RequiredQuestID = TEXT("");
    }
};

/**
 * Dialogue System Component - Handles narrative interactions and story delivery
 * Integrates with quest system and provides contextual dialogue based on game state
 */
UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Current dialogue state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    ENarr_DialogueState CurrentState;

    // Available dialogue sequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Content")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    // Currently active sequence
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    int32 ActiveSequenceIndex;

    // Current line in active sequence
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

    // Trigger dialogue sequence
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogueSequence(const FString& SequenceID);

    // Advance to next dialogue line
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue();

    // End current dialogue
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    // Check if dialogue is available
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueAvailable(const FString& SequenceID) const;

    // Get current dialogue line
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    // Add new dialogue sequence at runtime
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueSequence(const FNarr_DialogueSequence& NewSequence);

private:
    // Find sequence by ID
    int32 FindSequenceIndex(const FString& SequenceID) const;

    // Validate sequence requirements
    bool ValidateSequenceRequirements(const FNarr_DialogueSequence& Sequence) const;
};

/**
 * Dialogue Trigger Volume - Activates dialogue when player enters
 */
UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public ATriggerVolume
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

    // Trigger overlap events
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                       bool bFromSweep, const FHitResult& SweepResult);

public:
    // Dialogue sequence to trigger
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    FString DialogueSequenceID;

    // Trigger type for context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    ENarr_DialogueTriggerType TriggerType;

    // Can trigger multiple times
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    bool bCanRetrigger;

    // Has been triggered
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Trigger")
    bool bHasTriggered;

private:
    // Reference to dialogue system
    UPROPERTY()
    class UNarr_DialogueManager* DialogueManager;
};

/**
 * Dialogue Manager Subsystem - Global dialogue system management
 */
UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Register dialogue component
    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    void RegisterDialogueComponent(UNarr_DialogueComponent* Component);

    // Unregister dialogue component
    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    void UnregisterDialogueComponent(UNarr_DialogueComponent* Component);

    // Start dialogue with specific component
    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    bool StartDialogue(UNarr_DialogueComponent* Component, const FString& SequenceID);

    // Get active dialogue component
    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    UNarr_DialogueComponent* GetActiveDialogueComponent() const;

    // Check if any dialogue is active
    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    bool IsDialogueActive() const;

    // Load dialogue sequences from data table
    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    void LoadDialogueSequences(class UDataTable* DialogueTable);

private:
    // Registered dialogue components
    UPROPERTY()
    TArray<UNarr_DialogueComponent*> RegisteredComponents;

    // Currently active dialogue component
    UPROPERTY()
    UNarr_DialogueComponent* ActiveDialogueComponent;

    // Global dialogue sequences
    UPROPERTY()
    TArray<FNarr_DialogueSequence> GlobalDialogueSequences;
};