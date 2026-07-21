#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "MassEntityTypes.h"
#include "MassProcessingTypes.h"
#include "MassCommonTypes.h"
#include "SharedTypes.h"
#include "Crowd_GroupFormationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FormationSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector LocalPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FMassEntityHandle OccupyingEntity;

    FCrowd_FormationSlot()
    {
        LocalPosition = FVector::ZeroVector;
        Priority = 1.0f;
        bIsOccupied = false;
        OccupyingEntity = FMassEntityHandle();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_GroupFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FCrowd_FormationSlot> FormationSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECrowd_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    int32 MaxMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FMassEntityHandle> GroupMembers;

    FCrowd_GroupFormation()
    {
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 500.0f;
        FormationType = ECrowd_FormationType::Circle;
        MaxMembers = 10;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_GroupFormationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_GroupFormationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    int32 CreateFormation(ECrowd_FormationType FormationType, FVector Center, float Radius, int32 MaxMembers);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    bool AddEntityToFormation(int32 FormationID, FMassEntityHandle Entity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    bool RemoveEntityFromFormation(int32 FormationID, FMassEntityHandle Entity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    void UpdateFormationCenter(int32 FormationID, FVector NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    FVector GetFormationSlotPosition(int32 FormationID, FMassEntityHandle Entity);

    // Formation Types
    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    void GenerateCircleFormation(FCrowd_GroupFormation& Formation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    void GenerateLineFormation(FCrowd_GroupFormation& Formation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    void GenerateWedgeFormation(FCrowd_GroupFormation& Formation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    void GenerateColumnFormation(FCrowd_GroupFormation& Formation);

    // Cohesion and Separation
    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    FVector CalculateCohesionForce(FMassEntityHandle Entity, const TArray<FMassEntityHandle>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    FVector CalculateSeparationForce(FMassEntityHandle Entity, const TArray<FMassEntityHandle>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    FVector CalculateAlignmentForce(FMassEntityHandle Entity, const TArray<FMassEntityHandle>& Neighbors);

    // Formation Maintenance
    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    void MaintainFormationIntegrity(int32 FormationID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    bool IsFormationIntact(int32 FormationID, float ToleranceRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Formation")
    void ReorganizeFormation(int32 FormationID);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    TMap<int32, FCrowd_GroupFormation> ActiveFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    float FormationTolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    float ReorganizationCooldown;

private:
    int32 NextFormationID;
    float LastReorganizationTime;

    void InitializeFormationSlots(FCrowd_GroupFormation& Formation);
    FCrowd_FormationSlot* FindBestAvailableSlot(FCrowd_GroupFormation& Formation, FVector EntityPosition);
    void UpdateFormationSlotPositions(FCrowd_GroupFormation& Formation);
};