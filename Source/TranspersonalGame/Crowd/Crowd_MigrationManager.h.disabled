#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Crowd_MigrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxGroupSize;

    FCrowd_MigrationRoute()
    {
        Speed = 300.0f;
        GroupRadius = 1000.0f;
        MaxGroupSize = 50;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MigrationGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupCohesion;

    FCrowd_MigrationGroup()
    {
        CurrentTarget = FVector::ZeroVector;
        CurrentWaypointIndex = 0;
        GroupCohesion = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MigrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MigrationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FCrowd_MigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FCrowd_MigrationGroup> ActiveGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float SeasonalMigrationChance;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartMigration(const FCrowd_MigrationRoute& Route, const TArray<AActor*>& Actors);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StopMigration(int32 GroupIndex);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void UpdateMigrationGroups(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    FVector CalculateGroupCenter(const FCrowd_MigrationGroup& Group);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void ApplyFlockingBehavior(FCrowd_MigrationGroup& Group, float DeltaTime);

private:
    float LastUpdateTime;
    
    void UpdateGroupMovement(FCrowd_MigrationGroup& Group, float DeltaTime);
    FVector CalculateSeparation(AActor* Actor, const FCrowd_MigrationGroup& Group);
    FVector CalculateAlignment(AActor* Actor, const FCrowd_MigrationGroup& Group);
    FVector CalculateCohesion(AActor* Actor, const FCrowd_MigrationGroup& Group);
};