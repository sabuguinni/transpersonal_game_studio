#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurvivalQuestManager.generated.h"

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
    Survival    UMETA(DisplayName = "Survival"),
    Exploration UMETA(DisplayName = "Exploration"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Rescue      UMETA(DisplayName = "Rescue")
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
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , bCompleted(false)
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
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EQuest_Type::Hunt)
        , QuestState(EQuest_State::Inactive)
        , TimeLimit(0.0f)
        , bHasTimeLimit(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ASurvivalQuestManager : public AActor
{
    GENERATED_BODY()

public:
    ASurvivalQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Active quests registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Definition> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Definition> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MaxActiveQuests;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Definition GetQuestByID(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Definition> GetActiveQuests() const;

    // Predefined survival quests
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterRaptorPackHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterHerdMigrationWitnessQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterStampedeSurvivalQuest();

    // Event hooks from crowd simulation
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnRaptorFlankingTriggered();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnHerdStampedeTriggered();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnteredHerdZone();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerSurvivedStampede();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEscapedRaptors();

private:
    void InitializeDefaultQuests();
    bool FindQuestIndex(const FString& QuestID, int32& OutIndex) const;
    void CheckQuestCompletion(int32 QuestIndex);
    float ElapsedTime;
};
