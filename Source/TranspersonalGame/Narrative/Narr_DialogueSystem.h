#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting        UMETA(DisplayName = "Greeting"),
    Warning         UMETA(DisplayName = "Warning"),
    Teaching        UMETA(DisplayName = "Teaching"),
    Trading         UMETA(DisplayName = "Trading"),
    QuestGiving     UMETA(DisplayName = "Quest Giving"),
    Storytelling    UMETA(DisplayName = "Storytelling"),
    Emergency       UMETA(DisplayName = "Emergency")
};

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None            UMETA(DisplayName = "No Condition"),
    HasItem         UMETA(DisplayName = "Has Item"),
    CompletedQuest  UMETA(DisplayName = "Completed Quest"),
    TimeOfDay       UMETA(DisplayName = "Time of Day"),
    PlayerHealth    UMETA(DisplayName = "Player Health"),
    NearDanger      UMETA(DisplayName = "Near Danger"),
    FirstMeeting    UMETA(DisplayName = "First Meeting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueCondition RequiredCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ConditionValue;

    FNarr_DialogueChoice()
    {
        ChoiceText = FText::FromString("Continue...");
        NextDialogueID = "";
        RequiredCondition = ENarr_DialogueCondition::None;
        ConditionValue = "";
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        SpeakerName = FText::FromString("Unknown");
        DialogueText = FText::FromString("");
        DialogueType = ENarr_DialogueType::Greeting;
        AudioAssetPath = "";
        DisplayDuration = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CurrentDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanStartDialogue(AActor* Player) const;

private:
    FNarr_DialogueEntry* FindDialogueByID(const FString& DialogueID);
    bool CheckCondition(const ENarr_DialogueCondition& Condition, const FString& Value) const;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    UPROPERTY()
    TMap<FString, FNarr_DialogueEntry> GlobalDialogueDatabase;

    UPROPERTY()
    TArray<FString> CompletedDialogues;

    UPROPERTY()
    TMap<FString, int32> DialogueFlags;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetDialogue(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void MarkDialogueCompleted(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueCompleted(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueFlag(const FString& FlagName, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    int32 GetDialogueFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadPrehistoricDialogues();

private:
    void CreateSurvivalDialogues();
    void CreateTeachingDialogues();
    void CreateWarningDialogues();
};