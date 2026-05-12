#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Quest_SurvivalMissionController.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    ResourceCollection    UMETA(DisplayName = "Resource Collection"),
    CraftingChallenge    UMETA(DisplayName = "Crafting Challenge"),
    HuntingMission       UMETA(DisplayName = "Hunting Mission"),
    ShelterBuilding      UMETA(DisplayName = "Shelter Building"),
    ExplorationQuest     UMETA(DisplayName = "Exploration Quest"),
    SurvivalChallenge    UMETA(DisplayName = "Survival Challenge")
};

UENUM(BlueprintType)
enum class EQuest_MissionDifficulty : uint8
{
    Beginner    UMETA(DisplayName = "Beginner"),
    Intermediate UMETA(DisplayName = "Intermediate"),
    Advanced    UMETA(DisplayName = "Advanced"),
    Expert      UMETA(DisplayName = "Expert"),
    Master      UMETA(DisplayName = "Master")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<int32> ResourceQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("Default Mission");
        MissionDescription = TEXT("Complete this survival challenge");
        MissionType = EQuest_SurvivalMissionType::ResourceCollection;
        Difficulty = EQuest_MissionDifficulty::Beginner;
        TimeLimit = 300.0f; // 5 minutes
        ExperienceReward = 100;
        bIsCompleted = false;
        bIsActive = false;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    FQuest_NPCDialogue()
    {
        NPCName = TEXT("Unknown NPC");
        DialogueText = TEXT("Hello, survivor.");
        bIsQuestRelated = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalMissionController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalMissionController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_NPCDialogue> NPCDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 PlayerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 PlayerExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float MissionGenerationTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float MissionGenerationInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMission> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission CreateResourceCollectionMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission CreateCraftingMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission CreateHuntingMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission CreateShelterMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateMissionProgress(const FString& MissionName, const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckMissionCompletion(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AddPlayerExperience(int32 Experience);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeNPCDialogues();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_NPCDialogue GetNPCDialogue(const FString& NPCName) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void PlayNPCDialogue(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SetupTutorialMissions();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ValidateQuestSystem();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest System")
    void OnMissionStarted(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest System")
    void OnMissionCompleted(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest System")
    void OnPlayerLevelUp(int32 NewLevel);

private:
    void InitializeDefaultMissions();
    FString GenerateRandomMissionName(EQuest_SurvivalMissionType MissionType);
    EQuest_MissionDifficulty CalculateMissionDifficulty() const;
    TArray<FString> GetRandomResources(int32 Count) const;
    FVector GetRandomMissionLocation() const;
};