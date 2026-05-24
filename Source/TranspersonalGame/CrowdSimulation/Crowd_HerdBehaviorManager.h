#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassProcessingTypes.h"
#include "MassProcessor.h"
#include "MassObserverProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_HerdBehaviorManager.generated.h"

// Herd behavior fragments for Mass Entity
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdMemberFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FMassEntityHandle HerdLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    ECrowd_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float PreferredDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float MaxSeparationDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float CohesionStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float SeparationStrength = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float AlignmentStrength = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    bool bIsHerdLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float LastUpdateTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdStateFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd State")
    ECrowd_HerdBehavior CurrentBehavior = ECrowd_HerdBehavior::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd State")
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd State")
    float HerdRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd State")
    int32 HerdSize = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd State")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd State")
    FVector ThreatDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd State")
    float StateChangeTime = 0.0f;
};

// Herd behavior processor
UCLASS()
class TRANSPERSONALGAME_API UCrowd_HerdBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_HerdBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery HerdMemberQuery;
    FMassEntityQuery HerdLeaderQuery;

    // Herd behavior calculations
    FVector CalculateCohesion(const FMassEntityManager& EntityManager, const FMassExecutionContext& Context, 
                             FMassEntityHandle Entity, const FCrowd_HerdMemberFragment& HerdData);
    
    FVector CalculateSeparation(const FMassEntityManager& EntityManager, const FMassExecutionContext& Context,
                               FMassEntityHandle Entity, const FCrowd_HerdMemberFragment& HerdData);
    
    FVector CalculateAlignment(const FMassEntityManager& EntityManager, const FMassExecutionContext& Context,
                              FMassEntityHandle Entity, const FCrowd_HerdMemberFragment& HerdData);

    void UpdateHerdState(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessThreatResponse(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float UpdateFrequency = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float NeighborSearchRadius = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float ThreatDetectionRadius = 800.0f;
};

// Herd manager component for individual actors
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_HerdBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_HerdBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    ECrowd_DinosaurSpecies Species = ECrowd_DinosaurSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    bool bIsHerdLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float HerdRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float PreferredDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float AlignmentWeight = 1.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Herd State")
    ECrowd_HerdBehavior CurrentBehavior = ECrowd_HerdBehavior::Grazing;

    UPROPERTY(BlueprintReadOnly, Category = "Herd State")
    TArray<AActor*> HerdMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Herd State")
    FVector HerdCenter = FVector::ZeroVector;

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void SetHerdBehavior(ECrowd_HerdBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void AddHerdMember(AActor* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void RemoveHerdMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    FVector GetDesiredMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void DetectThreats();

private:
    void UpdateHerdMembers();
    void UpdateHerdCenter();
    FVector CalculateFlockingForce() const;
    FVector CalculateCohesionForce() const;
    FVector CalculateSeparationForce() const;
    FVector CalculateAlignmentForce() const;

    float LastUpdateTime = 0.0f;
    float UpdateInterval = 0.1f;
};

// Herd spawner for creating organized dinosaur groups
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_HerdSpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_HerdSpawner();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Spawning")
    ECrowd_DinosaurSpecies SpeciesToSpawn = ECrowd_DinosaurSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Spawning")
    int32 HerdSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Spawning")
    float SpawnRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Spawning")
    TSubclassOf<APawn> DinosaurClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Spawning")
    bool bSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Spawning")
    bool bUseMassEntity = true;

    UPROPERTY(BlueprintReadOnly, Category = "Herd State")
    TArray<AActor*> SpawnedHerd;

    UFUNCTION(BlueprintCallable, Category = "Herd Spawning")
    void SpawnHerd();

    UFUNCTION(BlueprintCallable, Category = "Herd Spawning")
    void SpawnMassEntityHerd();

    UFUNCTION(BlueprintCallable, Category = "Herd Spawning")
    void DestroyHerd();

private:
    void SpawnTraditionalHerd();
    FVector GetRandomSpawnLocation() const;
};