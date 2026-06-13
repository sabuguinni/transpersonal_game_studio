#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimationBlueprintLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_AdvancedIKSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_IKSolverType : uint8
{
    TwoBone     UMETA(DisplayName = "Two Bone IK"),
    FABRIK      UMETA(DisplayName = "FABRIK"),
    CCDIK       UMETA(DisplayName = "CCD IK"),
    Custom      UMETA(DisplayName = "Custom Solver")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKChainData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Chain")
    FName StartBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Chain")
    FName MiddleBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Chain")
    FName EndBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Chain")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Chain")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Chain")
    float ChainWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Chain")
    EAnim_IKSolverType SolverType;

    FAnim_IKChainData()
    {
        StartBone = NAME_None;
        MiddleBone = NAME_None;
        EndBone = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        ChainWeight = 1.0f;
        SolverType = EAnim_IKSolverType::TwoBone;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float GroundDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKWeight = 0.0f;
        GroundDistance = 0.0f;
        bIsGrounded = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_AdvancedIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_AdvancedIKSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Chain Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Chains")
    TArray<FAnim_IKChainData> IKChains;

    // Foot IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK;

    // Hand IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    FVector LeftHandTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    FVector RightHandTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    float HandIKWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    bool bEnableHandIK;

    // Look At IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    FVector LookAtTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float LookAtWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    bool bEnableLookAtIK;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateHandIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateLookAtIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SolveIKChain(FAnim_IKChainData& ChainData);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetIKChainTarget(int32 ChainIndex, const FVector& TargetLocation, const FRotator& TargetRotation);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetFootIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetHandIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetLookAtIKEnabled(bool bEnabled);

private:
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Internal IK calculation functions
    void CalculateFootIKData(FAnim_FootIKData& FootData, const FName& FootBoneName, float DeltaTime);
    FVector SolveTwoBoneIK(const FVector& RootPos, const FVector& JointPos, const FVector& EndPos, const FVector& TargetPos, float UpperLength, float LowerLength);
    void ApplyIKToSkeleton();
};