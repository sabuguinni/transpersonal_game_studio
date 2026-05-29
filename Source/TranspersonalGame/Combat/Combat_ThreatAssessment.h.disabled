#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Combat_ThreatAssessment.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat"),
    Lethal      UMETA(DisplayName = "Lethal Threat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    AActor* ThreatSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    FVector LastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bIsHostile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bCanSeeTarget;

    FCombat_ThreatData()
    {
        ThreatSource = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownPosition = FVector::ZeroVector;
        bIsHostile = false;
        bCanSeeTarget = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float CoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    bool bIsHunting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    bool bIsAmbushing;

    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        CurrentTarget = nullptr;
        CoordinationRadius = 1500.0f;
        bIsHunting = false;
        bIsAmbushing = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_ThreatAssessment : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_ThreatAssessment();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Threat Assessment Functions
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    ECombat_ThreatLevel AssessThreatLevel(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void UpdateThreatList();

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    AActor* GetHighestThreat();

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    TArray<FCombat_ThreatData> GetThreatsInRange(float Range);

    // Pack Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void InitializePackBehavior(const TArray<AActor*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    FVector GetFlankingPosition(AActor* Target, int32 FlankIndex);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    bool ShouldEngageTarget(AActor* Target);

    // Combat State Functions
    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void EnterCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void ExitCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    bool IsInCombat() const { return bIsInCombat; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    TArray<FCombat_ThreatData> CurrentThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    FCombat_PackCoordination PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float CombatTimeout;

private:
    float LastThreatUpdate;
    float CombatStartTime;

    // Internal threat calculation functions
    float CalculateDistanceThreat(float Distance);
    float CalculateSizeThreat(AActor* Actor);
    bool CanSeeActor(AActor* Actor);
    FVector CalculateInterceptPosition(AActor* Target, float PredictionTime);
};