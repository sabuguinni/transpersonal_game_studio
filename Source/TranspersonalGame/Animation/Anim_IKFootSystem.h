#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Anim_IKFootSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float GroundDistance;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = true;
        GroundDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootAdjustment;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        FootOffset = 5.0f;
        bEnableFootRotation = true;
        MaxFootAdjustment = 30.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName PelvisBoneName;

    UPROPERTY(BlueprintReadOnly, Category = "Hip Adjustment")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector GetFootWorldLocation(const FName& FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool PerformGroundTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator CalculateFootRotationFromNormal(const FVector& GroundNormal) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateHipOffset(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void ResetIKData();

private:
    class USkeletalMeshComponent* GetOwnerSkeletalMesh() const;
    void InitializeBoneNames();
    
    float TargetHipOffset;
    bool bIKInitialized;
};