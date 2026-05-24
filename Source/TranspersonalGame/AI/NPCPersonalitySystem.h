#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "NPCPersonalitySystem.generated.h"

// NPC Personality Traits
UENUM(BlueprintType)
enum class ENPC_PersonalityTrait : uint8
{
    Curious         UMETA(DisplayName = "Curious"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary"),
    Wise            UMETA(DisplayName = "Wise"),
    Impulsive       UMETA(DisplayName = "Impulsive"),
    Protective      UMETA(DisplayName = "Protective"),
    Adventurous     UMETA(DisplayName = "Adventurous")
};

// NPC Emotional States
UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Happy           UMETA(DisplayName = "Happy"),
    Sad             UMETA(DisplayName = "Sad"),
    Angry           UMETA(DisplayName = "Angry"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Excited         UMETA(DisplayName = "Excited"),
    Confused        UMETA(DisplayName = "Confused"),
    Focused         UMETA(DisplayName = "Focused"),
    Relaxed         UMETA(DisplayName = "Relaxed"),
    Stressed        UMETA(DisplayName = "Stressed")
};

// NPC Social Roles
UENUM(BlueprintType)
enum class ENPC_SocialRole : uint8
{
    Elder           UMETA(DisplayName = "Elder"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Guard           UMETA(DisplayName = "Guard"),
    Scout           UMETA(DisplayName = "Scout"),
    Child           UMETA(DisplayName = "Child"),
    Storyteller     UMETA(DisplayName = "Storyteller"),
    Healer          UMETA(DisplayName = "Healer")
};

// Daily Activity States
UENUM(BlueprintType)
enum class ENPC_DailyActivity : uint8
{
    Sleeping        UMETA(DisplayName = "Sleeping"),
    WakingUp        UMETA(DisplayName = "Waking Up"),
    MorningRitual   UMETA(DisplayName = "Morning Ritual"),
    Working         UMETA(DisplayName = "Working"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Eating          UMETA(DisplayName = "Eating"),
    Resting         UMETA(DisplayName = "Resting"),
    EveningGathering UMETA(DisplayName = "Evening Gathering"),
    Storytelling    UMETA(DisplayName = "Storytelling"),
    PreparingSleep  UMETA(DisplayName = "Preparing Sleep")
};

// Personality Profile Structure
USTRUCT(BlueprintType)
struct FNPC_PersonalityProfile : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    ENPC_SocialRole SocialRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENPC_PersonalityTrait> PrimaryTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENPC_PersonalityTrait> SecondaryTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float SocialTendency; // 0.0 = Very Solitary, 1.0 = Very Social

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float AggressionLevel; // 0.0 = Very Peaceful, 1.0 = Very Aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float CuriosityLevel; // 0.0 = Very Cautious, 1.0 = Very Curious

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float WisdomLevel; // 0.0 = Very Impulsive, 1.0 = Very Wise

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> PreferredActivities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> DislikedActivities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString BackgroundStory;

    FNPC_PersonalityProfile()
    {
        NPCName = TEXT("Unknown");
        SocialRole = ENPC_SocialRole::Gatherer;
        SocialTendency = 0.5f;
        AggressionLevel = 0.3f;
        CuriosityLevel = 0.5f;
        WisdomLevel = 0.4f;
        BackgroundStory = TEXT("A member of the tribe with their own unique story.");
    }
};

// Memory Entry for NPC experiences
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector EventLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EventTime; // Game time when event occurred

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_EmotionalState AssociatedEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalIntensity; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FString> InvolvedCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsPositiveMemory;

    FNPC_MemoryEntry()
    {
        EventDescription = TEXT("");
        EventLocation = FVector::ZeroVector;
        EventTime = 0.0f;
        AssociatedEmotion = ENPC_EmotionalState::Calm;
        EmotionalIntensity = 0.5f;
        bIsPositiveMemory = true;
    }
};

/**
 * NPC Personality System Component
 * Manages personality traits, emotional states, and memory for NPCs
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PersonalitySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PersonalitySystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Personality Profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FNPC_PersonalityProfile PersonalityProfile;

    // Current Emotional State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    ENPC_EmotionalState CurrentEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionalIntensity;

    // Daily Activity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Life")
    ENPC_DailyActivity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Life")
    float ActivityStartTime;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> RecentMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries;

    // Social Relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<FString, float> RelationshipScores; // NPC Name -> Relationship Score (-1.0 to 1.0)

    // Behavior Modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StressLevel; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float EnergyLevel; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel; // 0.0 to 1.0

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Personality")
    void SetPersonalityProfile(const FNPC_PersonalityProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void ChangeEmotionalState(ENPC_EmotionalState NewEmotion, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryEntry(const FString& Description, const FVector& Location, ENPC_EmotionalState Emotion, float Intensity, bool bPositive = true);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyRelationship(const FString& NPCName, float ChangeAmount);

    UFUNCTION(BlueprintCallable, Category = "Social")
    float GetRelationshipScore(const FString& NPCName) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldPerformActivity(const FString& ActivityName) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float GetPersonalityInfluence(ENPC_PersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "Daily Life")
    void UpdateDailyActivity(ENPC_DailyActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Daily Life")
    ENPC_DailyActivity GetRecommendedActivity(float CurrentGameTime) const;

private:
    // Internal functions
    void UpdateEmotionalDecay(float DeltaTime);
    void UpdateNeeds(float DeltaTime);
    void ProcessMemoryDecay();
    bool HasPersonalityTrait(ENPC_PersonalityTrait Trait) const;
};