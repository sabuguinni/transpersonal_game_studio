#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Attack      UMETA(DisplayName = "Attack"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackCoord   UMETA(DisplayName = "Pack Coordination")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    Predator_Solo    UMETA(DisplayName = "Solo Predator"),
    Predator_Pack    UMETA(DisplayName = "Pack Predator"),
    Herbivore_Small  UMETA(DisplayName = "Small Herbivore"),
    Herbivore_Large  UMETA(DisplayName = "Large Herbivore"),
    Apex_Predator    UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatDetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanFlank = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bUsesPackTactics = false;

    FCombat_TacticalData()
    {
        ThreatDetectionRange = 1000.0f;
        AttackRange = 200.0f;
        FlankingDistance = 500.0f;
        RetreatThreshold = 0.3f;
        PackSize = 3;
        bCanFlank = true;
        bUsesPackTactics = false;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tactical functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticalState GetTacticalState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinosaurType(ECombat_DinosaurType Type);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float LastThreatCheckTime;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    bool bIsPackLeader;

private:
    void UpdateThreatAssessment();
    void ExecuteHuntBehavior();
    void ExecuteTerritorialBehavior();
    void ExecuteRetreatBehavior();
    void UpdatePackCoordination();
    AActor* FindNearestThreat();
    bool IsInAttackRange(AActor* Target) const;
    FVector GetRetreatDirection() const;
};