#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_PackHuntingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<APawn> Member;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ENPC_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector FormationOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsAlive;

    FNPC_PackMember()
    {
        Member = nullptr;
        Role = ENPC_PackRole::Follower;
        FormationOffset = FVector::ZeroVector;
        Stamina = 100.0f;
        bIsAlive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    TWeakObjectPtr<AActor> Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    float TimeSinceLastSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    bool bIsValidTarget;

    FNPC_HuntTarget()
    {
        Target = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 1.0f;
        TimeSinceLastSeen = 0.0f;
        bIsValidTarget = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_PackHuntingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PackHuntingManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void InitializePack(const TArray<APawn*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AddPackMember(APawn* NewMember, ENPC_PackRole Role, const FVector& FormationOffset);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void SetPackLeader(APawn* NewLeader);

    // Hunt Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void StartHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void StopHunt();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ExecutePackTactic(ENPC_PackTactic Tactic);

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void UpdateFormation(ENPC_PackFormation Formation);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    FVector CalculateFormationPosition(const FNPC_PackMember& Member) const;

    // Pack State
    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    bool IsPackActive() const { return bIsPackActive; }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    APawn* GetPackLeader() const { return PackLeader.Get(); }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    ENPC_PackState GetPackState() const { return CurrentPackState; }

protected:
    // Pack Members
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    TWeakObjectPtr<APawn> PackLeader;

    // Hunt Target
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hunt")
    FNPC_HuntTarget CurrentTarget;

    // Pack State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ENPC_PackState CurrentPackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ENPC_PackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackActive;

    // Pack Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Config")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Config")
    float HuntRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Config")
    float CoordinationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Config")
    float PackCohesion;

    // Hunt Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hunt")
    float HuntDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Config")
    float MaxHuntDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Config")
    float TargetLostTimeout;

private:
    // Internal Methods
    void UpdatePackBehavior(float DeltaTime);
    void UpdateHuntBehavior(float DeltaTime);
    void UpdateFormationPositions();
    void CheckPackCohesion();
    void ValidatePackMembers();
    
    // Hunt Tactics
    void ExecuteEncircleTactic();
    void ExecuteAmbushTactic();
    void ExecuteDirectAssaultTactic();
    void ExecuteHerdingTactic();
    
    // Formation Calculations
    FVector CalculateLineFormationPosition(int32 MemberIndex) const;
    FVector CalculateWedgeFormationPosition(int32 MemberIndex) const;
    FVector CalculateCircleFormationPosition(int32 MemberIndex) const;
    FVector CalculateScatterFormationPosition(int32 MemberIndex) const;
};