#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Combat_EncounterManager.generated.h"

class UCombat_ThreatDetectionSystem;
class ACombat_TacticalAIController;

UENUM(BlueprintType)
enum class ECombat_EncounterType : uint8
{
    Ambush UMETA(DisplayName = "Ambush"),
    Territory UMETA(DisplayName = "Territory Defense"),
    Hunt UMETA(DisplayName = "Hunting Pack"),
    Patrol UMETA(DisplayName = "Patrol Route"),
    Feeding UMETA(DisplayName = "Feeding Ground"),
    Migration UMETA(DisplayName = "Migration Path")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    Low UMETA(DisplayName = "Low Threat"),
    Medium UMETA(DisplayName = "Medium Threat"),
    High UMETA(DisplayName = "High Threat"),
    Extreme UMETA(DisplayName = "Extreme Threat"),
    Lethal UMETA(DisplayName = "Lethal Threat")
};

USTRUCT(BlueprintType)
struct FCombat_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_EncounterType EncounterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FVector EncounterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float EncounterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    TArray<TSubclassOf<APawn>> EnemyTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 MinEnemyCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 MaxEnemyCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float ActivationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float CooldownTime;

    FCombat_EncounterData()
    {
        EncounterType = ECombat_EncounterType::Patrol;
        ThreatLevel = ECombat_ThreatLevel::Medium;
        EncounterLocation = FVector::ZeroVector;
        EncounterRadius = 2000.0f;
        MinEnemyCount = 1;
        MaxEnemyCount = 3;
        ActivationDistance = 1500.0f;
        bIsActive = true;
        CooldownTime = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_EncounterManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_EncounterManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    TArray<FCombat_EncounterData> EncounterZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    float PlayerDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    float EncounterCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    bool bDebugEncounters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCombat_ThreatDetectionSystem* ThreatDetection;

private:
    UPROPERTY()
    TArray<AActor*> ActiveEncounters;

    UPROPERTY()
    TArray<AActor*> SpawnedEnemies;

    float LastEncounterCheck;
    float LastThreatAssessment;

public:
    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void InitializeEncounterZones();

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void CheckPlayerProximity();

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    bool TriggerEncounter(const FCombat_EncounterData& EncounterData);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void SpawnEncounterEnemies(const FCombat_EncounterData& EncounterData);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void DeactivateEncounter(int32 EncounterIndex);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    ECombat_ThreatLevel AssessThreatLevel(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void UpdateDynamicThreatLevels();

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    TArray<AActor*> GetNearbyEnemies(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void SetEncounterActive(int32 EncounterIndex, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void AddEncounterZone(const FCombat_EncounterData& NewEncounter);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void RemoveEncounterZone(int32 EncounterIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Encounter Events")
    void OnEncounterTriggered(const FCombat_EncounterData& EncounterData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Encounter Events")
    void OnEncounterCompleted(const FCombat_EncounterData& EncounterData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Encounter Events")
    void OnThreatLevelChanged(ECombat_ThreatLevel NewThreatLevel, const FVector& Location);
};