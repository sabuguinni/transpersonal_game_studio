#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAIManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Alert UMETA(DisplayName = "Alert"),
    Engage UMETA(DisplayName = "Engage"),
    Flank UMETA(DisplayName = "Flank"),
    Retreat UMETA(DisplayName = "Retreat"),
    Regroup UMETA(DisplayName = "Regroup")
};

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    None UMETA(DisplayName = "None"),
    Line UMETA(DisplayName = "Line"),
    Wedge UMETA(DisplayName = "Wedge"),
    Circle UMETA(DisplayName = "Circle"),
    Ambush UMETA(DisplayName = "Ambush"),
    PackHunt UMETA(DisplayName = "Pack Hunt")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalUnit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Unit")
    AActor* UnitActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Unit")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Unit")
    FVector AssignedPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Unit")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Unit")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Unit")
    bool bIsLeader;

    FCombat_TacticalUnit()
    {
        UnitActor = nullptr;
        CurrentState = ECombat_TacticalState::Idle;
        AssignedPosition = FVector::ZeroVector;
        CurrentTarget = nullptr;
        ThreatLevel = 0.0f;
        bIsLeader = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_TacticalAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Tactical management
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void RegisterUnit(AActor* Unit, bool bAsLeader = false);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UnregisterUnit(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetFormation(ECombat_FormationType Formation);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetGroupTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteTacticalManeuver(ECombat_TacticalState NewState);

    // Formation management
    UFUNCTION(BlueprintCallable, Category = "Formation")
    void CalculateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Formation")
    FVector GetFormationPosition(int32 UnitIndex) const;

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    void UpdateThreatLevels();

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    AActor* GetHighestThreatTarget() const;

    // Tactical decisions
    UFUNCTION(BlueprintCallable, Category = "Tactical Decisions")
    ECombat_TacticalState DetermineBestTactic();

    UFUNCTION(BlueprintCallable, Category = "Tactical Decisions")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Tactical Decisions")
    bool CanExecuteFlankingManeuver() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<FCombat_TacticalUnit> TacticalUnits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_FormationType CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    AActor* GroupTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float FormationSpacing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float ThreatUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float LastThreatUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float RetreatThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float FlankingRange;

private:
    void UpdateFormationCenter();
    void AssignFormationPositions();
    FVector CalculateLineFormationPosition(int32 Index) const;
    FVector CalculateWedgeFormationPosition(int32 Index) const;
    FVector CalculateCircleFormationPosition(int32 Index) const;
    FVector CalculateAmbushPosition(int32 Index) const;
    FVector CalculatePackHuntPosition(int32 Index) const;
};