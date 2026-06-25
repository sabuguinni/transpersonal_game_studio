#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdHerdManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Alerting    UMETA(DisplayName = "Alerting")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    AActor* MeshActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WanderRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader = false;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FString HerdID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FCrowd_HerdMember> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdBehavior CurrentBehavior = ECrowd_HerdBehavior::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ThreatRadius = 2000.0f;
};

/**
 * UCrowdHerdManager — manages dinosaur herd groups for crowd simulation.
 * Handles grazing, migration, flee response, and LOD management.
 * Agent #13 — Crowd & Traffic Simulation
 */
UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdHerdManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdHerdManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Register a herd group for simulation */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterHerd(const FCrowd_HerdGroup& HerdGroup);

    /** Trigger flee response for all herds within radius of threat location */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius);

    /** Update herd behavior state */
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdBehavior(const FString& HerdID, ECrowd_HerdBehavior NewBehavior);

    /** Get current behavior of a herd */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    ECrowd_HerdBehavior GetHerdBehavior(const FString& HerdID) const;

    /** Max LOD draw distance for crowd actors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Performance")
    float MaxCrowdDrawDistance = 8000.0f;

    /** Tick interval for crowd AI updates (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Performance")
    float CrowdTickInterval = 0.5f;

    /** All registered herds */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
    TArray<FCrowd_HerdGroup> ActiveHerds;

private:
    float TimeSinceLastTick = 0.0f;

    void UpdateHerdPositions(float DeltaTime);
    void UpdateHerdBehaviors();
    FVector GetFleeDirection(FVector MemberLocation, FVector ThreatLocation) const;
};
