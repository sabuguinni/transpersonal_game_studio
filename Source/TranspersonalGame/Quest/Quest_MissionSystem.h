#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Quest_MissionSystem.generated.h"

// Forward declarations
class UQuest_MissionComponent;
class AQuest_ObjectiveMarker;

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt_Dinosaur,
    Escort_NPC,
    Gather_Resources,
    Explore_Territory,
    Defend_Settlement,
    Tribal_Diplomacy
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Available,
    Active,
    Completed,
    Failed,
    Locked
};

USTRUCT(BlueprintType)
struct FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_MissionObjective()
    {
        ObjectiveText = TEXT("Default Objective");
        bCompleted = false;
        CurrentProgress = 0;
        RequiredProgress = 1;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString BiomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    FQuest_MissionData()
    {
        MissionID = TEXT("MISSION_001");
        MissionTitle = TEXT("Survival Mission");
        MissionDescription = TEXT("Complete objectives to survive");
        MissionType = EQuest_MissionType::Hunt_Dinosaur;
        Status = EQuest_MissionStatus::Available;
        BiomeLocation = TEXT("Savana");
        TimeLimit = 0.0f;
        ExperienceReward = 100;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_MissionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_MissionSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeMissionSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_MissionData> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_MissionData GetMissionData(const FString& MissionID) const;

    // Mission Creation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntMission(const FString& DinosaurType, const FString& BiomeLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateEscortMission(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatherMission(const FString& ResourceType, int32 Quantity);

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, const FString&, MissionID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, const FString&, MissionID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, const FString&, MissionID, int32, ObjectiveIndex);

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnMissionStarted OnMissionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

private:
    UPROPERTY()
    TMap<FString, FQuest_MissionData> AllMissions;

    UPROPERTY()
    TArray<FString> ActiveMissionIDs;

    void CreateDefaultMissions();
    void SpawnObjectiveMarkers(const FQuest_MissionData& MissionData);
    bool ValidateMissionCompletion(const FQuest_MissionData& MissionData) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_MissionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_MissionComponent();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Quest Component")
    void RegisterWithMissionSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Component")
    void TriggerObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest Component")
    bool IsObjectiveTarget(const FString& MissionID, int32 ObjectiveIndex) const;

private:
    UPROPERTY()
    UQuest_MissionSystem* MissionSystem;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ObjectiveMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ObjectiveMarker();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FString AssociatedMissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    int32 ObjectiveIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    bool bIsActive;

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void ActivateMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void DeactivateMarker();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Marker")
    void OnMarkerActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Marker")
    void OnMarkerDeactivated();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    UFUNCTION()
    void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};