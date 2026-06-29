#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest system enums — Quest_ prefix to avoid conflicts
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive     UMETA(DisplayName = "Inactive"),
    Active       UMETA(DisplayName = "Active"),
    Completed    UMETA(DisplayName = "Completed"),
    Failed       UMETA(DisplayName = "Failed"),
    Locked       UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt         UMETA(DisplayName = "Hunt"),
    Gather       UMETA(DisplayName = "Gather"),
    Survive      UMETA(DisplayName = "Survive"),
    Escort       UMETA(DisplayName = "Escort"),
    Explore      UMETA(DisplayName = "Explore"),
    Craft        UMETA(DisplayName = "Craft"),
    Defend       UMETA(DisplayName = "Defend"),
    Track        UMETA(DisplayName = "Track")
};

UENUM(BlueprintType)
enum class EQuest_Category : uint8
{
    MainStory    UMETA(DisplayName = "Main Story"),
    Survival     UMETA(DisplayName = "Survival"),
    Hunting      UMETA(DisplayName = "Hunting"),
    Exploration  UMETA(DisplayName = "Exploration"),
    Crafting     UMETA(DisplayName = "Crafting"),
    Tribal       UMETA(DisplayName = "Tribal")
};

// ============================================================
// Quest objective struct
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
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TargetRadius;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::Hunt)
        , TargetTag(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
        , TargetLocation(FVector::ZeroVector)
        , TargetRadius(500.0f)
    {}
};

// ============================================================
// Quest reward struct
// ============================================================

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 FlintStones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Hides;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Bones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Meat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float XPBonus;

    FQuest_Reward()
        : FlintStones(0)
        , Hides(0)
        , Bones(0)
        , Meat(0)
        , XPBonus(0.0f)
    {}
};

// ============================================================
// Quest data struct
// ============================================================

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Category Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , Category(EQuest_Category::Survival)
        , State(EQuest_State::Inactive)
        , GiverNPCTag(TEXT(""))
        , bIsRepeatable(false)
        , TimeLimit(0.0f)
    {}
};

// ============================================================
// Quest Manager Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Quest lifecycle ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AbandonQuest(const FString& QuestID);

    // ---- Objective tracking ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyKill(const FString& TargetTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyGather(const FString& ItemTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyLocationReached(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyCraft(const FString& ItemTag);

    // ---- Herd integration ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnStampedeTriggered(FVector StampedeOrigin);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnHerdStateChanged(const FString& HerdSpecies, const FString& NewState);

    // ---- Query ----

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ---- Data ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bDebugLog;

private:
    void InitializeDefaultQuests();
    void CheckObjectiveCompletion(FQuest_Data& Quest);
    void GrantReward(const FQuest_Reward& Reward);
    FQuest_Data* FindQuestByID(const FString& QuestID);

    float TimeSinceLastTick;
};
