#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Quest_DynamicMissionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Survival_Gathering UMETA(DisplayName = "Survival Gathering"),
    Survival_Hunting UMETA(DisplayName = "Survival Hunting"),
    Survival_Exploration UMETA(DisplayName = "Survival Exploration"),
    Survival_Escape UMETA(DisplayName = "Survival Escape"),
    Survival_Crafting UMETA(DisplayName = "Survival Crafting"),
    Survival_Rescue UMETA(DisplayName = "Survival Rescue")
};

UENUM(BlueprintType)
enum class EQuest_MissionPriority : uint8
{
    Critical UMETA(DisplayName = "Critical"),
    High UMETA(DisplayName = "High"),
    Medium UMETA(DisplayName = "Medium"),
    Low UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Available UMETA(DisplayName = "Available"),
    Active UMETA(DisplayName = "Active"),
    Completed UMETA(DisplayName = "Completed"),
    Failed UMETA(DisplayName = "Failed"),
    Expired UMETA(DisplayName = "Expired")
};

USTRUCT(BlueprintType)
struct FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    FQuest_MissionData()
    {
        MissionID = "";
        MissionName = "";
        MissionDescription = "";
        MissionType = EQuest_MissionType::Survival_Gathering;
        Priority = EQuest_MissionPriority::Medium;
        Status = EQuest_MissionStatus::Available;
        TimeLimit = 0.0f;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 100.0f;
        RequiredQuantity = 1;
        ExperienceReward = 0;
    }
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerThirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerFear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> NearbyThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> AvailableResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bIsInDanger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bHasShelter;

    FQuest_SurvivalContext()
    {
        PlayerHealth = 100.0f;
        PlayerHunger = 100.0f;
        PlayerThirst = 100.0f;
        PlayerFear = 0.0f;
        TimeOfDay = 12.0f;
        CurrentBiome = "Forest";
        bIsInDanger = false;
        bHasShelter = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DynamicMissionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DynamicMissionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Generation
    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    FQuest_MissionData GenerateMissionBasedOnContext(const FQuest_SurvivalContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    TArray<FQuest_MissionData> GenerateEmergencyMissions(const FQuest_SurvivalContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    FQuest_MissionData CreateGatheringMission(const FVector& PlayerLocation, const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    FQuest_MissionData CreateHuntingMission(const FVector& PlayerLocation, const FString& PreyType);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    FQuest_MissionData CreateEscapeMission(const FVector& ThreatLocation, const FVector& SafeLocation);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    FQuest_MissionData CreateExplorationMission(const FVector& UnexploredArea);

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    void ActivateMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    void FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    void UpdateMissionProgress(const FString& MissionID, float Progress);

    // Mission Query
    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    TArray<FQuest_MissionData> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    FQuest_MissionData GetMissionByID(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    bool IsMissionActive(const FString& MissionID) const;

    // Context Analysis
    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    FQuest_SurvivalContext AnalyzePlayerContext(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    EQuest_MissionPriority DetermineMissionPriority(const FQuest_SurvivalContext& Context, EQuest_MissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Mission")
    bool ShouldGenerateEmergencyMission(const FQuest_SurvivalContext& Context);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_MissionData> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_MissionData> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_MissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float LastMissionGenerationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FString> PossibleResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FString> PossiblePreyTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FString> PossibleThreats;

private:
    FString GenerateUniqueMissionID();
    FVector FindNearestResourceLocation(const FVector& PlayerLocation, const FString& ResourceType);
    FVector FindSafeLocation(const FVector& PlayerLocation, float MinDistance);
    void CleanupExpiredMissions();
    void UpdateMissionTimers(float DeltaTime);
};