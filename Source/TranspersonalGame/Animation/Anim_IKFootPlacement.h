#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootOffset;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        FootOffset = 0.0f;
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

    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootIKBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
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
    FAnim_FootIKData CalculateFootIK(FName FootBoneName, FName IKBoneName);

    UFUNCTION(BlueprintCallable, Category = "IK")
    float CalculateHipOffset();

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

private:
    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;

    FVector PerformFootTrace(const FVector& FootLocation);
    FRotator CalculateFootRotation(const FVector& ImpactPoint, const FVector& ImpactNormal);
};