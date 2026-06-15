#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
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
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
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
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootAdjustment;

    FAnim_IKSettings()
    {
        TraceDistance = 100.0f;
        IKInterpSpeed = 15.0f;
        FootOffset = 5.0f;
        bEnableFootRotation = true;
        MaxFootAdjustment = 50.0f;
    }
};

/**
 * Advanced IK Foot Placement Component for realistic character ground adaptation
 * Provides precise foot positioning on uneven terrain with smooth interpolation
 */
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

    // Core IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    FAnim_FootIKData CalculateFootIK(const FName& SocketName, const FVector& RootLocation);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    FRotator CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void ResetIKData();

    // Getters
    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    float GetHipOffset() const { return CurrentHipOffset; }

    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    bool IsIKEnabled() const { return bIKEnabled; }

protected:
    // IK Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIKData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    float CurrentHipOffset;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bIKEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDebugDraw;

    // Socket Names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Names")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Names")
    FName RightFootSocketName;

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

private:
    // Internal state
    float TargetHipOffset;
    float PreviousHipOffset;
    
    // Smoothing
    FVector LeftFootTargetLocation;
    FVector RightFootTargetLocation;
    FRotator LeftFootTargetRotation;
    FRotator RightFootTargetRotation;

    // Helper functions
    void InitializeComponent();
    void UpdateHipOffset(float DeltaTime);
    float CalculateHipAdjustment(const FAnim_FootIKData& LeftFoot, const FAnim_FootIKData& RightFoot);
    void DrawDebugInfo();
};