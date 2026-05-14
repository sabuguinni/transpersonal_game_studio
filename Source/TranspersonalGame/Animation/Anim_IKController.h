#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Anim_IKController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float DistanceFromGround;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    bool bValidTrace;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bValidTrace = false;
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
    float MaxIKAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName PelvisSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnablePelvisAdjustment;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        MaxIKAdjustment = 30.0f;
        FootRadius = 5.0f;
        LeftFootSocketName = TEXT("foot_l");
        RightFootSocketName = TEXT("foot_r");
        PelvisSocketName = TEXT("pelvis");
        bEnableFootIK = true;
        bEnablePelvisAdjustment = true;
    }
};

/**
 * IK Controller for realistic foot placement on uneven terrain
 * Handles foot IK, pelvis adjustment, and terrain adaptation
 * Essential for prehistoric survival game where characters walk on natural terrain
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_IKController : public UObject
{
    GENERATED_BODY()

public:
    UAnim_IKController();

    // Main update function called from AnimInstance
    UFUNCTION(BlueprintCallable, Category = "IK Controller")
    void UpdateIK(USkeletalMeshComponent* MeshComponent, float DeltaTime);

    // Initialize IK system
    UFUNCTION(BlueprintCallable, Category = "IK Controller")
    void InitializeIK(USkeletalMeshComponent* MeshComponent);

    // Get foot IK data for animation blueprint
    UFUNCTION(BlueprintPure, Category = "IK Controller")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintPure, Category = "IK Controller")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintPure, Category = "IK Controller")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintPure, Category = "IK Controller")
    bool IsIKEnabled() const { return bIKEnabled; }

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bIKEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDebugDraw;

protected:
    // IK Data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float PelvisOffset;

    // Internal state
    UPROPERTY()
    USkeletalMeshComponent* CachedMeshComponent;

    UPROPERTY()
    UWorld* CachedWorld;

private:
    // Core IK functions
    FAnim_FootIKData CalculateFootIK(const FName& SocketName, float DeltaTime);
    float CalculatePelvisOffset(float DeltaTime);
    
    // Trace functions
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    FVector GetSocketWorldLocation(const FName& SocketName) const;
    FRotator CalculateFootRotationFromNormal(const FVector& HitNormal) const;
    
    // Helper functions
    float GetIKAlphaFromDistance(float Distance) const;
    void DrawDebugIK() const;
    bool IsValidForIK() const;
    
    // Interpolation targets
    float TargetPelvisOffset;
    FVector TargetLeftFootLocation;
    FVector TargetRightFootLocation;
    FRotator TargetLeftFootRotation;
    FRotator TargetRightFootRotation;
    
    // State tracking
    bool bInitialized;
    float LastUpdateTime;
};