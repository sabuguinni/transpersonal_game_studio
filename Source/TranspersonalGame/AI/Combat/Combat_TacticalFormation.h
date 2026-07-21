#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TacticalFormation.generated.h"

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
    TWeakObjectPtr<APawn> AssignedUnit;

    FCombat_FormationSlot()
    {
        RelativePosition = FVector::ZeroVector;
        Priority = 1.0f;
        bIsOccupied = false;
        AssignedUnit = nullptr;
    }
};

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    Line        UMETA(DisplayName = "Line Formation"),
    Wedge       UMETA(DisplayName = "Wedge Formation"),
    Circle      UMETA(DisplayName = "Circle Formation"),
    Ambush      UMETA(DisplayName = "Ambush Formation"),
    Scatter     UMETA(DisplayName = "Scatter Formation")
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalFormation : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalFormation();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void SetFormationType(ECombat_FormationType NewFormationType);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void AddUnitToFormation(APawn* Unit);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void RemoveUnitFromFormation(APawn* Unit);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    FVector GetFormationPosition(APawn* Unit) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void UpdateFormationCenter(const FVector& NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void RotateFormation(float RotationAngle);

    // Formation Queries
    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    bool IsFormationComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    int32 GetFormationSize() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    float GetFormationCohesion() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FCombat_FormationSlot> FormationSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float MaxFormationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationTightness;

private:
    void GenerateFormationSlots();
    void GenerateLineFormation();
    void GenerateWedgeFormation();
    void GenerateCircleFormation();
    void GenerateAmbushFormation();
    void GenerateScatterFormation();

    FVector CalculateSlotWorldPosition(const FCombat_FormationSlot& Slot) const;
    void AssignUnitsToSlots();
    float CalculateUnitDistance(APawn* Unit, const FVector& TargetPosition) const;
};