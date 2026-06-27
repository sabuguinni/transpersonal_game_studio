#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

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
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Defend      UMETA(DisplayName = "Defend"),
    Track       UMETA(DisplayName = "Track"),
    Explore     UMETA(DisplayName = "Explore"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Craft       UMETA(DisplayName = "Craft")
};

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType = EQuest_Type::Hunt;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status = EQuest_Status::Inactive;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString RewardDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FVector QuestZoneLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    float QuestZoneRadius = 1000.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Quest registry
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Definition> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FString> ActiveQuestIDs;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Definition GetQuestByID(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Definition> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest")
    void RegisterDefaultQuests();

private:
    void InitializeQuestRegistry();
    FQuest_Definition BuildHuntRexQuest() const;
    FQuest_Definition BuildGatherResourcesQuest() const;
    FQuest_Definition BuildDefendCampQuest() const;
    FQuest_Definition BuildTrackRaptorsQuest() const;
    FQuest_Definition BuildWaterSourceQuest() const;
};
