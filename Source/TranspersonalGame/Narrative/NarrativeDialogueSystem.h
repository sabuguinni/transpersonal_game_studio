#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "NarrativeDialogueSystem.generated.h"

// Narr_ prefix for all types per UE5 compilation rules

UENUM(BlueprintType)
enum class ENarr_DialogueTone : uint8
{
    Urgent      UMETA(DisplayName = "Urgent"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Informative UMETA(DisplayName = "Informative"),
    Commanding  UMETA(DisplayName = "Commanding"),
    Fearful     UMETA(DisplayName = "Fearful")
};

UENUM(BlueprintType)
enum class ENarr_QuestStage : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    FNarr_DialogueLine()
        : SpeakerID(TEXT("Unknown"))
        , Tone(ENarr_DialogueTone::Informative)
        , DisplayDuration(4.0f)
        , AudioAssetPath(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_QuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ENarr_QuestStage Stage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ObjectiveIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardDescription;

    FNarr_QuestData()
        : QuestID(TEXT(""))
        , Stage(ENarr_QuestStage::NotStarted)
        , RewardDescription(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FText NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString Role; // Hunter, Elder, Scout, Crafter

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FString> DialogueLineIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float TrustLevel; // 0.0 - 1.0

    FNarr_NPCProfile()
        : NPCID(TEXT(""))
        , Role(TEXT("Hunter"))
        , TrustLevel(0.5f)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeDialogueSystem();

    // Active dialogue lines loaded for current scene
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> ActiveDialogueLines;

    // NPC profiles registered in this scene
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPCs")
    TArray<FNarr_NPCProfile> RegisteredNPCs;

    // Current active quest
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|Quest")
    FNarr_QuestData ActiveQuest;

    // All quests in the game
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    TArray<FNarr_QuestData> AllQuests;

    // Current dialogue line index
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 CurrentLineIndex;

    // Is dialogue currently playing
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bDialogueActive;

    // Start a dialogue sequence by NPC ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogue(const FString& NPCID);

    // Advance to next dialogue line
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogue();

    // End current dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    // Get current dialogue line text
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    FText GetCurrentLineText() const;

    // Get current speaker name
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    FString GetCurrentSpeaker() const;

    // Start a quest by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    bool StartQuest(const FString& QuestID);

    // Complete a quest objective
    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    void CompleteObjective(const FString& ObjectiveID);

    // Mark active quest as complete
    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    void CompleteActiveQuest();

    // Fail the active quest
    UFUNCTION(BlueprintCallable, Category = "Narrative|Quest")
    void FailActiveQuest();

    // Register an NPC profile
    UFUNCTION(BlueprintCallable, Category = "Narrative|NPCs")
    void RegisterNPC(const FNarr_NPCProfile& NPCProfile);

    // Get NPC trust level
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|NPCs")
    float GetNPCTrust(const FString& NPCID) const;

    // Increase NPC trust (player helped them)
    UFUNCTION(BlueprintCallable, Category = "Narrative|NPCs")
    void IncreaseTrust(const FString& NPCID, float Amount);

    // Load default prehistoric survival quests
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative|Setup")
    void LoadDefaultQuests();

    // Load default NPC profiles
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative|Setup")
    void LoadDefaultNPCs();

protected:
    virtual void BeginPlay() override;

private:
    // Internal: find NPC by ID
    FNarr_NPCProfile* FindNPC(const FString& NPCID);

    // Internal: find quest by ID
    FNarr_QuestData* FindQuest(const FString& QuestID);

    // Completed objective IDs for active quest
    TArray<FString> CompletedObjectives;
};
