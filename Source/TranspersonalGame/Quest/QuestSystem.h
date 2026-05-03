#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "QuestSystem.generated.h"

// Quest objective types for survival gameplay
UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Explore_Biome       UMETA(DisplayName = "Explore Biome"),
    Hunt_Dinosaur       UMETA(DisplayName = "Hunt Dinosaur"),
    Gather_Resources    UMETA(DisplayName = "Gather Resources"),
    Craft_Item          UMETA(DisplayName = "Craft Item"),
    Survive_Duration    UMETA(DisplayName = "Survive Duration"),
    Reach_Location      UMETA(DisplayName = "Reach Location"),
    Defend_Area         UMETA(DisplayName = "Defend Area"),
    Escape_Predator     UMETA(DisplayName = "Escape Predator")
};

// Quest status tracking
UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    NotStarted          UMETA(DisplayName = "Not Started"),
    Active              UMETA(DisplayName = "Active"),
    Completed           UMETA(DisplayName = "Completed"),
    Failed              UMETA(DisplayName = "Failed"),
    Abandoned           UMETA(DisplayName = "Abandoned")
};

// Quest objective data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RequiredDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    FQuest_Objective()
    {
        ObjectiveID = "";
        Description = "";
        Type = EQuest_ObjectiveType::Explore_Biome;
        Status = EQuest_Status::NotStarted;
        TargetLocation = FVector::ZeroVector;
        RequiredDistance = 1000.0f;
        TargetActorClass = "";
        RequiredCount = 1;
        CurrentProgress = 0;
    }
};

// Quest data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    FQuest_Data()
    {
        QuestID = "";
        Title = "";
        Description = "";
        Status = EQuest_Status::NotStarted;
        TimeLimit = 0.0f;
        StartTime = 0.0f;
        bIsMainQuest = false;
    }
};

// Quest System Subsystem
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    // Location-based objective checking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckLocationObjectives(const FVector& PlayerLocation);

    // Resource gathering objective checking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    // Dinosaur hunting objective checking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnDinosaurKilled(const FString& DinosaurType);

    // Crafting objective checking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnItemCrafted(const FString& ItemType);

    // Initialize default quests
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeDefaultQuests();

protected:
    // Quest storage
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuest_Data> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FString> ActiveQuestIDs;

    // Helper functions
    void CreateExplorationQuests();
    void CreateSurvivalQuests();
    void CreateHuntingQuests();
    void CreateCraftingQuests();

    // Objective completion checking
    bool CheckObjectiveCompletion(FQuest_Objective& Objective, const FVector& PlayerLocation);
    void OnObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID);
    void CheckQuestCompletion(const FString& QuestID);

private:
    bool bIsInitialized;
    float LastUpdateTime;
};

// Quest Marker Actor for visual representation
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_Marker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_Marker();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString AssociatedQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString AssociatedObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    EQuest_ObjectiveType MarkerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    bool bIsActive;

    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void SetMarkerActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void SetMarkerData(const FString& QuestID, const FString& ObjectiveID, EQuest_ObjectiveType Type);
};