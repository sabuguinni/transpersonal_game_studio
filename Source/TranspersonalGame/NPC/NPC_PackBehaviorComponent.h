#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NPC_PackBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_PackRole : uint8
{
    Leader      UMETA(DisplayName = "Pack Leader"),
    Alpha       UMETA(DisplayName = "Alpha Member"), 
    Beta        UMETA(DisplayName = "Beta Member"),
    Omega       UMETA(DisplayName = "Omega Member"),
    Lone        UMETA(DisplayName = "Lone Wolf")
};

UENUM(BlueprintType)
enum class ENPC_PackState : uint8
{
    Idle        UMETA(DisplayName = "Idle/Roaming"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Coordinated Attack"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Retreat/Flee"),
    Territorial UMETA(DisplayName = "Defending Territory")
};

USTRUCT(BlueprintType)
struct FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<APawn> PackMember;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceFromLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAlive;

    FNPC_PackMember()
    {
        PackMember = nullptr;
        Role = ENPC_PackRole::Omega;
        DistanceFromLeader = 0.0f;
        bIsAlive = true;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PackBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PackBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(APawn* NewMember, ENPC_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackState(ENPC_PackState NewState);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    APawn* GetPackLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    TArray<APawn*> GetPackMembers() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsInPack() const;

    // Hunting Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void InitiatePackHunt(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    FVector GetFlankingPosition(APawn* Target, int32 MemberIndex);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void CoordinateAttack(APawn* Target);

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void SendPackSignal(const FString& SignalType);

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void ReceivePackSignal(const FString& SignalType, APawn* Sender);

protected:
    // Pack Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    ENPC_PackRole MyPackRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    ENPC_PackState CurrentPackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TWeakObjectPtr<APawn> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TWeakObjectPtr<APawn> CurrentTarget;

    // Pack Behavior Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float MaxPackDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float FlankingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bCanFormPacks;

    // Timers and State
    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    float TimeSinceLastHunt;

    UPROPERTY(BlueprintReadOnly, Category = "Pack State")
    float TimeSinceLastSignal;

private:
    void UpdatePackFormation();
    void UpdateHuntingBehavior();
    void CheckPackCohesion();
    FVector CalculateFormationPosition();
    void HandlePackStateTransition(ENPC_PackState NewState);
};