#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Quest_EmergencyMissionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_EmergencyType : uint8
{
    DinosaurAttack      UMETA(DisplayName = "Dinosaur Attack"),
    WeatherStorm        UMETA(DisplayName = "Weather Storm"),
    ResourceDepletion   UMETA(DisplayName = "Resource Depletion"),
    InjuredSurvivor     UMETA(DisplayName = "Injured Survivor"),
    PredatorThreat      UMETA(DisplayName = "Predator Threat"),
    WildFire           UMETA(DisplayName = "Wild Fire")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_EmergencyMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Mission")
    EQuest_EmergencyType EmergencyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Mission")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Mission")
    FVector EmergencyLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Mission")
    float UrgencyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Mission")
    bool bIsActive;

    FQuest_EmergencyMission()
    {
        EmergencyType = EQuest_EmergencyType::DinosaurAttack;
        MissionTitle = TEXT("Emergency Mission");
        MissionDescription = TEXT("Respond to emergency situation");
        EmergencyLocation = FVector::ZeroVector;
        UrgencyLevel = 1.0f;
        TimeLimit = 300.0f; // 5 minutes
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_EmergencyMissionSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_EmergencyMissionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Emergency Mission Management
    UFUNCTION(BlueprintCallable, Category = "Emergency Missions")
    void TriggerEmergencyMission(EQuest_EmergencyType EmergencyType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Emergency Missions")
    void CompleteEmergencyMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Emergency Missions")
    void FailEmergencyMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Emergency Missions")
    TArray<FQuest_EmergencyMission> GetActiveEmergencyMissions() const;

    // Emergency Detection
    UFUNCTION(BlueprintCallable, Category = "Emergency Detection")
    void CheckForEmergencies();

    UFUNCTION(BlueprintCallable, Category = "Emergency Detection")
    bool IsPlayerInDanger() const;

    UFUNCTION(BlueprintCallable, Category = "Emergency Detection")
    void SpawnEmergencyActors(EQuest_EmergencyType EmergencyType, FVector Location);

protected:
    // Emergency Mission Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Missions")
    TArray<FQuest_EmergencyMission> ActiveEmergencyMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Missions")
    int32 MaxConcurrentEmergencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Missions")
    float EmergencyCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Missions")
    float LastEmergencyCheckTime;

    // Emergency Spawn Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Spawning")
    float MinDistanceFromPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Spawning")
    float MaxDistanceFromPlayer;

    // Mission Rewards
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 EmergencyMissionXPReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    TArray<FString> EmergencyMissionRewards;

private:
    // Helper Functions
    FVector GetRandomEmergencyLocation() const;
    FString GenerateEmergencyMissionTitle(EQuest_EmergencyType EmergencyType) const;
    FString GenerateEmergencyMissionDescription(EQuest_EmergencyType EmergencyType) const;
    void CleanupExpiredEmergencies();
    void NotifyPlayerOfEmergency(const FQuest_EmergencyMission& Mission);
};