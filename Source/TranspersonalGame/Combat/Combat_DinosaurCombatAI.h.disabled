#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "Combat_DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Passive      UMETA(DisplayName = "Passive"),
    Alert        UMETA(DisplayName = "Alert"),
    Hunting      UMETA(DisplayName = "Hunting"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Retreating   UMETA(DisplayName = "Retreating"),
    Feeding      UMETA(DisplayName = "Feeding"),
    Territorial  UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ECombat_AttackPattern : uint8
{
    DirectCharge    UMETA(DisplayName = "Direct Charge"),
    CircleStalk     UMETA(DisplayName = "Circle Stalk"),
    AmbushWait      UMETA(DisplayName = "Ambush Wait"),
    PackFlank       UMETA(DisplayName = "Pack Flank"),
    TerritorialRush UMETA(DisplayName = "Territorial Rush"),
    FeedingDefense  UMETA(DisplayName = "Feeding Defense")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggressiveness = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    int32 PackSize = 1;

    FCombat_DinosaurCombatStats()
    {
        AttackDamage = 50.0f;
        AttackRange = 300.0f;
        DetectionRadius = 1000.0f;
        MovementSpeed = 400.0f;
        Aggressiveness = 0.7f;
        TerritorialRadius = 2000.0f;
        bIsPackHunter = false;
        PackSize = 1;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurCombatAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinosaurCombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_DinosaurCombatState GetCombatState() const { return CurrentCombatState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DefendTerritory();

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(TArray<UCombat_DinosaurCombatAI*> PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack(AActor* Target);

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatLevel(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> ScanForThreats();

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    FCombat_DinosaurCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    ECombat_AttackPattern PreferredAttackPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    TSubclassOf<APawn> PreferredPreyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    TArray<TSubclassOf<APawn>> FearClasses;

protected:
    // Internal State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombat_DinosaurCombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    TArray<UCombat_DinosaurCombatAI*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FVector TerritoryCenter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    bool bIsPackLeader;

    // Internal Methods
    void UpdateCombatBehavior(float DeltaTime);
    void ProcessThreatDetection();
    void ExecuteAttackPattern();
    void UpdatePackCoordination();
    bool IsInAttackRange(AActor* Target);
    bool IsValidTarget(AActor* PotentialTarget);
    void MoveTowardsTarget(float DeltaTime);
    void ExecuteFlankingManeuver();
    void BroadcastPackAlert(AActor* Threat);

private:
    float StateTimer;
    float ThreatScanInterval;
    float LastThreatScan;
    FVector LastKnownTargetLocation;
};