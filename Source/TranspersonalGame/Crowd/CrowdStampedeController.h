#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Dispersed   UMETA(DisplayName = "Dispersed")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader;

    FCrowd_HerdMember()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , FearLevel(0.0f)
        , bIsLeader(false)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector FleeDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float PanicRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdSize;

    FCrowd_StampedeData()
        : TriggerLocation(FVector::ZeroVector)
        , FleeDirection(FVector::ForwardVector)
        , StampedeSpeed(800.0f)
        , PanicRadius(1500.0f)
        , HerdSize(20)
    {}
};

/**
 * ACrowd_StampedeController
 * Manages herd stampede behavior for prehistoric animals.
 * Triggered by predator proximity, loud sounds, or player actions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_StampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_StampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Trigger a stampede from a specific location */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector TriggerLocation, FVector FleeDirection, float PanicRadius);

    /** Get current stampede state */
    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    ECrowd_StampedeState GetStampedeState() const { return CurrentState; }

    /** Get number of active herd members */
    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    int32 GetActiveHerdSize() const { return HerdMembers.Num(); }

    /** Add a herd member to this stampede controller */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void AddHerdMember(FVector Location, bool bIsLeader = false);

    /** Calm the herd (reduce fear over time) */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void CalmHerd(float CalmRate = 0.1f);

    /** Check if player is in danger zone of stampede */
    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    bool IsPlayerInDangerZone(FVector PlayerLocation) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    ECrowd_StampedeState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_StampedeData StampedeData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Members")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MaxStampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FearDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float DangerZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AlignmentForce;

private:
    void UpdateFlockingBehavior(float DeltaTime);
    void UpdateStampedeMovement(float DeltaTime);
    FVector CalculateSeparation(int32 MemberIndex) const;
    FVector CalculateCohesion(int32 MemberIndex) const;
    FVector CalculateAlignment(int32 MemberIndex) const;
    float AverageFearLevel() const;
};
