#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerBox.h"
#include "Engine/TriggerSphere.h"
#include "../SharedTypes.h"
#include "Quest_MassiveBattleQuestSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_BattleType : uint8
{
    RaptorPackAmbush     UMETA(DisplayName = "Raptor Pack Ambush"),
    TRexTerritorial      UMETA(DisplayName = "T-Rex Territorial War"),
    HerbivoreStampede    UMETA(DisplayName = "Herbivore Stampede"),
    PredatorHunting      UMETA(DisplayName = "Predator Hunting Grounds"),
    TriceratopsDefense   UMETA(DisplayName = "Triceratops Defense")
};

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt        UMETA(DisplayName = "Hunting Expedition"),
    Gather      UMETA(DisplayName = "Resource Gathering"),
    Defend      UMETA(DisplayName = "Territory Defense"),
    Explore     UMETA(DisplayName = "Exploration Mission"),
    Rescue      UMETA(DisplayName = "Rescue Operation")
};

USTRUCT(BlueprintType)
struct FQuest_BattleZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    EQuest_BattleType BattleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Zone")
    bool bIsActive;

    FQuest_BattleZoneData()
    {
        ZoneName = TEXT("DefaultBattleZone");
        Location = FVector::ZeroVector;
        Radius = 2000.0f;
        BattleType = EQuest_BattleType::RaptorPackAmbush;
        MaxCrowdSize = 50;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FQuest_MissionZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Zone")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Zone")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Zone")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Zone")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Zone")
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Zone")
    TArray<FString> Prerequisites;

    FQuest_MissionZoneData()
    {
        MissionName = TEXT("DefaultMission");
        Location = FVector::ZeroVector;
        MissionType = EQuest_MissionType::Hunt;
        TriggerRadius = 1500.0f;
        RequiredLevel = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_MassiveBattleQuestSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MassiveBattleQuestSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle System")
    TArray<FQuest_BattleZoneData> BattleZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_MissionZoneData> MissionZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    int32 MaxSimultaneousBattles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float BattleIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bEnableMassiveCrowdBattles;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    void InitializeBattleZones();

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    void InitializeMissionZones();

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    bool StartBattleQuest(EQuest_BattleType BattleType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    bool StartMissionQuest(EQuest_MissionType MissionType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    void UpdateBattleProgression(const FString& ZoneName, float ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    void TriggerMassiveBattle(const FVector& BattleCenter, int32 CrowdSize);

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    TArray<FQuest_BattleZoneData> GetActiveBattleZones() const;

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    TArray<FQuest_MissionZoneData> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    bool IsBattleZoneActive(const FString& ZoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Battle Quest System")
    void SetBattleIntensity(float NewIntensity);

private:
    void CreateBattleZoneTriggers();
    void CreateMissionZoneTriggers();
    void ValidateBattleZoneConfiguration();
    void UpdateBattleZoneStates();

    UPROPERTY()
    TArray<class ATriggerBox*> BattleZoneTriggers;

    UPROPERTY()
    TArray<class ATriggerSphere*> MissionZoneTriggers;

    float LastUpdateTime;
    int32 ActiveBattleCount;
};