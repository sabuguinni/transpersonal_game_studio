#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "CollisionQueryParams.h"
#include "Engine/HitResult.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK")
    FVector FootOffset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootOffset = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = false;
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // IK Configuration
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    float FootHeight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    FName LeftFootBoneName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    FName RightFootBoneName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "IK Settings")
    FName PelvisBoneName;

    // IK Data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float PelvisOffset;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData CalculateFootIK(FName FootBoneName, const FVector& FootLocation);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector GetFootWorldLocation(FName FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    bool PerformFootTrace(const FVector& StartLocation, FHitResult& HitResult) const;

private:
    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    UAnimInstance* AnimInstance;
};