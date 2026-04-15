#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

class UDialogueComponent;
class ANarrativeNPC;

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive,
    WaitingForPlayer,
    InProgress,
    Completed,
    Failed
};

UENUM(BlueprintType)
enum class ENarr_NPCMood : uint8
{
    Neutral,
    Friendly,
    Cautious,
    Hostile,
    Fearful,
    Aggressive
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_NPCMood RequiredMood;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        AudioAssetPath = TEXT("");
        Duration = 3.0f;
        RequiredMood = ENarr_NPCMood::Neutral;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        bIsRepeatable = false;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarr_DialogueSequence> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_NPCMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> CompletedDialogues;

    FNarr_NPCProfile()
    {
        NPCName = TEXT("");
        NPCRole = TEXT("");
        CurrentMood = ENarr_NPCMood::Neutral;
        TrustLevel = 0.5f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& NPCName, const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    // NPC Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNPC(const FString& NPCName, const FNarr_NPCProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_NPCProfile GetNPCProfile(const FString& NPCName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNPCMood(const FString& NPCName, ENarr_NPCMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNPCTrust(const FString& NPCName, float TrustDelta);

    // Story Progress
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkDialogueCompleted(const FString& NPCName, const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueCompleted(const FString& NPCName, const FString& SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableDialogues(const FString& NPCName) const;

protected:
    UPROPERTY()
    TMap<FString, FNarr_NPCProfile> NPCProfiles;

    UPROPERTY()
    FString CurrentNPCName;

    UPROPERTY()
    FString CurrentSequenceID;

    UPROPERTY()
    int32 CurrentLineIndex;

    UPROPERTY()
    ENarr_DialogueState DialogueState;

    UPROPERTY()
    float DialogueStartTime;

private:
    void LoadDefaultNPCProfiles();
    void CreateSurvivalDialogues();
    FNarr_NPCProfile CreateElderHunterProfile();
    FNarr_NPCProfile CreateScoutRunnerProfile();
    FNarr_NPCProfile CreateTribeChiefProfile();
};