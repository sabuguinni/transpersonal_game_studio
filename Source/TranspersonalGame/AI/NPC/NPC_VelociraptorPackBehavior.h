#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NPC_VelociraptorPackBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_PackState : uint8
{
    Hunting      UMETA(DisplayName = "Hunting"),
    Coordinating UMETA(DisplayName = "Coordinating"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Retreating   UMETA(DisplayName = "Retreating")
};

USTRUCT(BlueprintType)
struct FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    APawn* Raptor;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    FVector LastKnownPosition;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    float DistanceToTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    bool bIsAlpha;

    FNPC_PackMember()
    {
        Raptor = nullptr;
        LastKnownPosition = FVector::ZeroVector;
        DistanceToTarget = 0.0f;
        bIsAlpha = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_VelociraptorPackBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_VelociraptorPackBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float HuntingRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float AttackDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MaxPackSize = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float CoordinationDelay = 2.0f;

    // Pack State
    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    ENPC_PackState CurrentPackState;

    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    APawn* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    FVector PackCenterPoint;

    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    bool bIsPackLeader;

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void FindNearbyRaptors();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackMovement();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ExecutePackHunt();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void PerformCoordinatedAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector CalculateFlankingPosition(APawn* Target, int32 RaptorIndex);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsValidTarget(APawn* PotentialTarget);

private:
    float StateTimer;
    float LastCoordinationTime;
    FVector LastTargetPosition;
    bool bTargetLost;

    void TransitionToState(ENPC_PackState NewState);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateCoordinatingBehavior(float DeltaTime);
    void UpdateAttackingBehavior(float DeltaTime);
    void UpdateRetreatingBehavior(float DeltaTime);
};