#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Combat_PackCoordinator.generated.h"

class ACombat_AIController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<APawn> Member;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ECombat_Role AssignedRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector TargetPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsInPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceToTarget;

    FCombat_PackMember()
    {
        Member = nullptr;
        AssignedRole = ECombat_Role::Flanker;
        TargetPosition = FVector::ZeroVector;
        bIsInPosition = false;
        DistanceToTarget = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_Formation FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FVector> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector CenterPoint;

    FCombat_PackFormation()
    {
        FormationType = ECombat_Formation::Circle;
        FormationRadius = 500.0f;
        CenterPoint = FVector::ZeroVector;
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
    virtual void Tick(float DeltaTime) override;

    // Pack Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    TWeakObjectPtr<APawn> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    TWeakObjectPtr<APawn> CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    float PackCohesionRadius;

    // Formation System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FCombat_PackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_TacticState CurrentTactic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationUpdateInterval;

    // Combat Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatEngagementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MinimumPackSizeForAttack;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    TArray<FString> TacticalCallouts;

public:
    // Pack Management Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void AddPackMember(APawn* NewMember, ECombat_Role Role = ECombat_Role::Flanker);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void SetPackLeader(APawn* NewLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void SetPackTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    bool IsPackMember(APawn* Pawn) const;

    // Formation Functions
    UFUNCTION(BlueprintCallable, Category = "Formation")
    void SetFormation(ECombat_Formation Formation);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void UpdateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Formation")
    FVector GetAssignedPosition(APawn* Member) const;

    UFUNCTION(BlueprintCallable, Category = "Formation")
    bool IsPackInFormation() const;

    // Combat Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiateCombat(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteTactic(ECombat_TacticState Tactic);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OrderRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssignCombatRoles();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngageTarget(APawn* PotentialTarget) const;

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastTacticalCommand(const FString& Command);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendPackSignal(ECombat_PackSignal Signal);

protected:
    // Internal Functions
    void UpdatePackCohesion(float DeltaTime);
    void EvaluateThreatLevel();
    void UpdateTacticalState(float DeltaTime);
    FVector CalculateFormationCenter() const;
    void ValidatePackMembers();

private:
    float LastFormationUpdate;
    float LastTacticalEvaluation;
    ECombat_ThreatLevel CurrentThreatLevel;
};