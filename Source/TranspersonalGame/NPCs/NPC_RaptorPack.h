#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "SharedTypes.h"
#include "NPC_RaptorPack.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_RaptorPackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 PackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsHunting;

    FNPC_RaptorPackData()
    {
        PackSize = 3;
        PackRadius = 1500.0f;
        HuntingRange = 5000.0f;
        bIsHunting = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_RaptorPack : public APawn
{
    GENERATED_BODY()

public:
    ANPC_RaptorPack();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCapsuleComponent* CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPawnSensingComponent* PawnSensing;

    // Pack behavior data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FNPC_RaptorPackData PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float MaxHealth;

    // Pack coordination
    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    TArray<ANPC_RaptorPack*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    APawn* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    FVector PatrolCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    bool bIsPackLeader;

public:
    virtual void Tick(float DeltaTime) override;

    // Pack behavior functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePackMember(bool bAsLeader = false);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddToPackMember(ANPC_RaptorPack* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackTarget(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void StartHunting();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void StopHunting();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void PatrolArea();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsTargetInRange() const;

    // Damage and health
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Die();

protected:
    UFUNCTION()
    void OnSeePlayer(APawn* SeenPawn);

    UFUNCTION()
    void OnDetectionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    FVector GetRandomPatrolPoint() const;
    void UpdatePackCoordination();
    void MoveTowardsTarget(float DeltaTime);
    void MoveTowardsLocation(const FVector& TargetLocation, float DeltaTime);
};