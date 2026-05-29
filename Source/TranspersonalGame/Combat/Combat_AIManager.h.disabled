#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Combat_AIManager.generated.h"

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
    Hunting     UMETA(DisplayName = "Hunting"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Territorial UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float Distance;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float LastSeenTime;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector LastKnownPosition;

    FCombat_ThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownPosition = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_AIManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCombat_AIManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Combat AI management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatActor(AActor* Actor, ECombat_AIState InitialState = ECombat_AIState::Passive);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatAI(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel EvaluateThreat(AActor* Evaluator, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<FCombat_ThreatData> GetNearbyThreats(AActor* Actor, float SearchRadius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(AActor* Actor, ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetCombatState(AActor* Actor);

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void FormPack(const TArray<AActor*>& PackMembers, AActor* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void DisbandPack(AActor* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    TArray<AActor*> GetPackMembers(AActor* PackLeader);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TMap<AActor*, ECombat_AIState> CombatActors;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TMap<AActor*, TArray<AActor*>> PackFormations;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TMap<AActor*, FCombat_ThreatData> ThreatDatabase;

    // Combat parameters
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    float ThreatUpdateInterval;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    float MaxThreatDistance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    float PackCoordinationRadius;

private:
    float LastThreatUpdate;

    void UpdateThreatDatabase();
    void UpdatePackBehavior();
    float CalculateThreatScore(AActor* Evaluator, AActor* Target);
};