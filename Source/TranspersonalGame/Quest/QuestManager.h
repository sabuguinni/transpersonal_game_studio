#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestManager.h — Survival quest system for prehistoric world
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Explore     UMETA(DisplayName = "Explore"),
    Defend      UMETA(DisplayName = "Defend"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Craft       UMETA(DisplayName = "Craft"),
    Track       UMETA(DisplayName = "Track")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "ReachLocation"),
    CollectItem     UMETA(DisplayName = "CollectItem"),
    KillTarget      UMETA(DisplayName = "KillTarget"),
    SurviveTime     UMETA(DisplayName = "SurviveTime"),
    CraftItem       UMETA(DisplayName = "CraftItem"),
    EscapeArea      UMETA(DisplayName = "EscapeArea"),
    ProtectNPC      UMETA(DisplayName = "ProtectNPC")
};

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , Description(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
        , TargetLocation(FVector::ZeroVector)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString GiverNPCLabel;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , Description(TEXT(""))
        , QuestType(EQuest_Type::Gather)
        , QuestState(EQuest_State::Inactive)
        , GiverNPCLabel(TEXT(""))
        , bIsMainQuest(false)
    {}
};

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    // Query
    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // Registration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    // Built-in survival quests
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterDefaultSurvivalQuests();

private:
    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    void InitDefaultQuests();
};
