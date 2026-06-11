#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Quest_SurvivalQuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalQuestType : uint8
{
    Hunt_Dinosaur      UMETA(DisplayName = "Hunt Dinosaur"),
    Gather_Resources   UMETA(DisplayName = "Gather Resources"),
    Build_Shelter      UMETA(DisplayName = "Build Shelter"),
    Survive_Night      UMETA(DisplayName = "Survive Night"),
    Explore_Territory  UMETA(DisplayName = "Explore Territory"),
    Craft_Tools        UMETA(DisplayName = "Craft Tools")
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_SurvivalQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardExperience;

    FQuest_SurvivalObjective()
    {
        ObjectiveDescription = TEXT("Default Objective");
        QuestType = EQuest_SurvivalQuestType::Gather_Resources;
        TargetAmount = 1;
        CurrentProgress = 0;
        bIsCompleted = false;
        RewardExperience = 10.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AQuest_SurvivalQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalQuestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalObjective> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalObjective> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float QuestGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bAutoGenerateQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float SurvivalDifficultyMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    TArray<FString> DinosaurHuntTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    TArray<FString> ResourceGatherTargets;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateRandomSurvivalQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateQuestProgress(EQuest_SurvivalQuestType QuestType, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(int32 QuestIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalObjective> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetActiveQuestCount() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartHuntQuest(const FString& DinosaurType, int32 TargetCount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartGatherQuest(const FString& ResourceType, int32 TargetAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartSurvivalChallenge(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ResetAllQuests();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest System")
    void OnQuestCompleted(const FQuest_SurvivalObjective& CompletedQuest);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest System")
    void OnQuestProgressUpdated(const FQuest_SurvivalObjective& UpdatedQuest);

private:
    FTimerHandle QuestGenerationTimer;
    
    void InitializeDefaultQuests();
    FQuest_SurvivalObjective CreateHuntQuest(const FString& Target, int32 Count);
    FQuest_SurvivalObjective CreateGatherQuest(const FString& Resource, int32 Amount);
    FQuest_SurvivalObjective CreateSurvivalQuest(float Duration);
};