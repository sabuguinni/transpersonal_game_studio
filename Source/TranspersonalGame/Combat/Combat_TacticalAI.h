#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"), 
    Ambush      UMETA(DisplayName = "Ambush"),
    Flank       UMETA(DisplayName = "Flank"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Rally       UMETA(DisplayName = "Rally"),
    Defend      UMETA(DisplayName = "Defend")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float CoverValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bIsFlankingPosition;

    FCombat_TacticalPosition()
    {
        Position = FVector::ZeroVector;
        CoverValue = 0.0f;
        ThreatLevel = 0.0f;
        bIsFlankingPosition = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<FVector> MemberPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector LeaderPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ECombat_TacticalState FormationType;

    FCombat_PackFormation()
    {
        FormationRadius = 500.0f;
        FormationType = ECombat_TacticalState::Patrol;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAI : public AActor
{
    GENERATED_BODY()

public:
    ACombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TacticalRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<FCombat_TacticalPosition> TacticalPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_PackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float ThreatAssessmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float RetreatThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    bool bIsPackLeader;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void InitiatePackFormation(ECombat_TacticalState FormationType);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FCombat_TacticalPosition FindOptimalPosition(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteRetreatProtocol();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool IsPositionSafe(FVector Position);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalAssessment();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AddPackMember(AActor* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void RemovePackMember(AActor* Member);

private:
    void CalculateTacticalPositions();
    void UpdatePackFormation();
    float CalculateThreatLevel(AActor* Actor);
    FVector GetFlankingPosition(AActor* Target);
    void BroadcastTacticalOrder(ECombat_TacticalState Order);
};