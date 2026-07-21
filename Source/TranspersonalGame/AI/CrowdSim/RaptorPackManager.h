#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RaptorPackManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_PackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating")
};

USTRUCT(BlueprintType)
struct FCrowd_FlankPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Pack")
    FVector WorldPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Pack")
    float AngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Pack")
    bool bOccupied;

    FCrowd_FlankPosition()
        : WorldPosition(FVector::ZeroVector)
        , AngleDegrees(0.f)
        , bOccupied(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorPackManager : public AActor
{
    GENERATED_BODY()

public:
    ARaptorPackManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Pack Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Pack")
    int32 PackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Pack")
    float FlankRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Pack")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Pack")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Pack")
    float CoordinationUpdateInterval;

    // --- Pack State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Pack")
    ECrowd_PackState PackState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Pack")
    TArray<FCrowd_FlankPosition> FlankPositions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Pack")
    TWeakObjectPtr<AActor> CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Pack")
    TArray<TWeakObjectPtr<APawn>> PackMembers;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    void SetPackTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    void AddPackMember(APawn* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    void RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    void TriggerFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    void TriggerPackRetreat();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    FVector GetFlankPositionForMember(int32 MemberIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    int32 GetActiveMemberCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Pack")
    bool IsPackEngaged() const;

private:
    void UpdateFlankPositions();
    void CoordinatePackBehavior();
    void TransitionToState(ECrowd_PackState NewState);
    bool HasLineOfSightToTarget() const;

    float TimeSinceLastCoordination;
};
