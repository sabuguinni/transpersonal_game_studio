#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Quest_SurvivalMissionController.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    Hunt_Raptor UMETA(DisplayName = "Hunt Raptor"),
    Gather_Resources UMETA(DisplayName = "Gather Resources"),
    Build_Shelter UMETA(DisplayName = "Build Shelter"),
    Find_Water UMETA(DisplayName = "Find Water"),
    Craft_Tools UMETA(DisplayName = "Craft Tools"),
    Escape_Predator UMETA(DisplayName = "Escape Predator"),
    Explore_Cave UMETA(DisplayName = "Explore Cave")
};

UENUM(BlueprintType)
enum class EQuest_MissionDifficulty : uint8
{
    Beginner UMETA(DisplayName = "Beginner"),
    Novice UMETA(DisplayName = "Novice"),
    Intermediate UMETA(DisplayName = "Intermediate"),
    Advanced UMETA(DisplayName = "Advanced"),
    Expert UMETA(DisplayName = "Expert")
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType = EQuest_SurvivalMissionType::Hunt_Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionDifficulty Difficulty = EQuest_MissionDifficulty::Beginner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionTitle = TEXT("Survival Mission");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription = TEXT("Complete this survival challenge");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit = 300.0f; // 5 minutes default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    FQuest_SurvivalMissionData()
    {
        RequiredItems.Empty();
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
    UStaticMeshComponent* MissionMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    FQuest_SurvivalMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    TArray<FQuest_SurvivalMissionData> AvailableMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    bool bMissionActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    float MissionStartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    int32 CurrentProgress = 0;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void StartMission(EQuest_SurvivalMissionType MissionType, EQuest_MissionDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void UpdateProgress(int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    bool IsMissionComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    float GetRemainingTime() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    FString GetMissionStatusText() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateDifficultyBasedMission(EQuest_MissionDifficulty TargetDifficulty);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
        bool bFromSweep, const FHitResult& SweepResult);

    void InitializeMissionDatabase();
    void SetupMissionMarker();
    void UpdateMissionMarkerVisual();
    FQuest_SurvivalMissionData CreateMissionData(EQuest_SurvivalMissionType Type, EQuest_MissionDifficulty Difficulty);
};