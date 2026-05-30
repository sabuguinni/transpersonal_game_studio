#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NPC_DinosaurHerdBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Moving      UMETA(DisplayName = "Moving"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Drinking    UMETA(DisplayName = "Drinking")
};

USTRUCT(BlueprintType)
struct FNPC_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APawn* DinosaurPawn = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RelativePosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceFromLeader = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLeader = false;

    FNPC_HerdMember()
    {
        DinosaurPawn = nullptr;
        RelativePosition = FVector::ZeroVector;
        DistanceFromLeader = 0.0f;
        bIsLeader = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurHerdBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurHerdBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Herd Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Settings")
    float HerdRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Settings")
    int32 MaxHerdSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Settings")
    float GrazingDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Settings")
    float MovementSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Settings")
    float FleeSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Settings")
    float DangerDetectionRadius = 1500.0f;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Herd State")
    ENPC_HerdState CurrentState = ENPC_HerdState::Grazing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Herd State")
    TArray<FNPC_HerdMember> HerdMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Herd State")
    FVector HerdCenterLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Herd State")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Herd State")
    float StateTimer = 0.0f;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void InitializeHerd();

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void AddHerdMember(APawn* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void RemoveHerdMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void SetHerdState(ENPC_HerdState NewState);

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    bool DetectDanger();

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    void UpdateHerdFormation();

    UFUNCTION(BlueprintCallable, Category = "Herd Behavior")
    FVector GetFleeDirection();

private:
    void UpdateGrazingBehavior(float DeltaTime);
    void UpdateMovingBehavior(float DeltaTime);
    void UpdateAlertBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);
    void UpdateDrinkingBehavior(float DeltaTime);

    void FindNearbyDinosaurs();
    void CalculateHerdCenter();
    void MoveHerdToTarget(float DeltaTime);
    
    APawn* GetHerdLeader();
    bool IsNearWater();
    FVector FindNearestWaterSource();
};