#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Combat_PackHuntingAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha UMETA(DisplayName = "Alpha Leader"),
    Hunter UMETA(DisplayName = "Hunter"),
    Flanker UMETA(DisplayName = "Flanker"),
    Scout UMETA(DisplayName = "Scout")
};

USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Pack AI")
    AActor* Member;

    UPROPERTY(BlueprintReadOnly, Category = "Pack AI")
    ECombat_PackRole Role;

    UPROPERTY(BlueprintReadOnly, Category = "Pack AI")
    FVector AssignedPosition;

    UPROPERTY(BlueprintReadOnly, Category = "Pack AI")
    bool bIsActive;

    FCombat_PackMember()
    {
        Member = nullptr;
        Role = ECombat_PackRole::Hunter;
        AssignedPosition = FVector::ZeroVector;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PackHuntingAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PackHuntingAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    float PackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Pack AI")
    bool bIsHunting;

    UPROPERTY(BlueprintReadOnly, Category = "Pack AI")
    bool bIsAlpha;

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void InitializePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void AddPackMember(AActor* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void RemovePackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void StartHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void ExecutePackTactics();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void AssignFlankingPositions();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinateAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    AActor* FindNearestThreat();

private:
    float LastCoordinationUpdate;
    float CoordinationInterval;
    
    void UpdatePackFormation();
    void SendPackCommand(const FString& Command);
};