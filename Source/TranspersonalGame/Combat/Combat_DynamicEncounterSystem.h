#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerVolume.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Combat_DynamicEncounterSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_EncounterType : uint8
{
    None                UMETA(DisplayName = "None"),
    Ambush              UMETA(DisplayName = "Ambush"),
    Territorial         UMETA(DisplayName = "Territorial"),
    PackHunt            UMETA(DisplayName = "Pack Hunt"),
    Scavenger           UMETA(DisplayName = "Scavenger"),
    Apex                UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class ECombat_EncounterState : uint8
{
    Inactive            UMETA(DisplayName = "Inactive"),
    Preparing           UMETA(DisplayName = "Preparing"),
    Active              UMETA(DisplayName = "Active"),
    Escalating          UMETA(DisplayName = "Escalating"),
    Resolving           UMETA(DisplayName = "Resolving"),
    Completed           UMETA(DisplayName = "Completed")
};

USTRUCT(BlueprintType)
struct FCombat_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_EncounterType EncounterType = ECombat_EncounterType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 ThreatLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float Duration = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 ParticipantCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FVector EncounterCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float EncounterRadius = 1000.0f;

    FCombat_EncounterData()
    {
        EncounterType = ECombat_EncounterType::None;
        ThreatLevel = 1;
        Duration = 120.0f;
        ParticipantCount = 1;
        EncounterCenter = FVector::ZeroVector;
        EncounterRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombat_EscalationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation")
    float TriggerTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation")
    int32 ThreatIncrease = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation")
    bool bSpawnReinforcements = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation")
    int32 ReinforcementCount = 1;

    FCombat_EscalationRule()
    {
        TriggerTime = 30.0f;
        ThreatIncrease = 1;
        bSpawnReinforcements = false;
        ReinforcementCount = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_DynamicEncounterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DynamicEncounterComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    FCombat_EncounterData CurrentEncounter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    ECombat_EncounterState EncounterState = ECombat_EncounterState::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    TArray<FCombat_EscalationRule> EscalationRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    float EncounterTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    TArray<AActor*> EncounterParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    bool bAutoEscalate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter System")
    float PlayerProximityThreshold = 500.0f;

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void StartEncounter(ECombat_EncounterType Type, int32 InitialThreatLevel = 1);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void EscalateEncounter();

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void EndEncounter();

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void AddParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    void RemoveParticipant(AActor* Participant);

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    bool IsPlayerInEncounterRange() const;

    UFUNCTION(BlueprintCallable, Category = "Encounter System")
    float GetEncounterIntensity() const;

private:
    void UpdateEncounterState(float DeltaTime);
    void CheckEscalationRules();
    void SpawnReinforcements(int32 Count);
    AActor* GetPlayerActor() const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DynamicEncounterManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_DynamicEncounterManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Manager")
    TArray<FCombat_EncounterData> AvailableEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Manager")
    float EncounterSpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Manager")
    float MinEncounterInterval = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Manager")
    float MaxEncounterInterval = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Manager")
    int32 MaxSimultaneousEncounters = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Manager")
    TArray<UCombat_DynamicEncounterComponent*> ActiveEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter Manager")
    float NextEncounterTime = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Encounter Manager")
    void TriggerRandomEncounter();

    UFUNCTION(BlueprintCallable, Category = "Encounter Manager")
    void TriggerSpecificEncounter(ECombat_EncounterType Type, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Encounter Manager")
    void RegisterEncounter(UCombat_DynamicEncounterComponent* Encounter);

    UFUNCTION(BlueprintCallable, Category = "Encounter Manager")
    void UnregisterEncounter(UCombat_DynamicEncounterComponent* Encounter);

    UFUNCTION(BlueprintCallable, Category = "Encounter Manager")
    int32 GetActiveEncounterCount() const;

    UFUNCTION(BlueprintCallable, Category = "Encounter Manager")
    bool CanSpawnNewEncounter() const;

private:
    void UpdateEncounterSpawning(float DeltaTime);
    FVector FindValidEncounterLocation() const;
    ECombat_EncounterType SelectEncounterType() const;
    AActor* GetPlayerActor() const;
};