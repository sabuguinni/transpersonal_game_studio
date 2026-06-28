// QuestSystemTypes.h
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260628_003
// Shared quest types: enums, structs for the survival quest system.
// All types prefixed with Quest_ to avoid collision with other agents.

#pragma once

#include "CoreMinimal.h"
#include "QuestSystemTypes.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — Quest state machine
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Locked          UMETA(DisplayName = "Locked"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    ObjectiveReached UMETA(DisplayName = "ObjectiveReached"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Survival        UMETA(DisplayName = "Survival"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Defend          UMETA(DisplayName = "Defend"),
    Explore         UMETA(DisplayName = "Explore"),
    Craft           UMETA(DisplayName = "Craft"),
    Migrate         UMETA(DisplayName = "Migrate")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "ReachLocation"),
    TalkToNPC       UMETA(DisplayName = "TalkToNPC"),
    KillTarget      UMETA(DisplayName = "KillTarget"),
    CollectItem     UMETA(DisplayName = "CollectItem"),
    EscapeArea      UMETA(DisplayName = "EscapeArea"),
    ProtectNPC      UMETA(DisplayName = "ProtectNPC"),
    SurviveTime     UMETA(DisplayName = "SurviveTime")
};

UENUM(BlueprintType)
enum class EQuest_TriggerEvent : uint8
{
    None            UMETA(DisplayName = "None"),
    StampedeBegins  UMETA(DisplayName = "StampedeBegins"),
    PredatorNearby  UMETA(DisplayName = "PredatorNearby"),
    NPCSeparated    UMETA(DisplayName = "NPCSeparated"),
    CampAttacked    UMETA(DisplayName = "CampAttacked"),
    PlayerEntersZone UMETA(DisplayName = "PlayerEntersZone"),
    TalkToQuestGiver UMETA(DisplayName = "TalkToQuestGiver")
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — Quest data
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    EQuest_ObjectiveType ObjectiveType = EQuest_ObjectiveType::ReachLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float ProximityRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Objective")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Objective")
    bool bCompleted = false;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , ProximityRadius(300.0f)
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 ResourceFlint = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 ResourceHide = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 ResourceBone = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    float HealthRestore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FString UnlocksRecipeID;

    FQuest_Reward()
        : ResourceFlint(0)
        , ResourceHide(0)
        , ResourceBone(0)
        , HealthRestore(0.0f)
        , UnlocksRecipeID(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType = EQuest_Type::Survival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_TriggerEvent TriggerEvent = EQuest_TriggerEvent::TalkToQuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f;   // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit = false;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_State State = EQuest_State::Locked;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EQuest_Type::Survival)
        , TriggerEvent(EQuest_TriggerEvent::TalkToQuestGiver)
        , TimeLimit(0.0f)
        , bHasTimeLimit(false)
        , State(EQuest_State::Locked)
    {}
};
