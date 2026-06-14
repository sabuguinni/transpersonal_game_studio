#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct FAnim_FootData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    bool bIsGrounded = false;

    FAnim_FootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = false;
    }
};

USTRUCT(BlueprintType)
struct FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKDistance = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK = true;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        FootOffset = 5.0f;
        MaxIKDistance = 30.0f;
        bEnableIK = true;
    }
};

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootData GetLeftFootData() const { return LeftFootData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootData GetRightFootData() const { return RightFootData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetHipOffset() const { return HipOffset; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Foot Data")
    FAnim_FootData LeftFootData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot Data")
    FAnim_FootData RightFootData;

    UPROPERTY(BlueprintReadOnly, Category = "Hip Adjustment")
    float HipOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
    class ACharacter* OwnerCharacter;
    class USkeletalMeshComponent* CharacterMesh;

    UFUNCTION()
    FVector PerformFootTrace(const FVector& FootLocation, bool& bHitGround);

    UFUNCTION()
    void UpdateFootData(FAnim_FootData& FootData, const FName& BoneName, float DeltaTime);

    UFUNCTION()
    void CalculateHipOffset();

    float TargetHipOffset = 0.0f;
    float CurrentHipOffset = 0.0f;
};