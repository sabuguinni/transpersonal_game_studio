#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle,
    Scouting,
    Stalking,
    Flanking,
    Attacking,
    Retreating,
    Regrouping
};

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    None,
    Line,
    Circle,
    Wedge,
    Pincer,
    Ambush
};

USTRUCT(BlueprintType)
struct FCombat_TacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    FVector Position;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    float Priority;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    bool bIsOccupied;

    UPROPERTY(BlueprintReadWrite, Category = "Tactical")
    AActor* OccupyingActor;

    FCombat_TacticalPosition()
    {
        Position = FVector::ZeroVector;
        Priority = 0.0f;
        bIsOccupied = false;
        OccupyingActor = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FCombat_TacticalFormation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    ECombat_FormationType FormationType;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    TArray<FCombat_TacticalPosition> Positions;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    FVector CenterPoint;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    AActor* Target;

    FCombat_TacticalFormation()
    {
        FormationType = ECombat_FormationType::None;
        CenterPoint = FVector::ZeroVector;
        FormationRadius = 500.0f;
        Target = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
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
    void InitializeTacticalAI(AActor* InTarget, TArray<AActor*> InAllies);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticalState GetTacticalState() const { return CurrentTacticalState; }

    // Formation management
    UFUNCTION(BlueprintCallable, Category = "Formation")
    void CreateFormation(ECombat_FormationType FormationType, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    FVector GetAssignedPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Formation")
    bool AssignPositionInFormation(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void UpdateFormationPositions();

    // Tactical decision making
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void EvaluateTacticalSituation();

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldFlank() const;

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector FindFlankingPosition() const;

    // Communication with allies
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastTacticalOrder(ECombat_TacticalState OrderType);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceiveTacticalOrder(ECombat_TacticalState OrderType, AActor* Sender);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(BlueprintReadOnly, Category = "Target")
    AActor* PrimaryTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Allies")
    TArray<AActor*> AlliedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Formation")
    FCombat_TacticalFormation CurrentFormation;

    UPROPERTY(BlueprintReadOnly, Category = "Position")
    FVector AssignedPosition;

    UPROPERTY(BlueprintReadOnly, Category = "Position")
    int32 FormationIndex;

    // Tactical parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Settings")
    float FlankingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Settings")
    float RetreatHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Settings")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Settings")
    float TacticalUpdateInterval;

    // Timers
    UPROPERTY()
    float LastTacticalUpdate;

    UPROPERTY()
    float StateChangeTimer;

private:
    // Formation generation helpers
    TArray<FVector> GenerateLineFormation(const FVector& Center, const FVector& Direction, int32 NumPositions);
    TArray<FVector> GenerateCircleFormation(const FVector& Center, float Radius, int32 NumPositions);
    TArray<FVector> GenerateWedgeFormation(const FVector& Center, const FVector& Direction, int32 NumPositions);
    TArray<FVector> GeneratePincerFormation(const FVector& Center, const FVector& Direction, int32 NumPositions);
    TArray<FVector> GenerateAmbushFormation(const FVector& Center, const FVector& Direction, int32 NumPositions);

    // Tactical analysis
    float CalculateThreatLevel() const;
    bool HasClearLineOfSight(const FVector& From, const FVector& To) const;
    bool IsPositionSafe(const FVector& Position) const;
    FVector GetOptimalAttackVector() const;
};