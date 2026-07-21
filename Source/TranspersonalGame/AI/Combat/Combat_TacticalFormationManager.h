#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Combat_TacticalFormationManager.generated.h"

class APawn;
class UBehaviorTreeComponent;

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
    TWeakObjectPtr<APawn> AssignedPawn;

    FCombat_FormationSlot()
    {
        RelativePosition = FVector::ZeroVector;
        Priority = 1.0f;
        bIsOccupied = false;
        AssignedPawn = nullptr;
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
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_TacticalState RequiredState;

    FCombat_TacticalFormation()
    {
        FormationName = TEXT("Default");
        FormationRadius = 500.0f;
        RequiredState = ECombat_TacticalState::Hunting;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_TacticalFormationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalFormationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    bool CreateFormation(const FString& FormationName, const TArray<FVector>& SlotPositions, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    bool AssignPawnToFormation(APawn* Pawn, const FString& FormationName);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    bool RemovePawnFromFormation(APawn* Pawn);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    FVector GetAssignedPosition(APawn* Pawn) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    bool UpdateFormationCenter(const FString& FormationName, const FVector& NewCenter);

    // Tactical Analysis
    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    float CalculateFormationCohesion(const FString& FormationName) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    TArray<APawn*> GetFormationMembers(const FString& FormationName) const;

    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    bool IsFormationComplete(const FString& FormationName) const;

    // Combat Coordination
    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    void ExecuteCoordinatedAttack(const FString& FormationName, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    void ExecuteDefensiveManeuver(const FString& FormationName, const FVector& ThreatDirection);

    UFUNCTION(BlueprintCallable, Category = "Tactical Formation")
    void ExecuteFlankingManeuver(const FString& FormationName, AActor* Target);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Config")
    TMap<FString, FCombat_TacticalFormation> AvailableFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Config")
    FString ActiveFormationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Config")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Config")
    float MaxFormationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Config")
    float FormationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Config")
    bool bAutoUpdateFormations;

private:
    float LastUpdateTime;
    
    void UpdateFormationPositions();
    void ValidateFormationIntegrity();
    FVector CalculateOptimalSlotPosition(const FCombat_FormationSlot& Slot, const FVector& Center) const;
    bool IsSlotPositionValid(const FVector& Position) const;
};