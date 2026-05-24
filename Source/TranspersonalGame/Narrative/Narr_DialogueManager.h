#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        Duration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString QuestID;

    FNarr_DialogueTree()
    {
        TreeID = TEXT("");
        NPCName = TEXT("");
        bIsQuestRelated = false;
        QuestID = TEXT("");
    }
};

UENUM(BlueprintType)
enum class ENarr_NPCPersonality : uint8
{
    Wise_Elder      UMETA(DisplayName = "Wise Elder"),
    Practical_Guide UMETA(DisplayName = "Practical Guide"),
    Cautious_Scout  UMETA(DisplayName = "Cautious Scout"),
    Bold_Hunter     UMETA(DisplayName = "Bold Hunter"),
    Mystic_Reader   UMETA(DisplayName = "Mystic Reader")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
    ENarr_NPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
    TArray<FString> SpecialKnowledge;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
    FVector HomeLocation;

    FNarr_NPCProfile()
    {
        NPCName = TEXT("");
        Personality = ENarr_NPCPersonality::Wise_Elder;
        BackgroundStory = TEXT("");
        HomeLocation = FVector::ZeroVector;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& NPCName, class APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInDialogue() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue(int32 ResponseIndex = 0);

    // NPC management
    UFUNCTION(BlueprintCallable, Category = "NPC")
    void RegisterNPC(const FNarr_NPCProfile& NPCProfile);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    FNarr_NPCProfile GetNPCProfile(const FString& NPCName) const;

    UFUNCTION(BlueprintCallable, Category = "NPC")
    TArray<FString> GetNearbyNPCs(const FVector& PlayerLocation, float SearchRadius = 1000.0f) const;

    // Dialogue tree management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueTree(const FNarr_DialogueTree& DialogueTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueTree GetDialogueTree(const FString& NPCName) const;

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TriggerQuestDialogue(const FString& QuestID, const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool HasQuestDialogue(const FString& QuestID) const;

protected:
    UPROPERTY()
    TMap<FString, FNarr_NPCProfile> RegisteredNPCs;

    UPROPERTY()
    TMap<FString, FNarr_DialogueTree> DialogueTrees;

    UPROPERTY()
    bool bIsDialogueActive;

    UPROPERTY()
    FString CurrentNPCName;

    UPROPERTY()
    int32 CurrentDialogueIndex;

    UPROPERTY()
    class APawn* CurrentPlayerPawn;

private:
    void InitializeDefaultNPCs();
    void InitializeDefaultDialogues();
    bool ValidateDialogueTree(const FNarr_DialogueTree& Tree) const;
};