#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Combat_TacticalFormations.generated.h"

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    None            UMETA(DisplayName = "No Formation"),
    Pack            UMETA(DisplayName = "Pack Formation"),
    Ambush          UMETA(DisplayName = "Ambush Formation"),
    Defensive       UMETA(DisplayName = "Defensive Circle"),
    Flanking        UMETA(DisplayName = "Flanking Maneuver"),
    HuntingLine     UMETA(DisplayName = "Hunting Line")
};

UENUM(BlueprintType)
enum class ECombat_FormationRole : uint8
{
    Leader      UMETA(DisplayName = "Pack Leader"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Support     UMETA(DisplayName = "Support"),
    Scout       UMETA(DisplayName = "Scout"),
    Defender    UMETA(DisplayName = "Defender")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_FormationPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector RelativePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bIsFlexiblePosition;

    FCombat_FormationPosition()
    {
        RelativePosition = FVector::ZeroVector;
        Role = ECombat_FormationRole::Support;
        Priority = 1.0f;
        bIsFlexiblePosition = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FCombat_FormationPosition> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float MinEffectiveDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float MaxEffectiveDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bRequiresLineOfSight;

    FCombat_TacticalFormation()
    {
        FormationType = ECombat_FormationType::None;
        FormationRadius = 500.0f;
        MinEffectiveDistance = 200.0f;
        MaxEffectiveDistance = 1000.0f;
        bRequiresLineOfSight = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalFormations : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalFormations();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    void InitializeFormation(ECombat_FormationType FormationType);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    void AddUnitToFormation(AActor* Unit, ECombat_FormationRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    void RemoveUnitFromFormation(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    void UpdateFormationTarget(AActor* NewTarget);

    // Formation Execution
    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    void ExecuteFormation();

    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    void BreakFormation();

    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    bool IsFormationActive() const { return bFormationActive; }

    // Position Calculation
    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    FVector GetFormationPositionForUnit(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    ECombat_FormationRole GetUnitRole(AActor* Unit);

    // Formation Analysis
    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    bool IsFormationEffective();

    UFUNCTION(BlueprintCallable, Category = "Tactical Formations")
    float GetFormationCohesion();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Config")
    FCombat_TacticalFormation CurrentFormation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation State")
    TMap<AActor*, ECombat_FormationRole> FormationUnits;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation State")
    AActor* FormationTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation State")
    FVector FormationCenter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation State")
    bool bFormationActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Timing")
    float FormationUpdateInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation Timing")
    float LastFormationUpdate;

private:
    void SetupPackFormation();
    void SetupAmbushFormation();
    void SetupDefensiveFormation();
    void SetupFlankingFormation();
    void SetupHuntingLineFormation();
    
    void UpdateFormationPositions();
    bool ValidateFormationIntegrity();
    void AssignOptimalRoles();
    
    FVector CalculateFormationCenter();
    float CalculateDistanceToTarget();
};