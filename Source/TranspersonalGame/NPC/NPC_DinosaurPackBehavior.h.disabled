#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPC_DinosaurPackBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_PackRole : uint8
{
    Leader      UMETA(DisplayName = "Pack Leader"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Scout       UMETA(DisplayName = "Scout"),
    Guardian    UMETA(DisplayName = "Guardian")
};

UENUM(BlueprintType)
enum class ENPC_PackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Feeding     UMETA(DisplayName = "Feeding")
};

USTRUCT(BlueprintType)
struct FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APawn* DinosaurPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StaminaPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAlive;

    FNPC_PackMember()
    {
        DinosaurPawn = nullptr;
        Role = ENPC_PackRole::Hunter;
        HealthPercentage = 100.0f;
        StaminaPercentage = 100.0f;
        LastKnownPosition = FVector::ZeroVector;
        bIsAlive = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurPackBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurPackBehavior();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    APawn* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ENPC_PackState CurrentPackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float AttackCoordinationDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MinPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MaxPackSize;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePack(APawn* Leader, const TArray<APawn*>& Members);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(APawn* NewMember, ENPC_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackTarget(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ExecutePackHunt();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ExecuteCoordinatedAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RetreatPack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackIntact() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetPackCenterPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    APawn* GetNearestPackMember(FVector Position) const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackFormation();

private:
    void UpdatePackState();
    void ExecutePackBehavior(float DeltaTime);
    void AssignPackRoles();
    FVector CalculateFlankingPosition(APawn* Target, int32 MemberIndex);
    bool IsTargetWithinHuntingRange(APawn* Target) const;
    void CommunicateWithPack(const FString& Message);

    float LastStateUpdateTime;
    float LastCommunicationTime;
    TArray<FVector> FormationPositions;
};