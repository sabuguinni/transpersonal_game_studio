#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Quest_AdaptiveQuestController.generated.h"

UENUM(BlueprintType)
enum class EQuest_QuestDifficulty : uint8
{
    Beginner    UMETA(DisplayName = "Beginner"),
    Intermediate UMETA(DisplayName = "Intermediate"),
    Advanced    UMETA(DisplayName = "Advanced"),
    Expert      UMETA(DisplayName = "Expert")
};

UENUM(BlueprintType)
enum class EQuest_QuestCategory : uint8
{
    Survival    UMETA(DisplayName = "Survival"),
    Exploration UMETA(DisplayName = "Exploration"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Defense     UMETA(DisplayName = "Defense"),
    Rescue      UMETA(DisplayName = "Rescue")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_AdaptiveQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_QuestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_QuestDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FQuest_AdaptiveQuestData()
    {
        QuestName = TEXT("Unknown Quest");
        QuestDescription = TEXT("No description available");
        Category = EQuest_QuestCategory::Survival;
        Difficulty = EQuest_QuestDifficulty::Beginner;
        TargetLocation = FVector::ZeroVector;
        TimeLimit = 300.0f;
        RewardExperience = 100;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_PlayerSkillData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SurvivalLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HuntingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CraftingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExplorationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuccessRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 QuestsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageCompletionTime;

    FQuest_PlayerSkillData()
    {
        SurvivalLevel = 1;
        HuntingLevel = 1;
        CraftingLevel = 1;
        ExplorationLevel = 1;
        SuccessRate = 0.5f;
        QuestsCompleted = 0;
        AverageCompletionTime = 600.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_AdaptiveQuestController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_AdaptiveQuestController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_AdaptiveQuestData> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_AdaptiveQuestData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FQuest_PlayerSkillData PlayerSkills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float QuestGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float DifficultyAdaptationRate;

    FTimerHandle QuestGenerationTimer;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateAdaptiveQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdatePlayerSkills(EQuest_QuestCategory Category, bool bSuccess, float CompletionTime);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    EQuest_QuestDifficulty CalculateOptimalDifficulty(EQuest_QuestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_AdaptiveQuestData CreateSurvivalQuest(EQuest_QuestDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_AdaptiveQuestData CreateHuntingQuest(EQuest_QuestDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_AdaptiveQuestData CreateExplorationQuest(EQuest_QuestDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_AdaptiveQuestData CreateCraftingQuest(EQuest_QuestDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FQuest_AdaptiveQuestData& Quest);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestName, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CancelQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_AdaptiveQuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AnalyzePlayerPerformance();

private:
    void InitializeQuestTemplates();
    FVector GetSavanaLocation();
    FVector GetForestLocation();
    FVector GetSwampLocation();
    FVector GetDesertLocation();
    FVector GetMountainLocation();
};