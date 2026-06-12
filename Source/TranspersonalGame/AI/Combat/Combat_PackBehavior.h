#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Combat_PackBehavior.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<AActor> MemberActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ECombat_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceFromLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsAlive;

    FCombat_PackMember()
    {
        MemberActor = nullptr;
        Role = ECombat_PackRole::Follower;
        DistanceFromLeader = 0.0f;
        bIsAlive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_FormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FVector> RelativePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    FCombat_PackFormation()
    {
        FormationType = ECombat_FormationType::Circle;
        FormationRadius = 300.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PackBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PackBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePack(AActor* Leader, const TArray<AActor*>& Members);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(AActor* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackFormation(ECombat_FormationType NewFormation);

    // Combat Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Combat")
    void InitiatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Combat")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Combat")
    void RetreatFormation();

    // Pack State
    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    bool IsPackLeader() const;

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    int32 GetPackSize() const;

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    AActor* GetPackLeader() const;

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    TArray<AActor*> GetLivingMembers() const;

protected:
    // Pack Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<AActor> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    FCombat_PackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    ECombat_PackState PackState;

    // Combat Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackCoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float RetreatThreshold;

    // Internal Methods
    void UpdatePackFormation();
    void UpdatePackState();
    FVector CalculateFormationPosition(int32 MemberIndex);
    void BroadcastPackCommand(ECombat_PackCommand Command, AActor* Target = nullptr);

private:
    float LastFormationUpdate;
    TWeakObjectPtr<AActor> CurrentTarget;
};