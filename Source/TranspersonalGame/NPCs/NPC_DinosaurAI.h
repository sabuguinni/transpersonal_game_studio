#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Brachio     UMETA(DisplayName = "Brachiosaurus"),
    Trike       UMETA(DisplayName = "Triceratops"),
    Stego       UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Aggressive  UMETA(DisplayName = "Aggressive")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Damage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Speed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.3f;
};

USTRUCT(BlueprintType)
struct FNPC_PackBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<class ANPC_DinosaurAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    class ANPC_DinosaurAI* PackLeader = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurAI : public APawn
{
    GENERATED_BODY()

public:
    ANPC_DinosaurAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DinosaurMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* AttackSphere;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_PackBehavior PackBehavior;

    // AI Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FVector PatrolCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FVector CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AttackCooldown = 2.0f;

public:
    // Core AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void InitializeDinosaur(ENPC_DinosaurSpecies InSpecies, FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateAI(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void HandleDetection();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void MoveToTarget(FVector Target, float AcceptanceRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ANPC_DinosaurAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void UpdatePackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    TArray<ANPC_DinosaurAI*> FindNearbyDinosaurs(float SearchRadius = 2000.0f);

    // Species-Specific Behavior
    UFUNCTION(BlueprintCallable, Category = "Species")
    void ConfigureSpeciesStats();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void HandleTRexBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void HandleRaptorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species")
    void HandleHerbivoreBehavior();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    AActor* GetNearestPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsPlayerInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomPatrolPoint() const;

protected:
    // Detection Events
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnAttackSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    // Internal timers
    FTimerHandle StateUpdateTimer;
    FTimerHandle PackUpdateTimer;
    FTimerHandle PatrolTimer;

    // Internal state
    bool bHasTarget = false;
    bool bIsMoving = false;
    FVector LastKnownPlayerLocation;
    float TimeSinceLastPlayerSighting = 0.0f;
};