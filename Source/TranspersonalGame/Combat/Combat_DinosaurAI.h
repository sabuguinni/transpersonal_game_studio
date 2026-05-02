#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Combat_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Brachio     UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Pteranodon  UMETA(DisplayName = "Pteranodon")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    bool bIsPackHunter = false;

    FCombat_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 200.0f;
        DetectionRange = 800.0f;
        MovementSpeed = 300.0f;
        Aggression = 0.7f;
        bIsPackHunter = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurAI : public APawn
{
    GENERATED_BODY()

public:
    ACombat_DinosaurAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DinosaurMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* AttackSphere;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ECombat_DinosaurSpecies Species = ECombat_DinosaurSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FCombat_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FString DinosaurName = "Unknown";

    // AI State
    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    ECombat_DinosaurState CurrentState = ECombat_DinosaurState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    FVector PatrolOrigin;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    FVector PatrolTarget;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<ACombat_DinosaurAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ACombat_DinosaurAI* PackLeader = nullptr;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString CombatVoiceURL;

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetDinosaurState(ECombat_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void JoinPack(ACombat_DinosaurAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PlayCombatVoice();

    // Detection Events
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
    // AI Behavior Functions
    void UpdateAI(float DeltaTime);
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateAttackingBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);

    // Movement Functions
    void MoveTowards(FVector TargetLocation, float DeltaTime);
    FVector GetRandomPatrolPoint();
    bool IsValidTarget(AActor* Target);

    // Pack Behavior Functions
    void UpdatePackBehavior(float DeltaTime);
    void CoordinatePackAttack();
    void FormPackFormation();

    // Timer Handles
    FTimerHandle AttackCooldownTimer;
    FTimerHandle PatrolTimer;
    FTimerHandle VoiceTimer;

    // Internal State
    float LastAttackTime = 0.0f;
    float AttackCooldown = 2.0f;
    float PatrolRadius = 500.0f;
    float TimeInCurrentState = 0.0f;
};