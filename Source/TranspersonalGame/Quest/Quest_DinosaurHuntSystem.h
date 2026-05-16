#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Quest_DinosaurHuntSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_DinosaurSpecies : uint8
{
    Compsognathus    UMETA(DisplayName = "Compsognathus"),
    Parasaurolophus  UMETA(DisplayName = "Parasaurolophus"),
    Triceratops      UMETA(DisplayName = "Triceratops"),
    Velociraptor     UMETA(DisplayName = "Velociraptor"),
    Allosaurus       UMETA(DisplayName = "Allosaurus"),
    Tyrannosaurus    UMETA(DisplayName = "Tyrannosaurus Rex")
};

UENUM(BlueprintType)
enum class EQuest_HuntDifficulty : uint8
{
    Scavenger        UMETA(DisplayName = "Scavenger"),
    Hunter           UMETA(DisplayName = "Hunter"),
    Predator         UMETA(DisplayName = "Predator"),
    Apex             UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct FQuest_DinosaurTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    EQuest_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    EQuest_HuntDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float TrackingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    bool bRequiresSpecificWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FString RequiredWeaponType;

    FQuest_DinosaurTarget()
    {
        Species = EQuest_DinosaurSpecies::Compsognathus;
        RequiredCount = 1;
        Difficulty = EQuest_HuntDifficulty::Scavenger;
        LastKnownLocation = FVector::ZeroVector;
        TrackingRadius = 5000.0f;
        bRequiresSpecificWeapon = false;
        RequiredWeaponType = TEXT("Stone Spear");
    }
};

USTRUCT(BlueprintType)
struct FQuest_HuntMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    TArray<FQuest_DinosaurTarget> Targets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    bool bRequiresGroupHunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    int32 MinimumHunters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    TArray<FString> RewardItems;

    FQuest_HuntMission()
    {
        MissionName = TEXT("First Hunt");
        MissionDescription = TEXT("Track and hunt small prey to prove your survival skills");
        TimeLimit = 1800.0f; // 30 minutes
        StartLocation = FVector::ZeroVector;
        SearchRadius = 10000.0f;
        bRequiresGroupHunt = false;
        MinimumHunters = 1;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_DinosaurHuntSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DinosaurHuntSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    TArray<FQuest_HuntMission> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    FQuest_HuntMission CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    bool bMissionActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    float MissionStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    TMap<EQuest_DinosaurSpecies, int32> KillCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    TArray<AActor*> TrackedDinosaurs;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void InitializeHuntSystem();

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void StartHuntMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void RegisterDinosaurKill(EQuest_DinosaurSpecies Species, const FVector& KillLocation);

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    bool CheckMissionProgress();

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void UpdateDinosaurTracking();

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    TArray<FQuest_HuntMission> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    FQuest_HuntMission GetCurrentMission() const;

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    float GetRemainingTime() const;

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void CreateBasicHuntMissions();

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void SpawnDinosaurTargets();

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    AActor* FindNearestDinosaur(EQuest_DinosaurSpecies Species, const FVector& SearchLocation);

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void SetHuntWaypoint(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void ShowHuntObjectives();

private:
    void CreateScavengerMissions();
    void CreateHunterMissions();
    void CreatePredatorMissions();
    void CreateApexMissions();
    
    bool ValidateWeaponRequirement(const FString& WeaponType);
    void DistributeRewards(const TArray<FString>& Rewards);
    void UpdateMissionUI();
    void LogHuntProgress(const FString& Message);
};