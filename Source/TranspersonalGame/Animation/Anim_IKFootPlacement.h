#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector IKLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator IKRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float GroundDistance;

    FAnim_FootIKData()
    {
        SocketName = NAME_None;
        IKLocation = FVector::ZeroVector;
        IKRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = false;
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
    float MaxIKAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHipAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float HipAdjustmentAlpha;

    FAnim_IKSettings()
    {
        TraceDistance = 100.0f;
        InterpSpeed = 15.0f;
        MaxIKAdjustment = 50.0f;
        bEnableFootIK = true;
        bEnableHipAdjustment = true;
        HipAdjustmentAlpha = 0.5f;
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

    // IK Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip IK")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip IK")
    float TargetHipOffset;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Socket Names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Names")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Names")
    FName RightFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Names")
    FName HipSocketName;

public:
    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateHipIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData CalculateFootIK(const FName& SocketName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector PerformGroundTrace(const FVector& StartLocation, float TraceDistance);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator CalculateFootRotation(const FVector& ImpactPoint, const FVector& ImpactNormal);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "Hip IK")
    float GetHipOffset() const { return HipOffset; }

    UFUNCTION(BlueprintCallable, Category = "IK Settings")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK Settings")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

private:
    // Character reference
    class ACharacter* OwnerCharacter;
    class USkeletalMeshComponent* SkeletalMesh;
    
    // Internal functions
    void InitializeIKSockets();
    float CalculateHipAdjustment();
    void InterpolateIKData(FAnim_FootIKData& FootData, const FAnim_FootIKData& TargetData, float DeltaTime);
};