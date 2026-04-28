#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "QuestTypes.generated.h"

// Quest system types for prehistoric survival game
// Focus: Hunting, gathering, exploration, crafting, survival

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Explore     UMETA(DisplayName = "Explore"),
    Craft       UMETA(DisplayName = "Craft"),
    Survive     UMETA(DisplayName = "Survive"),
    Escort      UMETA(DisplayName = "Escort"),
    Defend      UMETA(DisplayName = "Defend"),
    Trade       UMETA(DisplayName = "Trade")
};

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Abandoned   UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Easy        UMETA(DisplayName = "Easy"),
    Medium      UMETA(DisplayName = "Medium"),
    Hard        UMETA(DisplayName = "Hard"),
    Extreme     UMETA(DisplayName = "Extreme")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    TalkToNPC       UMETA(DisplayName = "Talk To NPC"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    SurviveTime     UMETA(DisplayName = "Survive Time"),
    DefendArea      UMETA(DisplayName = "Defend Area"),
    EscortNPC       UMETA(DisplayName = "Escort NPC")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_Objective()
    {
        Type = EQuest_ObjectiveType::KillTarget;
        Description = TEXT("Default Objective");
        TargetName = TEXT("");
        RequiredCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        Radius = 500.0f;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    FQuest_Reward()
    {
        ItemName = TEXT("Stone");
        Quantity = 1;
        ExperiencePoints = 10;
        Description = TEXT("Basic reward");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Reward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MinLevel;

    FQuest_Data()
    {
        QuestID = TEXT("QUEST_001");
        QuestName = TEXT("Hunt the Beast");
        Description = TEXT("Eliminate dangerous predators threatening the tribe");
        QuestType = EQuest_Type::Hunt;
        Status = EQuest_Status::NotStarted;
        Difficulty = EQuest_Difficulty::Medium;
        QuestGiverName = TEXT("Elder Grok");
        TimeLimit = 0.0f;
        bHasTimeLimit = false;
        MinLevel = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC")
    FString Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC")
    TArray<FString> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC")
    FString Greeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC")
    bool bIsActive;

    FQuest_NPCData()
    {
        NPCName = TEXT("Unnamed NPC");
        Role = TEXT("Villager");
        Location = FVector::ZeroVector;
        Greeting = TEXT("Hello, traveler.");
        bIsActive = true;
    }
};