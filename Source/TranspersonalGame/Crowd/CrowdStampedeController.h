#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Dispersing  UMETA(DisplayName = "Dispersing")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    AActor* MemberActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_StampedeState State = ECrowd_StampedeState::Grazing;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 HerdSize = 20;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed = 1200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicRadius = 800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float SeparationForce = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float CohesionForce = 80.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float AlignmentForce = 120.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float TrampleDamage = 45.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_StampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_StampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector PanicOrigin, FVector FleeDirection);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void AddHerdMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void SetStampedeConfig(const FCrowd_StampedeConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    ECrowd_StampedeState GetHerdState() const { return CurrentHerdState; }

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    int32 GetHerdSize() const { return HerdMembers.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void DisbandHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    FVector ComputeFlockingForce(const FCrowd_HerdMember& Member);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede", meta = (AllowPrivateAccess = "true"))
    ECrowd_StampedeState CurrentHerdState;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FCrowd_StampedeConfig StampedeConfig;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector StampedeDirection;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeTimer;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float MaxStampedeDuration;

private:
    void UpdateHerdBehavior(float DeltaTime);
    void ApplyFlockingToMembers(float DeltaTime);
    void CheckPlayerProximity();
    void TransitionToState(ECrowd_StampedeState NewState);
};
