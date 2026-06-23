#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Interact    UMETA(DisplayName = "Interact"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_Role : uint8
{
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Watcher     UMETA(DisplayName = "Watcher"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer")
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeLastSeen;

    FNPC_Memory()
        : LastKnownThreatLocation(FVector::ZeroVector)
        , ThreatLevel(0.0f)
        , TimeLastSeen(0.0f)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
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
    void RegisterThreat(FVector ThreatLocation, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearMemory();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetThreatLevel() const { return Memory.ThreatLevel; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_Role NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FNPC_Memory Memory;

private:
    ENPC_BehaviorState CurrentState;

    float StateTimer;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickFlee(float DeltaTime);
};
