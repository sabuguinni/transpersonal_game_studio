#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "Quest_SurvivalMissionController.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    ResourceCollection UMETA(DisplayName = "Resource Collection"),
    HuntingMission UMETA(DisplayName = "Hunting Mission"),
    CraftingTask UMETA(DisplayName = "Crafting Task"),
    ExplorationQuest UMETA(DisplayName = "Exploration Quest"),
    ShelterBuilding UMETA(DisplayName = "Shelter Building"),
    TerritoryDefense UMETA(DisplayName = "Territory Defense")
};

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone UMETA(DisplayName = "Stone"),
    Wood UMETA(DisplayName = "Wood"),
    Water UMETA(DisplayName = "Water"),
    Food UMETA(DisplayName = "Food"),
    Fiber UMETA(DisplayName = "Fiber"),
    Bone UMETA(DisplayName = "Bone"),
    Hide UMETA(DisplayName = "Hide")
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ResourceType RequiredResource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    FQuest_SurvivalObjective()
    {
        ObjectiveName = TEXT("Default Objective");
        Description = TEXT("Complete this survival task");
        MissionType = EQuest_SurvivalMissionType::ResourceCollection;
        RequiredResource = EQuest_ResourceType::Stone;
        RequiredAmount = 1;
        CurrentAmount = 0;
        bIsCompleted = false;
        TimeLimit = 300.0f; // 5 minutes default
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EQuest_SurvivalMissionType SpecialtyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_SurvivalObjective> AvailableQuests;

    FQuest_NPCDialogue()
    {
        NPCName = TEXT("Unknown NPC");
        DialogueText = TEXT("Hello, traveler.");
        AudioURL = TEXT("");
        SpecialtyType = EQuest_SurvivalMissionType::ResourceCollection;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ControllerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    // Active Mission Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    TArray<FQuest_SurvivalObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    TArray<FQuest_NPCDialogue> NPCDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    int32 MaxConcurrentMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    bool bAutoGenerateMissions;

    // Resource Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<EQuest_ResourceType, int32> PlayerResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TArray<FVector> ResourceSpawnLocations;

    // Mission Progress Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float TotalPlayTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float SurvivalDifficulty;

public:
    virtual void Tick(float DeltaTime) override;

    // Mission Management Functions
    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void GenerateNewSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void CompleteObjective(const FString& ObjectiveName);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void UpdateObjectiveProgress(const FString& ObjectiveName, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    bool CheckMissionCompletion(const FQuest_SurvivalObjective& Objective);

    // Resource Management Functions
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool ConsumeResource(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 GetResourceCount(EQuest_ResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    void SpawnResourceNodes();

    // NPC Dialogue Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_NPCDialogue GetNPCDialogue(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AssignQuestFromNPC(const FString& NPCName, int32 QuestIndex);

    // Mission Generation Functions
    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    FQuest_SurvivalObjective CreateResourceCollectionMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    FQuest_SurvivalObjective CreateHuntingMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    FQuest_SurvivalObjective CreateCraftingMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    FQuest_SurvivalObjective CreateExplorationMission();

    // Event Handlers
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    FTimerHandle MissionGenerationTimer;
    
    void InitializeNPCDialogues();
    void InitializeResourceSpawnPoints();
    FVector GetRandomLocationNearPlayer(float MinDistance = 500.0f, float MaxDistance = 2000.0f);
    EQuest_ResourceType GetRandomResourceType();
    int32 GetRandomResourceAmount(EQuest_ResourceType ResourceType);
};