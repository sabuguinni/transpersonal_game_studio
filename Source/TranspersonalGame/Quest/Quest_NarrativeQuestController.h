#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Quest_NarrativeQuestController.generated.h"

UENUM(BlueprintType)
enum class EQuest_NarrativeType : uint8
{
    Discovery       UMETA(DisplayName = "Discovery Quest"),
    Investigation   UMETA(DisplayName = "Investigation Quest"),
    Social          UMETA(DisplayName = "Social Quest"),
    Ritual          UMETA(DisplayName = "Ritual Quest"),
    Challenge       UMETA(DisplayName = "Challenge Quest"),
    Survival        UMETA(DisplayName = "Survival Quest"),
    Exploration     UMETA(DisplayName = "Exploration Quest"),
    Combat          UMETA(DisplayName = "Combat Quest")
};

UENUM(BlueprintType)
enum class EQuest_StoryBeat : uint8
{
    Introduction    UMETA(DisplayName = "Story Introduction"),
    RisingAction    UMETA(DisplayName = "Rising Action"),
    Climax          UMETA(DisplayName = "Story Climax"),
    FallingAction   UMETA(DisplayName = "Falling Action"),
    Resolution      UMETA(DisplayName = "Story Resolution"),
    Epilogue        UMETA(DisplayName = "Story Epilogue")
};

UENUM(BlueprintType)
enum class EQuest_EmotionalTone : uint8
{
    Neutral         UMETA(DisplayName = "Neutral Tone"),
    Hopeful         UMETA(DisplayName = "Hopeful Tone"),
    Tense           UMETA(DisplayName = "Tense Tone"),
    Mysterious      UMETA(DisplayName = "Mysterious Tone"),
    Triumphant      UMETA(DisplayName = "Triumphant Tone"),
    Melancholic     UMETA(DisplayName = "Melancholic Tone"),
    Urgent          UMETA(DisplayName = "Urgent Tone"),
    Peaceful        UMETA(DisplayName = "Peaceful Tone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NarrativeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EQuest_NarrativeType NarrativeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EQuest_StoryBeat CurrentStoryBeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EQuest_EmotionalTone EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> ObjectiveTexts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float EmotionalImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsMainStoryQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 StoryChapter;

    FQuest_NarrativeData()
    {
        QuestTitle = TEXT("Untitled Quest");
        QuestDescription = TEXT("No description available");
        NarrativeType = EQuest_NarrativeType::Discovery;
        CurrentStoryBeat = EQuest_StoryBeat::Introduction;
        EmotionalTone = EQuest_EmotionalTone::Neutral;
        EmotionalImpact = 0.5f;
        bIsMainStoryQuest = false;
        StoryChapter = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CharacterArc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    FString InitialState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    FString DesiredOutcome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    float ProgressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    TArray<FString> CharacterMotivations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    bool bArcCompleted;

    FQuest_CharacterArc()
    {
        CharacterName = TEXT("Unknown Character");
        InitialState = TEXT("Starting point");
        DesiredOutcome = TEXT("Goal state");
        ProgressionLevel = 0.0f;
        bArcCompleted = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_NarrativeQuestController : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_NarrativeQuestController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core narrative quest management
    UFUNCTION(BlueprintCallable, Category = "Narrative Quest")
    void InitializeNarrativeQuest(const FQuest_NarrativeData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Narrative Quest")
    void AdvanceStoryBeat();

    UFUNCTION(BlueprintCallable, Category = "Narrative Quest")
    void UpdateEmotionalTone(EQuest_EmotionalTone NewTone);

    UFUNCTION(BlueprintCallable, Category = "Narrative Quest")
    void TriggerNarrativeEvent(const FString& EventName);

    // Character arc management
    UFUNCTION(BlueprintCallable, Category = "Character Arc")
    void RegisterCharacterArc(const FQuest_CharacterArc& CharacterArc);

    UFUNCTION(BlueprintCallable, Category = "Character Arc")
    void UpdateCharacterProgression(const FString& CharacterName, float ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Character Arc")
    bool IsCharacterArcComplete(const FString& CharacterName) const;

    // Dynamic narrative generation
    UFUNCTION(BlueprintCallable, Category = "Dynamic Narrative")
    FQuest_NarrativeData GenerateContextualQuest(EQuest_NarrativeType PreferredType);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Narrative")
    void AdaptQuestToPlayerActions(const TArray<FString>& PlayerActions);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Narrative")
    void EvaluateEmotionalImpact();

    // Narrative validation and quality
    UFUNCTION(BlueprintCallable, Category = "Narrative Quality")
    float CalculateNarrativeCoherence() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Quality")
    bool ValidateQuestProgression() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Quality")
    void OptimizeNarrativeFlow();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Narrative Quest")
    FQuest_NarrativeData GetCurrentNarrativeData() const { return CurrentNarrativeData; }

    UFUNCTION(BlueprintPure, Category = "Narrative Quest")
    EQuest_StoryBeat GetCurrentStoryBeat() const { return CurrentNarrativeData.CurrentStoryBeat; }

    UFUNCTION(BlueprintPure, Category = "Narrative Quest")
    float GetEmotionalImpact() const { return CurrentNarrativeData.EmotionalImpact; }

protected:
    // Core narrative data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative State")
    FQuest_NarrativeData CurrentNarrativeData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Arcs")
    TArray<FQuest_CharacterArc> ActiveCharacterArcs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    float NarrativeUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    float EmotionalImpactThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    bool bEnableDynamicNarrative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    int32 MaxConcurrentArcs;

    // Internal state
    UPROPERTY()
    float LastNarrativeUpdate;

    UPROPERTY()
    TArray<FString> NarrativeEventHistory;

    UPROPERTY()
    TMap<FString, float> CharacterProgressionMap;

private:
    // Helper functions
    void ProcessNarrativeEvents();
    void UpdateCharacterArcs(float DeltaTime);
    void ValidateNarrativeConsistency();
    FString GenerateContextualDialogue(EQuest_EmotionalTone Tone) const;
    void LogNarrativeEvent(const FString& Event);
};