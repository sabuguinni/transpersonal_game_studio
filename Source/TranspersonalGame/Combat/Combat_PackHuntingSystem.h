#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Combat_PackHuntingSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Leader"),
    Beta        UMETA(DisplayName = "Beta Support"),
    Scout       UMETA(DisplayName = "Scout"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Ambusher    UMETA(DisplayName = "Ambusher")
};

UENUM(BlueprintType)
enum class ECombat_PackTactic : uint8
{
    Surround    UMETA(DisplayName = "Surround Target"),
    HitAndRun   UMETA(DisplayName = "Hit and Run"),
    Ambush      UMETA(DisplayName = "Ambush"),
    Retreat     UMETA(DisplayName = "Tactical Retreat"),
    HoldGround  UMETA(DisplayName = "Hold Ground")
};

USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    APawn* PackMember;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    ECombat_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    float DistanceFromTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    bool bIsAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    float AggressionLevel;

    FCombat_PackMember()
    {
        PackMember = nullptr;
        Role = ECombat_PackRole::Beta;
        DistanceFromTarget = 0.0f;
        bIsAlive = true;
        AggressionLevel = 0.5f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PackHuntingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PackHuntingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AddPackMember(APawn* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void SetPackTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ExecutePackTactic(ECombat_PackTactic Tactic);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void UpdatePackFormation();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void CoordinateAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    APawn* GetAlphaLeader() const;

    // Pack Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void SendPackSignal(const FString& Signal);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ReceivePackSignal(const FString& Signal, APawn* Sender);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    ECombat_PackTactic CurrentTactic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float PackCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float RetreatThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float CoordinationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    bool bPackActive;

private:
    float LastCoordinationUpdate;
    
    void UpdatePackRoles();
    void CalculateOptimalPositions();
    FVector GetFlankingPosition(APawn* Member, APawn* Target);
    FVector GetAmbushPosition(APawn* Member, APawn* Target);
    bool IsPackIntact() const;
    float CalculatePackMorale() const;
};