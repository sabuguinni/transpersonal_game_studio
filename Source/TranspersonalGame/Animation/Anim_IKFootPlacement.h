#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        FootOffset = 0.0f;
        bIsGrounded = false;
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
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableDebugDraw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    FAnim_IKSettings()
    {
        TraceDistance = 150.0f;
        InterpSpeed = 15.0f;
        FootHeight = 10.0f;
        MaxFootOffset = 50.0f;
        bEnableDebugDraw = false;
        TraceChannel = ECC_Visibility;
    }
};

/**
 * Advanced IK Foot Placement System for Prehistoric Characters
 * Handles dynamic foot positioning on uneven terrain with realistic weight distribution
 * Optimized for outdoor survival gameplay with varied ground surfaces
 */
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
    // Core IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    FAnim_FootIKData GetFootIKData(const FName& FootBoneName, const FVector& FootSocketLocation);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void EnableIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void ResetIK();

    // Terrain Analysis
    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    float CalculateFootOffset(const FVector& FootLocation, const FVector& HitLocation);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    FRotator CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation);

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    bool IsIKEnabled() const { return bIKEnabled; }

protected:
    // IK Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    float PelvisOffset;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bIKEnabled;

    // Bone Names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Setup")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Setup")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Setup")
    FName PelvisBoneName;

    // Component References
    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Internal State
    float LastUpdateTime;
    FVector LastOwnerLocation;
    bool bNeedsUpdate;

private:
    // Helper Functions
    void InitializeComponent();
    void CacheComponentReferences();
    FVector GetSocketLocation(const FName& SocketName);
    void UpdatePelvisOffset();
    void InterpolateIKData(FAnim_FootIKData& CurrentData, const FAnim_FootIKData& TargetData, float DeltaTime);
    void DrawDebugInfo();
};