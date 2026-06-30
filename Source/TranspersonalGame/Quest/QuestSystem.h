#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "QuestSystem.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    ObjectivesMet   UMETA(DisplayName = "ObjectivesMet"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt            UMETA(DisplayName = "Hunt"),       // Kill specific creature
    Gather          UMETA(DisplayName = "Gather"),     // Collect resources
    Reach           UMETA(DisplayName = "Reach"),      // Arrive at location
    Survive         UMETA(DisplayName = "Survive"),    // Survive duration/event
    Escort          UMETA(DisplayName = "Escort"),     // Protect NPC to destination
    Scout           UMETA(DisplayName = "Scout"),      // Reveal map area / observe
    Craft           UMETA(DisplayName = "Craft")       // Craft specific item
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Scavenger       UMETA(DisplayName = "Scavenger"),  // Beginner — gather/explore
    Hunter          UMETA(DisplayName = "Hunter"),     // Intermediate — hunt small prey
    Apex            UMETA(DisplayName = "Apex"),       // Hard — large predators
    Extinction      UMETA(DisplayName = "Extinction") // Extreme — pack predators, stampedes
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
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
    FString TargetTag; // e.g. "raptor", "triceratops_alpha", "flint_rock"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation; // For Reach/Scout objectives

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius; // Acceptance radius in cm

    FQuest_Objective()
        : ObjectiveType(EQuest_ObjectiveType::Hunt)
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(500.0f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 BoneTokens; // Currency: bone tokens traded for crafting materials

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards; // e.g. "flint_spear", "hide_armor", "dried_meat"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ReputationGain; // Tribe reputation (0.0 - 100.0)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedRecipe; // Crafting recipe unlocked on completion

    FQuest_Reward()
        : BoneTokens(0)
        , ReputationGain(5.0f)
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
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimitSeconds; // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCTag; // Tag of the NPC who gives this quest

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_State State;

    FQuest_Definition()
        : Difficulty(EQuest_Difficulty::Hunter)
        , TimeLimitSeconds(0.0f)
        , bIsMainQuest(false)
        , State(EQuest_State::Inactive)
    {}
};

// ============================================================
// QUEST MANAGER COMPONENT
// ============================================================

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManagerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Quest lifecycle ----
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AcceptQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ReportProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckQuestComplete(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID, const FString& Reason);

    // ---- Query ----
    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestComplete(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FQuest_Definition> GetActiveQuests() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // ---- Registration ----
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Definition& QuestDef);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterDefaultQuests();

    // ---- Events (bind in Blueprint) ----
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAccepted, FString, QuestID);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestAccepted OnQuestAccepted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, FString, QuestID, FQuest_Reward, Reward);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestCompleted OnQuestCompleted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestFailed, FString, QuestID, FString, Reason);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestFailed OnQuestFailed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, FString, QuestID, FString, ObjectiveID, int32, NewCount);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnObjectiveUpdated OnObjectiveUpdated;

private:
    UPROPERTY()
    TMap<FString, FQuest_Definition> QuestRegistry;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    UPROPERTY()
    TArray<FString> CompletedQuestIDs;

    void GrantReward(const FQuest_Reward& Reward);
    void CheckTimeLimits(float DeltaTime);

    // Time tracking for timed quests
    TMap<FString, float> QuestTimers;
};

// ============================================================
// QUEST TRIGGER ACTOR — placed in world to start quests
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API AQuestTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    AQuestTriggerActor();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestIDToGive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAutoActivateOnOverlap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bOneTimeUse;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateQuest(AActor* Instigator);

private:
    UPROPERTY(VisibleAnywhere)
    USphereComponent* TriggerSphere;

    bool bUsed;

    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
