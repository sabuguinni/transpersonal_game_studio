#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdHerdBehavior.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    AActor* MemberActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsLeader = false;
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdHerdBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdHerdBehavior();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void AddHerdMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RemoveHerdMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerStampede(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SetHerdState(ECrowd_HerdState NewState);

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    ECrowd_HerdState GetHerdState() const { return CurrentHerdState; }

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    int32 GetHerdSize() const { return HerdMembers.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector ComputeFlockingVelocity(const FCrowd_HerdMember& Member);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float AlignmentRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float GrazeSpeed = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float WanderSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float FearDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MaxHerdSize = 50;

private:
    UPROPERTY()
    TArray<FCrowd_HerdMember> HerdMembers;

    ECrowd_HerdState CurrentHerdState = ECrowd_HerdState::Grazing;

    FVector StampedeThreatLocation = FVector::ZeroVector;
    float StateTimer = 0.0f;

    void UpdateGrazing(float DeltaTime);
    void UpdateWandering(float DeltaTime);
    void UpdateFleeing(float DeltaTime);
    void UpdateStampeding(float DeltaTime);
    void UpdateResting(float DeltaTime);

    FVector ComputeSeparation(const FCrowd_HerdMember& Member);
    FVector ComputeCohesion(const FCrowd_HerdMember& Member);
    FVector ComputeAlignment(const FCrowd_HerdMember& Member);
};
