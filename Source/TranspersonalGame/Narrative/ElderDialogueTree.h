#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElderDialogueTree.generated.h"

// ============================================================
// Elder Dialogue Tree — Agent #15 Narrative & Dialogue
// Tribal Elder NPC for QuestSurviveStampede
// Practical, weathered tracker — reads animal behaviour
// NO spiritual content — pure survival pragmatism
// ============================================================

UENUM(BlueprintType)
enum class ENarr_ElderDialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    QuestOffer      UMETA(DisplayName = "QuestOffer"),
    BranchAccept    UMETA(DisplayName = "BranchAccept"),
    BranchRefuse    UMETA(DisplayName = "BranchRefuse"),
    BranchAskMore   UMETA(DisplayName = "BranchAskMore"),
    QuestActive     UMETA(DisplayName = "QuestActive"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    PostQuestLore   UMETA(DisplayName = "PostQuestLore")
};

UENUM(BlueprintType)
enum class ENarr_ElderMood : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Urgent      UMETA(DisplayName = "Urgent"),
    Reflective  UMETA(DisplayName = "Reflective"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_ElderMood Mood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Elder"))
        , LineText(TEXT(""))
        , AudioURL(TEXT(""))
        , Mood(ENarr_ElderMood::Calm)
        , DisplayDuration(4.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueBranch
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString BranchID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString PlayerChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> ElderResponseLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_ElderDialogueState NextState;

    FNarr_DialogueBranch()
        : BranchID(TEXT(""))
        , PlayerChoiceText(TEXT(""))
        , NextState(ENarr_ElderDialogueState::Idle)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_LoreEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EntryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString BodyText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bUnlocked;

    FNarr_LoreEntry()
        : EntryID(TEXT(""))
        , Title(TEXT(""))
        , BodyText(TEXT(""))
        , AudioURL(TEXT(""))
        , bUnlocked(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AElderDialogueTree : public AActor
{
    GENERATED_BODY()

public:
    AElderDialogueTree();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Current State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    ENarr_ElderDialogueState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    bool bPlayerInRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|State")
    float InteractionRadius;

    // ---- Quest Offer Lines ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lines")
    TArray<FNarr_DialogueLine> QuestOfferLines;

    // ---- Dialogue Branches ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Branches")
    FNarr_DialogueBranch BranchAccept;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Branches")
    FNarr_DialogueBranch BranchRefuse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Branches")
    FNarr_DialogueBranch BranchAskMore;

    // ---- Post-Quest Lines ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lines")
    TArray<FNarr_DialogueLine> PostQuestLines;

    // ---- Lore Entries ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    TArray<FNarr_LoreEntry> LoreEntries;

    // ---- Audio URLs (ElevenLabs TTS) ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString AudioURL_StampedeWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString AudioURL_QuestComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString AudioURL_LoreDrought;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString AudioURL_PostQuestReflection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString AudioURL_AskMoreBranch;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogueState(ENarr_ElderDialogueState NewState);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayerChoseAccept();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayerChoseRefuse();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayerChoseAskMore();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestCompleted();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_LoreEntry GetLoreEntry(const FString& EntryID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnlockLoreEntry(const FString& EntryID);

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FString GetCurrentAudioURL() const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool IsPlayerInInteractionRange(const FVector& PlayerLocation) const;

    UFUNCTION(CallInEditor, Category = "Narrative|Debug")
    void DEBUG_PrintAllDialogue();

    UFUNCTION(CallInEditor, Category = "Narrative|Debug")
    void DEBUG_ForceQuestComplete();

private:
    void InitialiseDialogueData();
    void InitialiseLoreData();
};
