#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector FootLocation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FRotator FootRotation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float IKAlpha;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float DistanceFromGround;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    bool bShowDebugTraces;

    // Foot bone names
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Names")
    FName LeftFootIKBoneName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Names")
    FName RightFootIKBoneName;

    // IK Data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float HipOffset;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData TraceForFoot(const FName& FootBoneName, float TraceDistanceOverride = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetFootIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector GetFootWorldLocation(const FName& FootBoneName);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void ResetIKData();

private:
    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent;

    // Internal functions
    FHitResult PerformFootTrace(const FVector& StartLocation, const FVector& EndLocation);
    float CalculateHipOffset();
    void InterpolateIKData(FAnim_FootIKData& CurrentData, const FAnim_FootIKData& TargetData, float DeltaTime);
};