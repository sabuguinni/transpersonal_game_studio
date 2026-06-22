#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Flee        UMETA(DisplayName = "Flee"),
    Alert       UMETA(DisplayName = "Alert"),
    Interact    UMETA(DisplayName = "Interact"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTime;

    FNPC_PatrolPoint() : Location(FVector::ZeroVector), WaitTime(2.0f) {}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AlertNearbyNPCs(float AlertRadius);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AddPatrolPoint(FVector Location, float WaitTime = 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentState;

private:
    int32 CurrentPatrolIndex;
    float WaitTimer;
    bool bWaiting;

    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickFlee(float DeltaTime);
};
