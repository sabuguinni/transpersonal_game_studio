#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Crowd_PrehistoricTribalFormation.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TribalFormationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Formation")
    FVector CenterPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Formation")
    float InnerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Formation")
    float OuterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Formation")
    int32 EntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Formation")
    ECrowd_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Formation")
    float RotationSpeed;

    FCrowd_TribalFormationData()
    {
        CenterPoint = FVector::ZeroVector;
        InnerRadius = 300.0f;
        OuterRadius = 600.0f;
        EntityCount = 25;
        FormationType = ECrowd_FormationType::Circle;
        RotationSpeed = 10.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TribalMemberData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
    APawn* EntityPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
    FVector AssignedPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
    float AngularPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
    ECrowd_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Member")
    bool bIsActive;

    FCrowd_TribalMemberData()
    {
        EntityPawn = nullptr;
        AssignedPosition = FVector::ZeroVector;
        AngularPosition = 0.0f;
        TribalRole = ECrowd_TribalRole::Member;
        MovementSpeed = 50.0f;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_PrehistoricTribalFormation : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_PrehistoricTribalFormation();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void InitializeTribalFormation(const FCrowd_TribalFormationData& FormationData);

    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void AddTribalMember(APawn* NewMember, ECrowd_TribalRole Role = ECrowd_TribalRole::Member);

    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void RemoveTribalMember(APawn* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void UpdateFormationCenter(const FVector& NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void ChangeFormationType(ECrowd_FormationType NewType);

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void StartTribalGathering();

    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void StopTribalGathering();

    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void SetFormationRotation(bool bShouldRotate, float RotationSpeed = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Tribal Formation")
    void TriggerTribalDispersion(float DispersionRadius = 1000.0f);

    // Query Functions
    UFUNCTION(BlueprintPure, Category = "Tribal Formation")
    int32 GetTribalMemberCount() const;

    UFUNCTION(BlueprintPure, Category = "Tribal Formation")
    TArray<APawn*> GetTribalMembers() const;

    UFUNCTION(BlueprintPure, Category = "Tribal Formation")
    FVector GetFormationCenter() const;

    UFUNCTION(BlueprintPure, Category = "Tribal Formation")
    bool IsFormationActive() const;

protected:
    // Formation Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    FCrowd_TribalFormationData FormationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation State")
    TArray<FCrowd_TribalMemberData> TribalMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation State")
    bool bFormationActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation State")
    bool bShouldRotate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation State")
    float CurrentRotationAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    float FormationTolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Settings")
    float MemberSpacing;

private:
    // Internal Formation Logic
    void UpdateTribalPositions(float DeltaTime);
    void CalculateFormationPositions();
    FVector GetPositionForMember(int32 MemberIndex) const;
    void MoveTribalMemberToPosition(FCrowd_TribalMemberData& Member, const FVector& TargetPosition, float DeltaTime);
    void HandleFormationRotation(float DeltaTime);
    void ValidateTribalMembers();

    // Formation Patterns
    FVector CalculateCirclePosition(int32 Index, int32 TotalCount, float Radius) const;
    FVector CalculateWedgePosition(int32 Index, int32 TotalCount) const;
    FVector CalculateLinePosition(int32 Index, int32 TotalCount) const;
};