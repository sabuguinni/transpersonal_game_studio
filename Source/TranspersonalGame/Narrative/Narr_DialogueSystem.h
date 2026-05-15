#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Narrator        UMETA(DisplayName = "Narrator"),
    Warning         UMETA(DisplayName = "Warning"),
    HuntLeader      UMETA(DisplayName = "Hunt Leader"),
    ElderWisdom     UMETA(DisplayName = "Elder Wisdom"),
    ScoutReport     UMETA(DisplayName = "Scout Report"),
    QuestGiver      UMETA(DisplayName = "Quest Giver")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnQuestStart        UMETA(DisplayName = "Quest Start"),
    OnDinosaurSighted   UMETA(DisplayName = "Dinosaur Sighted"),
    OnDangerArea        UMETA(DisplayName = "Danger Area"),
    OnCampSetup         UMETA(DisplayName = "Camp Setup"),
    OnHuntSuccess       UMETA(DisplayName = "Hunt Success"),
    OnSurvivalTip       UMETA(DisplayName = "Survival Tip")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    FNarr_DialogueEntry()
    {
        SpeakerType = ENarr_DialogueType::Narrator;
        TriggerCondition = ENarr_DialogueTrigger::OnQuestStart;
        Priority = 1.0f;
        bIsRepeatable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FString& ContextData = "");

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueEntry(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueEntry> GetDialoguesForTrigger(ENarr_DialogueTrigger TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetDialogueEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueEnabled() const { return bDialogueEnabled; }

protected:
    // Dialogue storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bDialogueEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DialogueCooldownTime;

    // Internal state
    UPROPERTY()
    TArray<FString> PlayedDialogueIDs;

    UPROPERTY()
    float LastDialogueTime;

    // Helper methods
    void LoadDefaultDialogues();
    bool CanPlayDialogue(const FNarr_DialogueEntry& Entry);
    void MarkDialogueAsPlayed(const FString& DialogueID);
};