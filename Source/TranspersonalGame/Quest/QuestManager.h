#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestManager.h — Survival quest system for prehistoric world
// PROD_CYCLE_AUTO_20260620_003
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Survival    UMETA(DisplayName = "Survival"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Explore     UMETA(DisplayName = "Explore"),
    Craft       UMETA(DisplayName = "Craft"),
    Escape      UMETA(DisplayName = "Escape"),
    Defend      UMETA(DisplayName = "Defend")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "ReachLocation"),
    CollectItem     UMETA(DisplayName = "CollectItem"),
    KillTarget      UMETA(DisplayName = "KillTarget"),
    SurviveTime     UMETA(DisplayName = "SurviveTime"),
    CraftItem       UMETA(DisplayName = "CraftItem"),
    EscapeZone      UMETA(DisplayName = "EscapeZone")
};

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(200.0f)
        , bCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeElapsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , Title(TEXT(""))
        , Description(TEXT(""))
        , QuestType(EQuest_Type::Survival)
        , Status(EQuest_Status::Inactive)
        , TimeLimit(0.0f)
        , TimeElapsed(0.0f)
        , bHasTimeLimit(false)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestActivated, FString, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FString, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, FString, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FString, QuestID, FString, ObjectiveID);

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckLocationObjective(const FString& QuestID, const FString& ObjectiveID, FVector PlayerLocation);

    // Query
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool GetQuestDefinition(const FString& QuestID, FQuest_Definition& OutQuest) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FString> GetActiveQuestIDs() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_Status GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    // Tick
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TickQuests(float DeltaTime);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestActivated OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnObjectiveUpdated OnObjectiveUpdated;

private:
    UPROPERTY()
    TMap<FString, FQuest_Definition> QuestRegistry;

    void RegisterSurvivalQuests();
    void RegisterHuntQuests();
    void RegisterCraftQuests();
    bool AreAllObjectivesComplete(const FQuest_Definition& Quest) const;
    void CheckObjectiveCompletion(FQuest_Definition& Quest, FQuest_Objective& Objective);
};
