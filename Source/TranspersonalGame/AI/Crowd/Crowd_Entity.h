#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Crowd_Entity.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorType : uint8
{
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Wanderer    UMETA(DisplayName = "Wanderer"),
    Guard       UMETA(DisplayName = "Guard"),
    Scout       UMETA(DisplayName = "Scout")
};

USTRUCT(BlueprintType)
struct FCrowd_EntityStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Speed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float GroupCohesion = 0.7f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_Entity : public APawn
{
    GENERATED_BODY()

public:
    ACrowd_Entity();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EntityMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECrowd_BehaviorType BehaviorType = ECrowd_BehaviorType::Wanderer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FCrowd_EntityStats EntityStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WanderRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float GroupDistance = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector TargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsMoving = false;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorType(ECrowd_BehaviorType NewBehaviorType);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Group")
    TArray<ACrowd_Entity*> FindNearbyEntities(float Radius);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector CalculateFlockingForce();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector CalculateSeparationForce();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector CalculateAlignmentForce();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector CalculateCohesionForce();

private:
    void ExecuteGathererBehavior(float DeltaTime);
    void ExecuteHunterBehavior(float DeltaTime);
    void ExecuteWandererBehavior(float DeltaTime);
    void ExecuteGuardBehavior(float DeltaTime);
    void ExecuteScoutBehavior(float DeltaTime);

    float BehaviorTimer = 0.0f;
    float NextBehaviorChange = 5.0f;
};