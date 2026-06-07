#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

// Forward declarations
class UBehaviorTreeComponent;
class UBlackboardComponent;
class APawn;
class AActor;

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol,
    Alert,
    Hunting,
    Engaging,
    Flanking,
    Retreating,
    Coordinating,
    Ambushing
};

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    Solo,
    PairHunt,
    PackSurround,
    LineFormation,
    AmbushCircle,
    PincerMovement
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    ECombat_FormationType Formation = ECombat_FormationType::Solo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    float PackCoordination = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    float TimeSinceLastContact = 0.0f;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_TacticalState::Patrol;
        Formation = ECombat_FormationType::Solo;
        AggressionLevel = 0.5f;
        FearLevel = 0.0f;
        PackCoordination = 1.0f;
        LastKnownTargetLocation = FVector::ZeroVector;
        TimeSinceLastContact = 0.0f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tactical AI functions
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void InitializeTacticalAI(ECombat_FormationType InitialFormation = ECombat_FormationType::Solo);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void UpdateTacticalState(AActor* Target, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    ECombat_TacticalState GetTacticalState() const { return TacticalData.CurrentState; }

    // Formation and coordination
    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void SetFormation(ECombat_FormationType NewFormation);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void CoordinateWithPack(const TArray<UCombat_TacticalAI*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    FVector CalculateFormationPosition(int32 PackIndex, int32 TotalPackSize);

    // Target assessment and engagement
    UFUNCTION(BlueprintCallable, Category = "Combat Target")
    bool ShouldEngageTarget(AActor* PotentialTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat Target")
    float AssessTargetThreat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Target")
    void UpdateTargetTracking(AActor* Target, float DeltaTime);

    // Tactical decision making
    UFUNCTION(BlueprintCallable, Category = "Combat Decision")
    void MakeTacticalDecision(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat Decision")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat Decision")
    bool ShouldCallForBackup();

    // Animation integration (bridge with Agent #11's work)
    UFUNCTION(BlueprintCallable, Category = "Combat Animation")
    void TriggerCombatAnimation(const FString& AnimationState);

    UFUNCTION(BlueprintCallable, Category = "Combat Animation")
    void UpdateAnimationFromTacticalState();

    // Audio integration
    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void PlayCombatAudio(const FString& AudioType);

protected:
    // Core tactical data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical", meta = (AllowPrivateAccess = "true"))
    FCombat_TacticalData TacticalData;

    // AI behavior references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComponent;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters", meta = (AllowPrivateAccess = "true"))
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters", meta = (AllowPrivateAccess = "true"))
    float EngagementRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters", meta = (AllowPrivateAccess = "true"))
    float RetreatThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters", meta = (AllowPrivateAccess = "true"))
    float PackCoordinationRadius = 2000.0f;

    // Current target and pack references
    UPROPERTY(BlueprintReadOnly, Category = "Combat State", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State", meta = (AllowPrivateAccess = "true"))
    TArray<UCombat_TacticalAI*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State", meta = (AllowPrivateAccess = "true"))
    int32 PackIndex = 0;

private:
    // Internal tactical calculations
    void CalculateAggressionLevel(float DeltaTime);
    void CalculateFearLevel(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    
    // State transition logic
    void TransitionToAlert();
    void TransitionToHunting();
    void TransitionToEngaging();
    void TransitionToRetreating();
    
    // Formation calculations
    FVector CalculateSoloPosition();
    FVector CalculatePairHuntPosition();
    FVector CalculatePackSurroundPosition();
    FVector CalculateLineFormationPosition();
    FVector CalculateAmbushCirclePosition();
    FVector CalculatePincerMovementPosition();
};