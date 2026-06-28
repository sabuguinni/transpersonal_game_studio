#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "QuestSystemCore.generated.h"

// ============================================================
// QUEST SYSTEM CORE — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260628_004
// ============================================================

// ----------------------------------------------------------
// ENUMS — must be at global scope (UE5 compilation rule)
// ----------------------------------------------------------

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Locked      UMETA(DisplayName = "Locked"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation       UMETA(DisplayName = "Reach Location"),
    EliminateThreat     UMETA(DisplayName = "Eliminate Threat"),
    WarnNPC             UMETA(DisplayName = "Warn NPC"),
    CollectResource     UMETA(DisplayName = "Collect Resource"),
    ObserveCreature     UMETA(DisplayName = "Observe Creature"),
    EscapeArea          UMETA(DisplayName = "Escape Area"),
    ProtectNPC          UMETA(DisplayName = "Protect NPC"),
    FollowTrail         UMETA(DisplayName = "Follow Trail")
};

UENUM(BlueprintType)
enum class EQuest_TriggerType : uint8
{
    ProximityEnter      UMETA(DisplayName = "Proximity Enter"),
    ProximityExit       UMETA(DisplayName = "Proximity Exit"),
    InteractNPC         UMETA(DisplayName = "Interact NPC"),
    TimedEvent          UMETA(DisplayName = "Timed Event"),
    CreatureEvent       UMETA(DisplayName = "Creature Event")
};

// ----------------------------------------------------------
// STRUCTS — must be at global scope
// ----------------------------------------------------------

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
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ProximityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , ProximityRadius(300.0f)
        , bIsCompleted(false)
        , RequiredCount(1)
        , CurrentCount(0)
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
    EQuest_State State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardDescription;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , State(EQuest_State::Locked)
        , TimeLimit(0.0f)
        , bHasTimeLimit(false)
        , RewardDescription(TEXT(""))
    {}
};

// ----------------------------------------------------------
// AQuestTriggerVolume — proximity-based quest trigger
// ----------------------------------------------------------

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestTriggerVolume : public AActor
{
    GENERATED_BODY()

public:
    AQuestTriggerVolume();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString LinkedQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString LinkedObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bSingleUse;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bHasTriggered;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateTrigger();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsPlayerInRange() const;

protected:
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// ----------------------------------------------------------
// AQuestManager — master quest state machine
// ----------------------------------------------------------

UCLASS(ClassGroup = (Quest))
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Active quest registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Definition> QuestRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString ActiveQuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 TotalQuestsCompleted;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Definition GetQuestDefinition(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Definition& QuestDef);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetActiveQuestIDs() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // Time-limit tracking
    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetQuestTimeRemaining(const FString& QuestID) const;

private:
    TMap<FString, float> QuestStartTimes;
    TMap<FString, float> QuestElapsedTimes;

    void CheckTimeLimits(float DeltaTime);
    bool AreAllObjectivesComplete(const FQuest_Definition& Quest) const;
    int32 FindQuestIndex(const FString& QuestID) const;
};
