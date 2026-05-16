#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Anim_IKFootPlacement.generated.h"

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
    float DistanceFromGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bIsGrounded = true;
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
    float MaxIKAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHipAdjustment;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        FootOffset = 5.0f;
        MaxIKAdjustment = 30.0f;
        bEnableFootRotation = true;
        bEnableHipAdjustment = true;
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

    // IK calculation functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetHipOffset() const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool IsIKEnabled() const;

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    FVector GetGroundNormal(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    float GetGroundHeight(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    bool IsOnValidGround(const FVector& Location) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hip Adjustment")
    float HipOffset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hip Adjustment")
    float TargetHipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName HipBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Control")
    bool bIKEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugTraces;

private:
    // Cached references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    USkeletalMeshComponent* MeshComponent;

    // Internal state
    FAnim_FootIKData PreviousLeftFootIK;
    FAnim_FootIKData PreviousRightFootIK;
    float PreviousHipOffset;

    // Helper functions
    void CacheComponentReferences();
    FAnim_FootIKData CalculateFootIK(const FName& BoneName, const FAnim_FootIKData& PreviousData, float DeltaTime);
    FVector GetBoneWorldLocation(const FName& BoneName) const;
    FRotator CalculateFootRotationFromNormal(const FVector& GroundNormal) const;
    bool PerformGroundTrace(const FVector& StartLocation, FVector& HitLocation, FVector& HitNormal) const;
    void UpdateHipAdjustment(float DeltaTime);
    void DrawDebugInfo() const;
};