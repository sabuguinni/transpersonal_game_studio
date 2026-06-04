#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "../../SharedTypes.h"
#include "Combat_AdvancedFormationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_FormationSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector RelativePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bIsOccupied;

    FCombat_FormationSlot()
    {
        RelativePosition = FVector::ZeroVector;
        Priority = 1.0f;
        Role = ECombat_FormationRole::Flanker;
        bIsOccupied = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FCombat_FormationSlot> FormationSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector CenterPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationType FormationType;

    FCombat_TacticalFormation()
    {
        CenterPosition = FVector::ZeroVector;
        FormationRadius = 500.0f;
        FormationType = ECombat_FormationType::PackHunt;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_AdvancedFormationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AdvancedFormationSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation", meta = (AllowPrivateAccess = "true"))
    FCombat_TacticalFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation", meta = (AllowPrivateAccess = "true"))
    TArray<APawn*> FormationMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation", meta = (AllowPrivateAccess = "true"))
    APawn* FormationLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation", meta = (AllowPrivateAccess = "true"))
    float FormationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation", meta = (AllowPrivateAccess = "true"))
    float MaxFormationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation", meta = (AllowPrivateAccess = "true"))
    bool bFormationActive;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void InitializeFormation(ECombat_FormationType InFormationType, APawn* Leader);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    bool AddMemberToFormation(APawn* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void RemoveMemberFromFormation(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void UpdateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Formation")
    FVector GetAssignedPosition(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void SetFormationType(ECombat_FormationType NewType);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void ActivateFormation(bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    bool IsFormationIntact();

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver);

private:
    void CreateFormationSlots(ECombat_FormationType FormationType);
    void AssignMembersToSlots();
    FVector CalculateSlotWorldPosition(const FCombat_FormationSlot& Slot);
    void HandleFormationBreakdown();
};