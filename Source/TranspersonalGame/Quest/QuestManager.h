// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260620_008
// Quest system manager: tracks active quests, objectives, and NPC interactions

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
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
    Defend      UMETA(DisplayName = "Defend"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Gather      UMETA(DisplayName = "Gather"),
    Explore     UMETA(DisplayName = "Explore"),
    Follow      UMETA(DisplayName = "Follow")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    ProtectNPC      UMETA(DisplayName = "Protect NPC"),
    FollowNPC       UMETA(DisplayName = "Follow NPC"),
    SurviveTime     UMETA(DisplayName = "Survive Time")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

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
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
        , TargetLocation(FVector::ZeroVector)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
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
    EQuest_State QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverNPCLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , Description(TEXT(""))
        , QuestType(EQuest_Type::Hunt)
        , QuestState(EQuest_State::Inactive)
        , QuestGiverNPCLabel(TEXT(""))
        , TimeLimit(0.0f)
        , ElapsedTime(0.0f)
        , bHasTimeLimit(false)
    {}
};

// ============================================================
// UCLASS
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // Tick for time-limited quests
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TickQuests(float DeltaTime);

    // Crowd integration: alert tribe NPCs when quest triggers
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyQuestEvent(const FString& QuestID, const FString& EventType, const FVector& EventLocation);

private:
    void RegisterDefaultQuests();

    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;
};
