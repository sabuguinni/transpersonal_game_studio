#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_HuntingMissionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_HuntTargetType : uint8
{
    SmallPrey,      // Compsognathus, small mammals
    MediumPrey,     // Parasaurolophus, Protoceratops
    LargePrey,      // Triceratops, Ankylosaurus
    Predator,       // Velociraptor pack
    ApexPredator    // T-Rex
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Target")
    EQuest_HuntTargetType TargetType;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Target")
    FString SpeciesName;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Target")
    float DifficultyRating;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Target")
    FVector LastKnownLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Target")
    bool bIsActive;

    FQuest_HuntTarget()
    {
        TargetType = EQuest_HuntTargetType::SmallPrey;
        SpeciesName = TEXT("Unknown");
        RequiredCount = 1;
        DifficultyRating = 1.0f;
        LastKnownLocation = FVector::ZeroVector;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntMission
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    FString MissionID;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    FString MissionName;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    TArray<FQuest_HuntTarget> Targets;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    float TimeLimit;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    float TimeRemaining;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    bool bIsCompleted;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    bool bIsFailed;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    FVector HuntingGrounds;

    UPROPERTY(BlueprintReadWrite, Category = "Hunt Mission")
    float HuntingRadius;

    FQuest_HuntMission()
    {
        MissionID = TEXT("");
        MissionName = TEXT("Hunt Mission");
        Description = TEXT("Hunt the specified targets");
        TimeLimit = 1800.0f; // 30 minutes
        TimeRemaining = TimeLimit;
        bIsCompleted = false;
        bIsFailed = false;
        HuntingGrounds = FVector::ZeroVector;
        HuntingRadius = 10000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_HuntingMissionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_HuntingMissionSystem();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission creation
    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    FString CreateHuntMission(const FString& MissionName, EQuest_HuntTargetType TargetType, int32 TargetCount, float TimeLimit);

    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    bool StartHuntMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    bool CompleteHuntMission(const FString& MissionID);

    // Mission tracking
    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    void UpdateHuntProgress(const FString& MissionID, const FString& SpeciesName, const FVector& KillLocation);

    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    FQuest_HuntMission GetHuntMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    TArray<FQuest_HuntMission> GetActiveHuntMissions();

    // Mission management
    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    void UpdateMissionTimers(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    bool IsHuntMissionActive(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    void CancelHuntMission(const FString& MissionID);

    // Hunting ground management
    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    void SetHuntingGrounds(const FString& MissionID, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    bool IsInHuntingGrounds(const FString& MissionID, const FVector& Location);

    // Difficulty and rewards
    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    float CalculateHuntDifficulty(EQuest_HuntTargetType TargetType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Hunting Missions")
    void GrantHuntRewards(const FString& MissionID);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Hunt Missions")
    TMap<FString, FQuest_HuntMission> ActiveHuntMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Hunt Missions")
    TArray<FString> CompletedMissionIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Hunt Missions")
    int32 NextMissionID;

    // Mission templates
    void InitializeHuntMissionTemplates();
    FQuest_HuntMission CreateBasicHuntTemplate(EQuest_HuntTargetType TargetType);
    
    // Timer management
    FTimerHandle MissionTimerHandle;
    void CheckMissionTimeouts();

    // Validation
    bool ValidateHuntTarget(const FQuest_HuntTarget& Target);
    bool ValidateMissionParameters(const FString& MissionName, EQuest_HuntTargetType TargetType, int32 TargetCount);
};