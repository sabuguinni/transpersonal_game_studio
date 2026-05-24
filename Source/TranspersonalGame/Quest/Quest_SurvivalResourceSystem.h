#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Quest_SurvivalResourceSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None = 0,
    Stone,
    Wood,
    Water,
    Food,
    Fiber,
    Hide,
    Bone
};

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    None = 0,
    GatherStone,
    CollectWood,
    FindWater,
    HuntFood,
    BuildShelter,
    CraftTool,
    ExploreArea,
    DefendCamp
};

USTRUCT(BlueprintType)
struct FQuest_ResourceRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 RequiredAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float QualityThreshold = 0.5f;

    FQuest_ResourceRequirement()
    {
        ResourceType = EQuest_ResourceType::None;
        RequiredAmount = 1;
        QualityThreshold = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType = EQuest_SurvivalMissionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_ResourceRequirement> ResourceRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float CompletionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive = false;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("Survival Mission");
        Description = TEXT("Complete survival objective");
        MissionType = EQuest_SurvivalMissionType::None;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        ExperienceReward = 10;
        bIsCompleted = false;
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SurvivalResourceSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalResourceSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core survival mission management
    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void InitializeSurvivalMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void StartMission(EQuest_SurvivalMissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void CompleteMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    bool CheckMissionCompletion(int32 MissionIndex);

    // Resource management
    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    int32 GetResourceCount(EQuest_ResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    bool HasRequiredResources(const TArray<FQuest_ResourceRequirement>& Requirements);

    // Mission progression
    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void UpdateMissionProgress();

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    TArray<FQuest_SurvivalMission> GetActiveMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    FQuest_SurvivalMission GetMissionByType(EQuest_SurvivalMissionType MissionType);

    // Location-based missions
    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void CheckLocationBasedMissions(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void CreateResourceGatheringMission(EQuest_ResourceType ResourceType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void CreateCraftingMission(const TArray<FQuest_ResourceRequirement>& Requirements);

protected:
    // Mission data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    // Resource inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    TMap<EQuest_ResourceType, int32> ResourceInventory;

    // Mission configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    int32 MaxActiveMissions = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float MissionCheckInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float LastMissionCheck = 0.0f;

    // Tutorial progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    bool bTutorialCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    int32 CurrentTutorialStep = 0;

private:
    // Internal mission management
    void GenerateBasicSurvivalMissions();
    void GenerateAdvancedSurvivalMissions();
    FQuest_SurvivalMission CreateGatheringMission(EQuest_ResourceType ResourceType, int32 Amount);
    FQuest_SurvivalMission CreateCraftingMission(const FString& ItemName, const TArray<FQuest_ResourceRequirement>& Requirements);
    FQuest_SurvivalMission CreateExplorationMission(FVector TargetLocation, const FString& AreaName);
    
    // Mission validation
    bool ValidateMissionRequirements(const FQuest_SurvivalMission& Mission);
    void CleanupCompletedMissions();
    void UpdateMissionObjectives();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalResourceManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalResourceManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Global survival quest management
    UFUNCTION(BlueprintCallable, Category = "Survival Quest Manager")
    void InitializeGlobalSurvivalSystem();

    UFUNCTION(BlueprintCallable, Category = "Survival Quest Manager")
    void RegisterPlayer(APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest Manager")
    void CreateEmergencyMission(EQuest_SurvivalMissionType MissionType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest Manager")
    void BroadcastMissionUpdate(const FQuest_SurvivalMission& Mission);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UQuest_SurvivalResourceSystem* SurvivalSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest Manager")
    TArray<AActor*> RegisteredPlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest Manager")
    bool bGlobalSystemActive = true;

private:
    void UpdateGlobalMissionState();
    void CheckPlayerSurvivalStatus();
    void GenerateEnvironmentalMissions();
};