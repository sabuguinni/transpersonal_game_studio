#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalStance : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Ambush          UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    float AdvantageScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    bool bIsFlankingPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    bool bHasCover;

    FCombat_TacticalPosition()
    {
        Position = FVector::ZeroVector;
        AdvantageScore = 0.0f;
        bIsFlankingPosition = false;
        bHasCover = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    AActor* ThreatTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    bool bIsDirectThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float LastSeenTime;

    FCombat_ThreatAssessment()
    {
        ThreatTarget = nullptr;
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        bIsDirectThreat = false;
        LastSeenTime = 0.0f;
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
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AnalyzeTacticalSituation();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FCombat_TacticalPosition FindOptimalPosition(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalStance(ECombat_TacticalStance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    ECombat_TacticalStance GetCurrentStance() const { return CurrentStance; }

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    AActor* GetPrimaryThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    float CalculateThreatLevel(AActor* Target) const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void RequestPackSupport(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void RespondToPackCall(const FVector& RallyPoint);

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldEngageTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector CalculateFlankingPosition(AActor* Target) const;

protected:
    // Current tactical state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tactical State")
    ECombat_TacticalStance CurrentStance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tactical State")
    FCombat_TacticalPosition CurrentPosition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tactical State")
    TArray<FCombat_ThreatAssessment> KnownThreats;

    // Tactical parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Parameters")
    float TacticalAnalysisRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Parameters")
    float ThreatAssessmentInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Parameters")
    float PackCoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Parameters")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Parameters")
    float RetreatThreshold;

    // Pack coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Coordination")
    TArray<UCombat_TacticalAI*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Coordination")
    bool bIsPackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Coordination")
    FVector PackRallyPoint;

private:
    // Internal state
    float LastThreatUpdate;
    float LastTacticalAnalysis;
    AActor* CachedPrimaryThreat;

    // Helper functions
    void UpdatePackMembers();
    float CalculatePositionAdvantage(const FVector& Position, AActor* Target) const;
    bool HasLineOfSight(const FVector& FromLocation, const FVector& ToLocation) const;
    void BroadcastTacticalInfo();
};