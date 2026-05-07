#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Quest_SurvivalQuestController.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalQuestType : uint8
{
    Hunt_Herbivore      UMETA(DisplayName = "Hunt Herbivore"),
    Gather_Resources    UMETA(DisplayName = "Gather Resources"),
    Craft_Tool         UMETA(DisplayName = "Craft Tool"),
    Build_Shelter      UMETA(DisplayName = "Build Shelter"),
    Explore_Territory  UMETA(DisplayName = "Explore Territory"),
    Defend_Camp        UMETA(DisplayName = "Defend Camp"),
    Track_Migration    UMETA(DisplayName = "Track Migration"),
    Escape_Predator    UMETA(DisplayName = "Escape Predator")
};

UENUM(BlueprintType)
enum class EQuest_QuestPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Optional    UMETA(DisplayName = "Optional")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_SurvivalQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 TargetQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    FQuest_SurvivalObjective()
    {
        ObjectiveDescription = TEXT("Default Objective");
        QuestType = EQuest_SurvivalQuestType::Gather_Resources;
        TargetQuantity = 1;
        CurrentProgress = 0;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        bIsCompleted = false;
        bIsOptional = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_SurvivalObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    FQuest_SurvivalQuest()
    {
        QuestName = TEXT("Default Quest");
        QuestDescription = TEXT("A basic survival quest");
        Priority = EQuest_QuestPriority::Medium;
        TimeLimit = 0.0f; // 0 = no time limit
        ElapsedTime = 0.0f;
        bIsActive = false;
        bIsCompleted = false;
        ExperienceReward = 100;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalQuestController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalQuestController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarkerMesh;

    // Quest Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float QuestUpdateInterval;

    // Visual Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* DefaultQuestMarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UMaterialInterface* QuestMarkerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float MarkerScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowQuestMarkers;

    // Quest Management Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FQuest_SurvivalQuest& NewQuest);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateQuestProgress(const FString& QuestName, int32 ObjectiveIndex, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GetActiveQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> GetActiveQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ClearCompletedQuests();

    // Survival Quest Generators
    UFUNCTION(BlueprintCallable, Category = "Quest Generation")
    FQuest_SurvivalQuest GenerateHuntingQuest(const FVector& HuntLocation, const FString& TargetSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest Generation")
    FQuest_SurvivalQuest GenerateGatheringQuest(const FVector& GatherLocation, const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest Generation")
    FQuest_SurvivalQuest GenerateCraftingQuest(const FString& ItemToCraft);

    UFUNCTION(BlueprintCallable, Category = "Quest Generation")
    FQuest_SurvivalQuest GenerateExplorationQuest(const FVector& ExploreLocation, float ExploreRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest Generation")
    FQuest_SurvivalQuest GenerateDefenseQuest(const FVector& DefenseLocation);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    float GetQuestCompletionPercentage(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateQuestMarkers();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SetQuestMarkerVisibility(bool bVisible);

private:
    // Internal tracking
    float LastUpdateTime;
    
    // Helper functions
    void UpdateActiveQuests(float DeltaTime);
    void CheckQuestCompletion();
    bool AreAllObjectivesCompleted(const FQuest_SurvivalQuest& Quest);
    void SpawnQuestMarker(const FVector& Location, const FString& QuestName);
    void RemoveQuestMarker(const FString& QuestName);
};