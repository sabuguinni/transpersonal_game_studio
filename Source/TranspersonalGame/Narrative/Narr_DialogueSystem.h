#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Tutorial     UMETA(DisplayName = "Tutorial"),
    Warning      UMETA(DisplayName = "Warning"),
    Discovery    UMETA(DisplayName = "Discovery"),
    Combat       UMETA(DisplayName = "Combat"),
    Story        UMETA(DisplayName = "Story"),
    Ambient      UMETA(DisplayName = "Ambient")
};

UENUM(BlueprintType)
enum class ENarr_NPCType : uint8
{
    TribalElder  UMETA(DisplayName = "Tribal Elder"),
    Scout        UMETA(DisplayName = "Scout"),
    Trader       UMETA(DisplayName = "Trader"),
    Shaman       UMETA(DisplayName = "Shaman"),
    Hunter       UMETA(DisplayName = "Hunter"),
    Survivor     UMETA(DisplayName = "Survivor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> Prerequisites;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        DialogueText = FText::GetEmpty();
        DialogueType = ENarr_DialogueType::Ambient;
        SpeakerType = ENarr_NPCType::Survivor;
        AudioFilePath = "";
        Duration = 3.0f;
        bIsRepeatable = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText StoryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText StoryDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 CurrentStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 TotalSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedDialogues;

    FNarr_StoryProgress()
    {
        StoryID = "";
        StoryTitle = FText::GetEmpty();
        StoryDescription = FText::GetEmpty();
        bIsCompleted = false;
        CurrentStep = 0;
        TotalSteps = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(const FString& DialogueID, AActor* Speaker = nullptr, AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueAvailable(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetDialogue(const FString& DialogueID) const;

    // Story Progress
    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStory(const FString& StoryID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetStoryProgress(const FString& StoryID, int32 NewStep);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryProgress GetStoryProgress(const FString& StoryID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteStory(const FString& StoryID);

    // Context-based Dialogue
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerContextDialogue(ENarr_DialogueType DialogueType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerNPCDialogue(ENarr_NPCType NPCType, const FString& Context = "");

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueAudio(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopCurrentDialogue();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    TMap<FString, FNarr_StoryProgress> StoryProgressMap;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float DialogueStartTime;

private:
    void InitializeDefaultDialogues();
    void InitializeStoryProgression();
    bool CheckDialoguePrerequisites(const FNarr_DialogueEntry& DialogueEntry) const;
    void OnDialogueCompleted(const FString& DialogueID);
};