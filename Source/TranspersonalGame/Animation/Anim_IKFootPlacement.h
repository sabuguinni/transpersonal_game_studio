#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float DistanceFromGround = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    bool bIsGrounded = false;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bIsGrounded = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight = 13.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKDistance = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        FootHeight = 13.0f;
        MaxIKDistance = 30.0f;
        bEnableFootIK = true;
        bEnableFootRotation = true;
        
        // Default to World Static for terrain tracing
        TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class USkeletalMeshComponent* CharacterMesh;

    // Socket names for foot bones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
    FName LeftFootSocketName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
    FName RightFootSocketName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
    FName PelvisSocketName = TEXT("pelvis");

private:
    void UpdateFootIK(float DeltaTime);
    FAnim_FootIKData CalculateFootIK(const FName& FootSocketName, float DeltaTime);
    FHitResult PerformFootTrace(const FVector& FootLocation) const;
    FVector GetFootWorldLocation(const FName& FootSocketName) const;
    void UpdatePelvisOffset(float DeltaTime);

    // Cached values for smooth interpolation
    float PelvisOffset = 0.0f;
    float TargetPelvisOffset = 0.0f;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool IsIKEnabled() const { return IKSettings.bEnableFootIK; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void ResetIK();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetTraceDistance(float NewDistance);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetInterpSpeed(float NewSpeed);

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawFootTraces();
};