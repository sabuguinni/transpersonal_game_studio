#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle,
    Patrol,
    Chase,
    Attack,
    Flee,
    Social,
    Trade,
    Sleep
};

UENUM(BlueprintType)
enum class ENPC_NPCType : uint8
{
    Dinosaur,
    TribalHuman,
    WildAnimal,
    Neutral
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float IdleTime = 5.0f;

    FNPC_BehaviorSettings()
    {
        PatrolRadius = 2000.0f;
        ChaseDistance = 1500.0f;
        AttackDistance = 300.0f;
        FleeDistance = 500.0f;
        SocialDistance = 800.0f;
        PatrolSpeed = 150.0f;
        ChaseSpeed = 400.0f;
        IdleTime = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* Target = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Threat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Familiarity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime = 0.0f;

    FNPC_MemoryEntry()
    {
        Target = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        Threat = 0.0f;
        Familiarity = 0.0f;
        LastSeenTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_NPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_NPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Behavior State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_NPCType NPCType = ENPC_NPCType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_BehaviorSettings BehaviorSettings;

    // Memory System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_MemoryEntry> Memory;

    // Patrol System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
    FVector PatrolCenter = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
    FVector CurrentPatrolTarget = FVector::ZeroVector;

    // Current Target
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
    AActor* CurrentTarget = nullptr;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryEntry(AActor* Target, float Threat, float Familiarity);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_MemoryEntry* GetMemoryEntry(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemory(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void InitializePatrol();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void UpdatePatrol();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    AActor* FindNearestTarget();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    float GetDistanceToTarget(AActor* Target);

    // Social Behavior
    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateSocialBehavior();

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> FindNearbyNPCs();

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle MemoryUpdateTimer;
    
    float StateChangeTime = 0.0f;
    float LastBehaviorUpdate = 0.0f;
};