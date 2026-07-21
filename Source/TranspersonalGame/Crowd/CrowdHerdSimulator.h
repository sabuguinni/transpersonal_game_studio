#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdHerdSimulator.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdRole : uint8
{
    Leader      UMETA(DisplayName = "Leader"),
    Follower    UMETA(DisplayName = "Follower"),
    Scout       UMETA(DisplayName = "Scout"),
    Straggler   UMETA(DisplayName = "Straggler")
};

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    AActor* Actor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdRole Role = ECrowd_HerdRole::Follower;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector TargetOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius = 200.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxMembers = 10;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionStrength = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationStrength = 1.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Config")
    float AlignmentStrength = 0.8f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Config")
    float WanderRadius = 3000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Config")
    float FleeRadius = 1500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Config")
    float MoveSpeed = 300.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Config")
    bool bIsPack = false;
};

/**
 * UCrowdHerdSimulator — Boids-based herd simulation component.
 * Implements cohesion, separation, and alignment for dinosaur herds.
 * Attach to a HerdManager actor; register member actors via RegisterMember().
 */
UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent), DisplayName = "Crowd Herd Simulator")
class TRANSPERSONALGAME_API UCrowdHerdSimulator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdHerdSimulator();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_HerdConfig HerdConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_HerdState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    FVector HerdCentroid;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveMemberCount;

    /** Register a new actor as a herd member */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RegisterMember(AActor* NewMember, ECrowd_HerdRole Role = ECrowd_HerdRole::Follower);

    /** Remove an actor from the herd */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void UnregisterMember(AActor* Member);

    /** Trigger flee behavior — all members scatter from ThreatLocation */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerFlee(FVector ThreatLocation);

    /** Return herd to grazing state */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void ReturnToGrazing();

    /** Get the current leader actor */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    AActor* GetLeader() const;

    /** Compute centroid of all active members */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector ComputeCentroid() const;

    /** Debug: print herd state to log */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Debug")
    void LogHerdState() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<FCrowd_HerdMember> Members;

    float TickAccumulator;
    static constexpr float TickInterval = 0.1f; // 10Hz crowd update

    void UpdateCentroid();
    void ApplyBoids(float DeltaTime);
    FVector ComputeCohesion(const FCrowd_HerdMember& Member) const;
    FVector ComputeSeparation(const FCrowd_HerdMember& Member) const;
    FVector ComputeAlignment(const FCrowd_HerdMember& Member) const;
    void ElectNewLeader();
};
