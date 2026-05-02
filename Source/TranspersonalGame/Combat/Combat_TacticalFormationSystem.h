#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "CombatAITypes.h"
#include "Combat_TacticalFormationSystem.generated.h"

class APawn;
class ACombatAIController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_FormationSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector RelativePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TWeakObjectPtr<APawn> OccupyingUnit;

    FCombat_FormationSlot()
    {
        RelativePosition = FVector::ZeroVector;
        Priority = 1.0f;
        bIsOccupied = false;
        OccupyingUnit = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString FormationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FCombat_FormationSlot> Slots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationType FormationType;

    FCombat_TacticalFormation()
    {
        FormationName = TEXT("Default");
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 500.0f;
        FormationType = ECombat_FormationType::Circle;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_TacticalFormationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalFormationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void InitializeFormation(const FString& FormationName, ECombat_FormationType Type, int32 MaxUnits);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    bool AddUnitToFormation(APawn* Unit);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void RemoveUnitFromFormation(APawn* Unit);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void UpdateFormationCenter(const FVector& NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    FVector GetFormationPositionForUnit(APawn* Unit);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void BreakFormation();

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void ReformFormation();

    // Formation Queries
    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    bool IsFormationActive() const { return bFormationActive; }

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    int32 GetFormationSize() const { return UnitsInFormation.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    float GetFormationCohesion() const;

    // Tactical Commands
    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void ExecuteFlankingManeuver(const FVector& TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void ExecutePincerAttack(const FVector& TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void ExecuteRetreat(const FVector& RetreatDirection);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FCombat_TacticalFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<TWeakObjectPtr<APawn>> UnitsInFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bFormationActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationTolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float ReformationDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FTimerHandle ReformationTimer;

private:
    void CreateCircleFormation(int32 NumSlots);
    void CreateLineFormation(int32 NumSlots);
    void CreateWedgeFormation(int32 NumSlots);
    void CreateAmbushFormation(int32 NumSlots);

    void UpdateFormationPositions();
    bool IsUnitInPosition(APawn* Unit, const FVector& TargetPosition) const;
    void AssignUnitToSlot(APawn* Unit, int32 SlotIndex);
    int32 FindBestSlotForUnit(APawn* Unit) const;
};