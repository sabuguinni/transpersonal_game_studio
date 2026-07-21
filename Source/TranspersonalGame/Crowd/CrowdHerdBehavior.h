#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdHerdBehavior.generated.h"

// ============================================================
// Crowd Herd Behavior — Agent #13 Crowd & Traffic Simulation
// Implements flocking (Boids), herd cohesion, separation,
// alignment for herbivore dinosaur herds (up to 200 agents).
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing      UMETA(DisplayName = "Grazing"),
    Wandering    UMETA(DisplayName = "Wandering"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    Stampeding   UMETA(DisplayName = "Stampeding"),
    Resting      UMETA(DisplayName = "Resting"),
    Migrating    UMETA(DisplayName = "Migrating")
};

UENUM(BlueprintType)
enum class ECrowd_HerdRole : uint8
{
    Leader       UMETA(DisplayName = "Leader"),
    Follower     UMETA(DisplayName = "Follower"),
    Scout        UMETA(DisplayName = "Scout"),
    Juvenile     UMETA(DisplayName = "Juvenile")
};

USTRUCT(BlueprintType)
struct FCrowd_BoidParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float AlignmentRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float SeparationWeight = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float MaxForce = 150.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdRole Role = ECrowd_HerdRole::Follower;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float PanicLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsAlive = true;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    int32 AgentID = -1;
};

UCLASS(ClassGroup=(Crowd), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdHerdBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdHerdBehavior();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Herd State ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState CurrentHerdState = ECrowd_HerdState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MaxHerdSize = 40;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float HerdCenterRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FCrowd_BoidParams BoidParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float PanicDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float PanicThresholdFlee = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float PanicThresholdStampede = 0.75f;

    // ---- Members ----
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    float AveragePanicLevel = 0.0f;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void InitializeHerd(int32 NumMembers, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void ApplyThreatAtLocation(FVector ThreatLocation, float ThreatRadius, float PanicAmount);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SetHerdState(ECrowd_HerdState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector ComputeBoidForce(const FCrowd_HerdMember& Agent) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector GetHerdFleeDirection(FVector ThreatLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    int32 GetAliveCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void KillMember(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    bool IsHerdInPanic() const;

private:
    void UpdateHerdCenter();
    void UpdatePanicLevels(float DeltaTime);
    void EvaluateHerdStateTransition();
    FVector ComputeSeparation(const FCrowd_HerdMember& Agent) const;
    FVector ComputeAlignment(const FCrowd_HerdMember& Agent) const;
    FVector ComputeCohesion(const FCrowd_HerdMember& Agent) const;
    FVector LimitVector(FVector V, float MaxMagnitude) const;
};
