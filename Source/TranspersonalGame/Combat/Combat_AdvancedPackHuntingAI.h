#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_AdvancedPackHuntingAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackFormation : uint8
{
    Loose,
    Tight,
    Wedge,
    Line,
    Circle,
    Ambush
};

UENUM(BlueprintType)
enum class ECombat_HuntingPhase : uint8
{
    Patrol,
    Detection,
    Approach,
    Surround,
    Attack,
    Pursuit,
    Retreat
};

USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<APawn> MemberPawn;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    ECombat_PackRole Role;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    FVector FormationPosition;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    float HealthPercentage;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    bool bIsActive;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    float LastCommunicationTime;

    FCombat_PackMember()
    {
        Role = ECombat_PackRole::Follower;
        FormationPosition = FVector::ZeroVector;
        HealthPercentage = 100.0f;
        bIsActive = true;
        LastCommunicationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombat_HuntingTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Hunting")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(BlueprintReadWrite, Category = "Hunting")
    FVector LastKnownPosition;

    UPROPERTY(BlueprintReadWrite, Category = "Hunting")
    FVector PredictedPosition;

    UPROPERTY(BlueprintReadWrite, Category = "Hunting")
    float ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Hunting")
    float TrackingTime;

    UPROPERTY(BlueprintReadWrite, Category = "Hunting")
    bool bIsValidTarget;

    FCombat_HuntingTarget()
    {
        LastKnownPosition = FVector::ZeroVector;
        PredictedPosition = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        TrackingTime = 0.0f;
        bIsValidTarget = false;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AdvancedPackHuntingAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AdvancedPackHuntingAI();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void InitializePackHunting(APawn* AlphaPawn);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AddPackMember(APawn* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void RemovePackMember(APawn* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void SetPackFormation(ECombat_PackFormation NewFormation);

    // Hunting Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void UpdateHuntingPhase(ECombat_HuntingPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ExecuteCoordinatedAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AbortHunt();

    // Formation Control
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    FVector CalculateFormationPosition(int32 MemberIndex, const FVector& CenterPosition);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void UpdateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AdaptFormationToTerrain();

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void BroadcastPackSignal(ECombat_PackSignal Signal);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ReceivePackSignal(ECombat_PackSignal Signal, APawn* Sender);

    // Tactical Analysis
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    float CalculateHuntingSuccess();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AnalyzeThreatLevel(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void UpdateTacticalAssessment();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    ECombat_HuntingPhase GetCurrentHuntingPhase() const { return CurrentHuntingPhase; }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    bool IsHunting() const { return CurrentTarget.bIsValidTarget; }

protected:
    // Pack Data
    UPROPERTY(BlueprintReadOnly, Category = "Pack Hunting")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Hunting")
    TWeakObjectPtr<APawn> AlphaPawn;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Hunting")
    ECombat_PackFormation CurrentFormation;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Hunting")
    ECombat_HuntingPhase CurrentHuntingPhase;

    // Hunting Data
    UPROPERTY(BlueprintReadOnly, Category = "Pack Hunting")
    FCombat_HuntingTarget CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Hunting")
    TArray<FVector> FormationWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Hunting")
    float HuntingStartTime;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float MaxPackDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float FormationTightness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float HuntingTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float MinThreatLevelToHunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float MaxThreatLevelToEngage;

private:
    // Internal Methods
    void UpdatePackCohesion(float DeltaTime);
    void ProcessPackCommunication(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void CalculateOptimalFormation();
    void PredictTargetMovement();
    void AssignRolesToMembers();
    bool ValidatePackMember(const FCombat_PackMember& Member);
    FVector GetPackCenterPosition();
    void HandleMemberLoss(APawn* LostMember);
    void AdjustFormationForObstacles();
    float CalculateFormationEffectiveness();

    // Timers
    float LastFormationUpdate;
    float LastCommunicationBroadcast;
    float LastTacticalAssessment;
};