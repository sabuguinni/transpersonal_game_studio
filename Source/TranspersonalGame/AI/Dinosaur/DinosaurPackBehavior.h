#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "DinosaurPackBehavior.generated.h"

UENUM(BlueprintType)
enum class EDinosaurPackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Leader"),
    Beta        UMETA(DisplayName = "Beta Second"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Scout       UMETA(DisplayName = "Scout"),
    Follower    UMETA(DisplayName = "Follower")
};

UENUM(BlueprintType)
enum class EPackBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Aggressive  UMETA(DisplayName = "Aggressive")
};

USTRUCT(BlueprintType)
struct FPackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class APawn* DinosaurPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurPackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPosition;

    FPackMember()
    {
        DinosaurPawn = nullptr;
        Role = EDinosaurPackRole::Follower;
        HealthPercentage = 100.0f;
        HungerLevel = 50.0f;
        bIsAlive = true;
        LastKnownPosition = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurPackBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurPackBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<FPackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    EPackBehaviorState CurrentPackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    class APawn* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    class AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float HuntingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackMoraleLevel;

public:
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(APawn* NewMember, EDinosaurPackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(APawn* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackState(EPackBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitiateHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitiateFlee(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinateAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetPackCenterPosition();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    TArray<APawn*> GetLivingPackMembers();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackIntact();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackMorale();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AssignPackRoles();

private:
    void UpdatePackCohesion();
    void UpdatePackMemberStatus();
    void HandlePackCommunication();
    void ExecutePackFormation();
    
    float LastCohesionUpdate;
    float LastMoraleUpdate;
    float PackFormationSpacing;
};