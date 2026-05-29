#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AI/NavigationSystemBase.h"
#include "NavigationSystem.h"
#include "SharedTypes.h"
#include "Crowd_PrehistoricHerdManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    bool bIsAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Member")
    ECrowd_DinosaurSpecies Species;

    FCrowd_HerdMember()
    {
        Actor = nullptr;
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Health = 100.0f;
        Fear = 0.0f;
        bIsAlpha = false;
        Species = ECrowd_DinosaurSpecies::Triceratops;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdBehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    ECrowd_HerdState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    FVector HerdCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    FVector MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float AlertLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    AActor* ThreatTarget;

    FCrowd_HerdBehaviorState()
    {
        CurrentState = ECrowd_HerdState::Grazing;
        HerdCenter = FVector::ZeroVector;
        MovementDirection = FVector::ZeroVector;
        HerdRadius = 1000.0f;
        AlertLevel = 0.0f;
        ThreatTarget = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_PrehistoricHerdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_PrehistoricHerdManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core herd management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    FCrowd_HerdBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    ECrowd_DinosaurSpecies HerdSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    int32 MaxHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    float AlignmentRadius;

    // Movement parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WanderDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WanderJitter;

    // Threat detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    TArray<TSubclassOf<AActor>> ThreatClasses;

    // Grazing behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grazing")
    float GrazingDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grazing")
    float GrazingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grazing")
    TArray<FVector> GrazingSpots;

    // Migration behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float WaypointReachRadius;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* HerdAudioComponent;

    // Methods
    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void AddHerdMember(AActor* NewMember, bool bIsAlpha = false);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void RemoveHerdMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void SetHerdState(ECrowd_HerdState NewState);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    FVector CalculateHerdCenter();

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void UpdateHerdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector CalculateCohesionForce(const FCrowd_HerdMember& Member);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector CalculateSeparationForce(const FCrowd_HerdMember& Member);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector CalculateAlignmentForce(const FCrowd_HerdMember& Member);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector CalculateWanderForce(FCrowd_HerdMember& Member);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector CalculateSeekForce(const FCrowd_HerdMember& Member, FVector Target);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector CalculateFleeForce(const FCrowd_HerdMember& Member, FVector Threat);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void HandleThreatResponse(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void UpdateMigration(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Grazing")
    void StartGrazing();

    UFUNCTION(BlueprintCallable, Category = "Grazing")
    void UpdateGrazing(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayHerdSound(ECrowd_HerdState State);

private:
    float GrazingTimer;
    FVector WanderTarget;
    float LastThreatCheckTime;
    float ThreatCheckInterval;
};