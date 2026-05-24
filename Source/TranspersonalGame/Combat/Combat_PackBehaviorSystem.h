#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Combat_PackBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Leader"),
    Beta        UMETA(DisplayName = "Beta Support"),
    Scout       UMETA(DisplayName = "Scout"),
    Ambusher    UMETA(DisplayName = "Ambusher"),
    Flanker     UMETA(DisplayName = "Flanker")
};

UENUM(BlueprintType)
enum class ECombat_PackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Coordinating UMETA(DisplayName = "Coordinating"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<APawn> MemberPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ECombat_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector LastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceFromTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsInPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float AggressionLevel;

    FCombat_PackMember()
    {
        MemberPawn = nullptr;
        Role = ECombat_PackRole::Beta;
        LastKnownPosition = FVector::ZeroVector;
        DistanceFromTarget = 0.0f;
        bIsInPosition = false;
        AggressionLevel = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector AlphaPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector BetaPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector ScoutPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector AmbushPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector FlankPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    FCombat_PackFormation()
    {
        AlphaPosition = FVector::ZeroVector;
        BetaPosition = FVector::ZeroVector;
        ScoutPosition = FVector::ZeroVector;
        AmbushPosition = FVector::ZeroVector;
        FlankPosition = FVector::ZeroVector;
        FormationRadius = 500.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PackBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PackBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePack(APawn* AlphaPawn);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(APawn* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(APawn* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackTarget(APawn* NewTarget);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackFormation();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinateAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitiateAmbush();

    // Pack State Management
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackState(ECombat_PackState NewState);

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    ECombat_PackState GetPackState() const { return CurrentPackState; }

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    bool IsPackInPosition() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TWeakObjectPtr<APawn> PackTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    ECombat_PackState CurrentPackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    FCombat_PackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float CoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float AttackCoordinationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float AmbushWaitTime;

private:
    void UpdateMemberPositions();
    void CalculateOptimalFormation();
    void CheckPackCohesion();
    void HandlePackCommunication();
    
    FCombat_PackMember* FindPackMember(APawn* Pawn);
    FVector CalculateFlankingPosition(const FVector& TargetPosition, const FVector& AlphaPosition);
    bool IsValidAmbushPosition(const FVector& Position);
};