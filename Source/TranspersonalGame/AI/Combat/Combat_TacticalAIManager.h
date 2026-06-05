#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/SphereComponent.h"
#include "../../SharedTypes.h"
#include "Combat_TacticalAIManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FVector> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    int32 MaxUnits;

    FCombat_TacticalFormation()
    {
        FormationType = ECombat_FormationType::Circle;
        FormationRadius = 300.0f;
        MaxUnits = 5;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalUnit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    AActor* UnitActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    ECombat_UnitRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    FVector AssignedPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    float CombatEffectiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    bool bIsInCombat;

    FCombat_TacticalUnit()
    {
        UnitActor = nullptr;
        Role = ECombat_UnitRole::Attacker;
        AssignedPosition = FVector::ZeroVector;
        CombatEffectiveness = 1.0f;
        bIsInCombat = false;
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
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* DetectionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<FCombat_TacticalUnit> ManagedUnits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<FCombat_TacticalFormation> AvailableFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float FormationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    AActor* PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FVector CombatCenter;

public:
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void RegisterCombatUnit(AActor* Unit, ECombat_UnitRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UnregisterCombatUnit(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetFormation(ECombat_FormationType FormationType);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AssignTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector GetOptimalAttackPosition(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver);

protected:
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void UpdateFormationPositions();
    void AnalyzeTacticalSituation();
    void IssueUnitOrders();
    void CalculateCombatEffectiveness();

    float LastFormationUpdate;
    TArray<AActor*> DetectedEnemies;
    TArray<AActor*> DetectedAllies;
};