#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "NarrativeFramework.generated.h"

// Story progression stages
UENUM(BlueprintType)
enum class ENarr_StoryStage : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstContact    UMETA(DisplayName = "First Contact"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    GreatMigration  UMETA(DisplayName = "Great Migration"),
    ApexPredator    UMETA(DisplayName = "Apex Predator"),
    Survival        UMETA(DisplayName = "Survival")
};

// Narrative trigger types
UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    LocationBased   UMETA(DisplayName = "Location Based"),
    QuestCompletion UMETA(DisplayName = "Quest Completion"),
    DinosaurEncounter UMETA(DisplayName = "Dinosaur Encounter"),
    SurvivalMilestone UMETA(DisplayName = "Survival Milestone"),
    TimeProgression UMETA(DisplayName = "Time Progression")
};

// Story beat data structure
USTRUCT(BlueprintType)
struct FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryStage StoryStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> PrerequisiteBeatIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        Title = FText::GetEmpty();
        Description = FText::GetEmpty();
        StoryStage = ENarr_StoryStage::Awakening;
        TriggerType = ENarr_TriggerType::LocationBased;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        bIsCompleted = false;
    }
};

// Dialogue context for dynamic responses
USTRUCT(BlueprintType)
struct FNarr_DialogueContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioClipPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float PlaybackDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsNarration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ContextTags;

    FNarr_DialogueContext()
    {
        SpeakerID = TEXT("Narrator");
        DialogueText = FText::GetEmpty();
        AudioClipPath = TEXT("");
        PlaybackDelay = 0.0f;
        bIsNarration = true;
    }
};

/**
 * Central narrative framework that manages story progression, dialogue delivery,
 * and integration with quest and crowd simulation systems.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeFramework();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeStoryFramework();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryStage(ENarr_StoryStage NewStage);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryStage GetCurrentStoryStage() const { return CurrentStoryStage; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    // Location-based narrative triggers
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckLocationTriggers(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterLocationTrigger(const FNarr_StoryBeat& StoryBeat);

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestCompleted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnCrowdInteractionCompleted(const FString& InteractionType, int32 HerdSize);

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FNarr_DialogueContext& DialogueContext);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarration(const FString& NarrationID);

    // Dinosaur encounter narratives
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDinosaurEncounter(const FString& DinosaurType, const FVector& Location, bool bIsHostile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnTerritorialBehaviorObserved(const FString& DinosaurType, const FVector& TerritoryCenter);

    // Survival milestone narratives
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnSurvivalMilestone(const FString& MilestoneType, float Value);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnMigrationEventTriggered(const FString& HerdType, const FVector& MigrationStart, const FVector& MigrationEnd);

protected:
    // Current story state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    ENarr_StoryStage CurrentStoryStage;

    // Story beats database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_StoryBeat> StoryBeats;

    // Completed story beats
    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TSet<FString> CompletedBeats;

    // Active dialogue queue
    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_DialogueContext> DialogueQueue;

    // Location triggers
    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_StoryBeat> LocationTriggers;

private:
    // Internal story beat processing
    void ProcessStoryBeat(const FNarr_StoryBeat& StoryBeat);
    void ValidateStoryBeatPrerequisites(const FNarr_StoryBeat& StoryBeat, bool& bCanTrigger);
    void CreateDefaultStoryBeats();
    void SetupBiomeNarratives();
    void SetupDinosaurEncounterNarratives();
    void SetupSurvivalMilestoneNarratives();

    // Dialogue processing
    void ProcessDialogueQueue();
    FTimerHandle DialogueTimerHandle;
};