// NarrativeDialogueSystem.h
// Narrative & Dialogue Agent #15 — Transpersonal Game Studio
// Complete header: dialogue trigger types, speaker roles, line structs, conversation structs, bark system

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NarrativeDialogueSystem.generated.h"

// ============================================================
// ENarr_DialogueTriggerType — how dialogue is activated
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "Quest Start"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    CombatAlert     UMETA(DisplayName = "Combat Alert"),
    ResourceFound   UMETA(DisplayName = "Resource Found"),
    PlayerDeath     UMETA(DisplayName = "Player Death"),
    DayNightChange  UMETA(DisplayName = "Day Night Change"),
    Manual          UMETA(DisplayName = "Manual")
};

// ============================================================
// ENarr_SpeakerRole — who is speaking
// ============================================================
UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Narrator        UMETA(DisplayName = "Narrator")
};

// ============================================================
// ENarr_BarkType — single-line NPC reactions
// ============================================================
UENUM(BlueprintType)
enum class ENarr_BarkType : uint8
{
    DinosaurSpotted     UMETA(DisplayName = "Dinosaur Spotted"),
    ResourceNearby      UMETA(DisplayName = "Resource Nearby"),
    PlayerLowHealth     UMETA(DisplayName = "Player Low Health"),
    NightFalling        UMETA(DisplayName = "Night Falling"),
    CraftingSuccess     UMETA(DisplayName = "Crafting Success"),
    CraftingFailed      UMETA(DisplayName = "Crafting Failed"),
    QuestObjectiveNear  UMETA(DisplayName = "Quest Objective Near"),
    Idle                UMETA(DisplayName = "Idle")
};

// ============================================================
// FNarr_DialogueLine — a single line of dialogue
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole SpeakerRole = ENarr_SpeakerRole::Narrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsPlayerChoice = false;
};

// ============================================================
// FNarr_DialogueConversation — a named sequence of lines
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueConversation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName ConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bAutoAdvance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCanRepeat = false;
};

// ============================================================
// FNarr_BarkLine — a single-line NPC reaction bark
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_BarkLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_BarkType BarkType = ENarr_BarkType::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText BarkText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;
};

// ============================================================
// UNarr_DialogueComponent — ActorComponent for NPC dialogue
// ============================================================
UCLASS(ClassGroup = "Narrative", meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:

    // --- Dialogue Library ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Library")
    TArray<FNarr_DialogueConversation> DialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Library")
    TArray<FNarr_BarkLine> BarkLibrary;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FName CurrentConversationID;

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float LineDisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    ENarr_SpeakerRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    bool bCanRepeatDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bHasBeenTriggered;

    // --- Delegates ---
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueStarted, FName, ConversationID);
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueStarted OnDialogueStarted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded, FName, ConversationID);
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnLineDisplayed, FNarr_DialogueLine, Line);
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnLineDisplayed OnLineDisplayed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnBarkTriggered, FNarr_BarkLine, Bark);
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnBarkTriggered OnBarkTriggered;

    // --- Methods ---
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartConversation(FName ConversationID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerBark(ENarr_BarkType BarkType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterConversation(FNarr_DialogueConversation NewConversation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetLineCount() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetCurrentLineIndex() const;

private:
    FNarr_DialogueConversation ActiveConversation;
    FTimerHandle LineAdvanceTimer;
};
