#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntityProcessor.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_TribalFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    int32 TribeID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float SocialStatus = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FVector WorkLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 CurrentBehavior = 0; // 0=Idle, 1=Work, 2=Hunt, 3=Gather, 4=Social, 5=Rest

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsGroupLeader = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityProcessor();

    // UMassProcessor interface
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

protected:
    // Mass Entity queries
    FMassEntityQuery TribalBehaviorQuery;
    FMassEntityQuery MovementQuery;
    FMassEntityQuery SocialInteractionQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxEntitiesPerFrame = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float BehaviorUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SocialInteractionRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float WorkLocationRadius = 500.0f;

    // Behavior processing methods
    UFUNCTION(BlueprintCallable, Category = "Crowd Processing")
    void ProcessTribalBehaviors(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Crowd Processing")
    void ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Crowd Processing")
    void ProcessSocialInteractions(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Crowd Processing")
    void UpdateBehaviorState(FCrowd_TribalFragment& TribalData, FCrowd_BehaviorFragment& BehaviorData, float DeltaTime);

private:
    float LastUpdateTime = 0.0f;
    int32 ProcessedEntitiesThisFrame = 0;
};