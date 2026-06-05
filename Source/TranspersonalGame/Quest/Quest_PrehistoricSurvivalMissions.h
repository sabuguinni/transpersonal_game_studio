#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_PrehistoricSurvivalMissions.generated.h"

// Survival mission types for prehistoric gameplay
UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    WaterSource         UMETA(DisplayName = "Find Water Source"),
    ShelterBuilding     UMETA(DisplayName = "Build Shelter"),
    FireCrafting        UMETA(DisplayName = "Create Fire"),
    ToolMaking          UMETA(DisplayName = "Craft Tools"),
    FoodGathering       UMETA(DisplayName = "Gather Food"),
    HuntingPreparation  UMETA(DisplayName = "Prepare for Hunt"),
    TerritoryMapping    UMETA(DisplayName = "Map Territory"),
    ResourceStockpiling UMETA(DisplayName = "Stockpile Resources"),
    WeatherSurvival     UMETA(DisplayName = "Survive Weather"),
    InjuryTreatment     UMETA(DisplayName = "Treat Injuries")
};

// Survival mission difficulty levels
UENUM(BlueprintType)
enum class EQuest_SurvivalDifficulty : uint8
{
    Novice      UMETA(DisplayName = "Novice Survivor"),
    Apprentice  UMETA(DisplayName = "Apprentice Survivor"),
    Experienced UMETA(DisplayName = "Experienced Survivor"),
    Expert      UMETA(DisplayName = "Expert Survivor"),
    Master      UMETA(DisplayName = "Master Survivor")
};

// Survival mission requirements
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalRequirements
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    float MinimumHealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    float MinimumStaminaPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    int32 RequiredSkillLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    bool bRequiresDaylight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    float MaximumDistanceFromShelter;

    FQuest_SurvivalRequirements()
    {
        MinimumHealthPercentage = 50.0f;
        MinimumStaminaPercentage = 30.0f;
        RequiredSkillLevel = 1;
        bRequiresDaylight = false;
        MaximumDistanceFromShelter = 5000.0f;
    }
};

// Survival mission rewards
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalRewards
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    float HealthRestoration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    float StaminaBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 SkillPointsAwarded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    FString UnlockedRecipe;

    FQuest_SurvivalRewards()
    {
        ExperiencePoints = 100;
        HealthRestoration = 25.0f;
        StaminaBonus = 15.0f;
        SkillPointsAwarded = 1;
    }
};

/**
 * Comprehensive prehistoric survival mission system
 * Manages all survival-related quests and challenges
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_PrehistoricSurvivalMissions : public AActor
{
    GENERATED_BODY()

public:
    AQuest_PrehistoricSurvivalMissions();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE SURVIVAL MISSION PROPERTIES ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* MissionTriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MissionMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    EQuest_SurvivalDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    FQuest_SurvivalRequirements Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    FQuest_SurvivalRewards Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    float MissionTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    int32 MaxCompletions;

    // === MISSION STATE TRACKING ===

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    bool bIsMissionActive;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    bool bIsMissionCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    float MissionStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    float RemainingTime;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    int32 CompletionCount;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    class APawn* CurrentPlayer;

    // === SURVIVAL MISSION FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool CanStartMission(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool StartSurvivalMission(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool CompleteSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CancelSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    float GetMissionProgress();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool CheckMissionRequirements(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void AwardMissionRewards(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FString GetMissionStatusText();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void UpdateMissionObjectives();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool IsPlayerInMissionArea(class APawn* Player);

    // === SPECIALIZED SURVIVAL MISSIONS ===

    UFUNCTION(BlueprintCallable, Category = "Water Mission")
    bool ProcessWaterSourceMission();

    UFUNCTION(BlueprintCallable, Category = "Shelter Mission")
    bool ProcessShelterBuildingMission();

    UFUNCTION(BlueprintCallable, Category = "Fire Mission")
    bool ProcessFireCraftingMission();

    UFUNCTION(BlueprintCallable, Category = "Tool Mission")
    bool ProcessToolMakingMission();

    UFUNCTION(BlueprintCallable, Category = "Food Mission")
    bool ProcessFoodGatheringMission();

    UFUNCTION(BlueprintCallable, Category = "Hunting Mission")
    bool ProcessHuntingPreparationMission();

    UFUNCTION(BlueprintCallable, Category = "Territory Mission")
    bool ProcessTerritoryMappingMission();

    UFUNCTION(BlueprintCallable, Category = "Resource Mission")
    bool ProcessResourceStockpilingMission();

    UFUNCTION(BlueprintCallable, Category = "Weather Mission")
    bool ProcessWeatherSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Medical Mission")
    bool ProcessInjuryTreatmentMission();

    // === MISSION TRIGGER EVENTS ===

    UFUNCTION()
    void OnMissionTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                     bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnMissionTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
    // === INTERNAL MISSION LOGIC ===

    UFUNCTION()
    void UpdateMissionTimer();

    UFUNCTION()
    void CheckMissionCompletion();

    UFUNCTION()
    void HandleMissionTimeout();

    UFUNCTION()
    void InitializeMissionData();

    UFUNCTION()
    void SetupMissionMarker();

    UFUNCTION()
    void ValidatePlayerSurvivalStats(class APawn* Player);

private:
    // Mission timer handle
    FTimerHandle MissionTimerHandle;
    
    // Mission completion tracking
    TArray<FString> CompletedObjectives;
    TArray<FString> RemainingObjectives;
    
    // Player tracking
    TWeakObjectPtr<APawn> TrackedPlayer;
    
    // Mission validation
    bool bMissionDataInitialized;
    float LastValidationTime;
};