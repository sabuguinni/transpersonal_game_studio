#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "QuestSystemCore.generated.h"

// ============================================================
// Enums — must be at global scope (UE5 UHT requirement)
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
enum class EQuest_ObjectiveType : uint8
{
    Reach       UMETA(DisplayName = "Reach Location"),
    Kill        UMETA(DisplayName = "Kill Target"),
    Collect     UMETA(DisplayName = "Collect Item"),
    Survive     UMETA(DisplayName = "Survive Duration"),
    Destroy     UMETA(DisplayName = "Destroy Object"),
    Escort      UMETA(DisplayName = "Escort NPC")
};

// ============================================================
// Structs — must be at global scope
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

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::Reach)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedArea;

    FQuest_Reward()
        : ExperiencePoints(0)
        , UnlockedArea(TEXT(""))
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
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , Description(TEXT(""))
        , Status(EQuest_Status::Inactive)
        , bIsMainQuest(false)
    {}
};

// ============================================================
// AQuestTriggerVolume — placed in world to start/advance quests
// ============================================================

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestTriggerVolume : public AActor
{
    GENERATED_BODY()

public:
    AQuestTriggerVolume();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bStartsQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompletesObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bOneTimeUse;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    bool bHasTriggered;
};

// ============================================================
// UQuestManagerComponent — attach to GameState or GameMode
// ============================================================

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManagerComponent();

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Data> ActiveQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Data> CompletedQuests;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuestDefinition(const FQuest_Data& QuestDef);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest")
    void InitializeDefaultQuests();

protected:
    virtual void BeginPlay() override;

private:
    TArray<FQuest_Data> QuestDefinitions;

    FQuest_Data* FindActiveQuest(const FString& QuestID);
    void CheckQuestCompletion(FQuest_Data& Quest);
    void BuildRaptorDenQuest();
    void BuildFirstHuntQuest();
    void BuildSurvivalNightQuest();
};
