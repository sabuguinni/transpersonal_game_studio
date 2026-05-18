#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DistanceToTarget = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bHasLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsFlankingTarget = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackMembersNearby = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastAttackTime = 0.0f;

    FCombat_TacticalState()
    {
        ThreatLevel = ECombat_ThreatLevel::None;
        DistanceToTarget = 0.0f;
        bHasLineOfSight = false;
        bIsFlankingTarget = false;
        PackMembersNearby = 0;
        LastAttackTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ECombat_PackTactic CurrentTactic = ECombat_PackTactic::Surround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float TacticStartTime = 0.0f;

    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        PrimaryTarget = nullptr;
        CurrentTactic = ECombat_PackTactic::Surround;
        TacticStartTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Tactical Analysis
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_TacticalState AnalyzeTacticalSituation(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AttackPattern SelectOptimalAttackPattern(const FCombat_TacticalState& TacticalState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalPosition(AActor* Target, ECombat_AttackPattern Pattern);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void InitializePackCoordination(const TArray<AActor*>& PackMembers, AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void UpdatePackTactic(ECombat_PackTactic NewTactic);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    FVector GetPackPosition(int32 MemberIndex, ECombat_PackTactic Tactic);

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel AssessThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat(const FCombat_TacticalState& TacticalState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldCallForBackup(const FCombat_TacticalState& TacticalState);

    // Combat Timing
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackNow(float CooldownTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterAttack();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FCombat_TacticalState CurrentTacticalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    FCombat_PackCoordination PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float TacticalUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float LastTacticalUpdate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float FlankingAngle = 45.0f;

private:
    bool IsLineOfSightClear(AActor* Target);
    float CalculateFlankingAdvantage(AActor* Target);
    TArray<AActor*> FindNearbyPackMembers(float SearchRadius = 1000.0f);
};