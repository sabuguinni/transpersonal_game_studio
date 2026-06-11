#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../../SharedTypes.h"
#include "CombatAIManager.generated.h"

class APawn;
class ACharacter;

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Extreme     UMETA(DisplayName = "Extreme Threat")
};

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Territorial UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FString> DinosaurTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EngagementRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsActive;

    FCombat_EncounterData()
    {
        Location = FVector::ZeroVector;
        ThreatLevel = ECombat_ThreatLevel::None;
        EngagementRadius = 1000.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float GroupCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanAmbush;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bPackHunter;

    FCombat_TacticalData()
    {
        AttackRange = 300.0f;
        FleeDistance = 1500.0f;
        GroupCoordination = 0.5f;
        bCanAmbush = false;
        bPackHunter = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatAIManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat Zone Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatZone(ATriggerBox* Zone, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CreateTacticalEncounter(FVector Location, const TArray<FString>& DinosaurTypes, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel CalculateThreatLevel(APawn* Target, const TArray<APawn*>& Enemies);

    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinosaurAIState(APawn* Dinosaur, ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetDinosaurAIState(APawn* Dinosaur) const;

    // Tactical Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack(const TArray<APawn*>& PackMembers, APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(APawn* Attacker, APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(APawn* Dinosaur, APawn* Target);

    // Combat Events
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnPlayerEnterCombatZone(ACharacter* Player, ATriggerBox* Zone);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnDinosaurSpotPlayer(APawn* Dinosaur, ACharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnCombatInitiated(APawn* Aggressor, APawn* Target);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<FCombat_EncounterData> ActiveEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TMap<APawn*, ECombat_AIState> DinosaurStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TMap<FString, FCombat_TacticalData> SpeciesTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxEngagementDistance;

private:
    void UpdateCombatStates(float DeltaTime);
    void ProcessTacticalDecisions();
    void InitializeSpeciesTactics();
    
    float LastCombatUpdate;
    ACharacter* PlayerCharacter;
};