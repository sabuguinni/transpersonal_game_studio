#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Combat_PackCoordinator.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackFormation : uint8
{
    Triangle        UMETA(DisplayName = "Triangle Formation"),
    Line           UMETA(DisplayName = "Line Formation"), 
    Pincer         UMETA(DisplayName = "Pincer Movement"),
    Ambush         UMETA(DisplayName = "Ambush Formation"),
    Scatter        UMETA(DisplayName = "Scatter Formation")
};

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha          UMETA(DisplayName = "Pack Alpha"),
    Beta           UMETA(DisplayName = "Pack Beta"),
    Hunter         UMETA(DisplayName = "Hunter"),
    Flanker        UMETA(DisplayName = "Flanker"),
    Distractor     UMETA(DisplayName = "Distractor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    class APawn* MemberPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    ECombat_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    FVector AssignedPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    bool bIsInPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    float DistanceToTarget;

    FCombat_PackMember()
    {
        MemberPawn = nullptr;
        Role = ECombat_PackRole::Hunter;
        AssignedPosition = FVector::ZeroVector;
        bIsInPosition = false;
        DistanceToTarget = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackTactics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Tactics")
    ECombat_PackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Tactics")
    AActor* PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Tactics")
    FVector TargetLastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Tactics")
    float CoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Tactics")
    bool bIsHunting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Tactics")
    float HuntStartTime;

    FCombat_PackTactics()
    {
        CurrentFormation = ECombat_PackFormation::Triangle;
        PrimaryTarget = nullptr;
        TargetLastKnownPosition = FVector::ZeroVector;
        CoordinationRadius = 1500.0f;
        bIsHunting = false;
        HuntStartTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_PackCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ACombat_PackCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Pack Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    FCombat_PackTactics CurrentTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    float FormationSpacing;

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AddPackMember(APawn* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void SetPrimaryTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void ChangeFormation(ECombat_PackFormation NewFormation);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void InitiateHunt();

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AbortHunt();

    // Formation Calculation
    UFUNCTION(BlueprintCallable, Category = "Pack Formation")
    TArray<FVector> CalculateFormationPositions(FVector TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Pack Formation")
    void UpdateMemberPositions();

    UFUNCTION(BlueprintCallable, Category = "Pack Formation")
    bool IsPackInFormation();

    // Pack Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void BroadcastThreatAlert(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void BroadcastTargetLost();

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void BroadcastAttackSignal();

protected:
    // Internal Methods
    void UpdatePackBehavior(float DeltaTime);
    void CheckPackCohesion();
    void AssignRolesBasedOnPosition();
    FVector GetAlphaPosition();
    FCombat_PackMember* FindPackAlpha();

    // Formation Calculations
    TArray<FVector> CalculateTriangleFormation(FVector TargetPosition);
    TArray<FVector> CalculateLineFormation(FVector TargetPosition);
    TArray<FVector> CalculatePincerFormation(FVector TargetPosition);
    TArray<FVector> CalculateAmbushFormation(FVector TargetPosition);
    TArray<FVector> CalculateScatterFormation(FVector TargetPosition);

private:
    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    bool bPackInitialized;
};