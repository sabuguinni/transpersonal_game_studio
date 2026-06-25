#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 RULE 1)
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
    Track       UMETA(DisplayName = "Track"),
    Craft       UMETA(DisplayName = "Craft"),
    Defend      UMETA(DisplayName = "Defend"),
    Explore     UMETA(DisplayName = "Explore"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Ambush      UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    SurviveTime     UMETA(DisplayName = "Survive Time"),
    EscapeZone      UMETA(DisplayName = "Escape Zone"),
    FollowTarget    UMETA(DisplayName = "Follow Target")
};

// ============================================================
// STRUCTS — must be at global scope (UE5 RULE 1)
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
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , TargetRadius(400.f)
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
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
    EQuest_State State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , Description(TEXT(""))
        , QuestType(EQuest_Type::Explore)
        , State(EQuest_State::Inactive)
        , RewardDescription(TEXT(""))
        , bIsMainQuest(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString OutputItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TMap<FString, int32> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTime;

    FQuest_CraftingRecipe()
        : RecipeID(TEXT(""))
        , OutputItemName(TEXT(""))
        , CraftingTime(3.0f)
    {}
};

// ============================================================
// QUEST MANAGER ACTOR
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame")
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Quest Registry ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> CompletedQuestIDs;

    // ---- Crafting ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    TArray<FQuest_CraftingRecipe> CraftingRecipes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Crafting")
    TMap<FString, int32> PlayerInventory;

    // ---- Settings ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Settings")
    float ObjectiveCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Settings")
    float QuestTriggerRadius;

    // ---- Quest Functions ----
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // ---- Crafting Functions ----
    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CanCraftItem(const FString& RecipeID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CraftItem(const FString& RecipeID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void AddResourceToInventory(const FString& ResourceName, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    int32 GetResourceCount(const FString& ResourceName) const;

    // ---- Location Check ----
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckPlayerLocationObjectives(FVector PlayerLocation);

    UFUNCTION(CallInEditor, Category = "Quest|Debug")
    void PrintQuestStatus();

private:
    void InitializeDefaultQuests();
    void InitializeCraftingRecipes();
    void CheckQuestCompletion(FQuest_Data& Quest);

    float TimeSinceLastCheck;
};
