#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector SurfaceNormal;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bIsGrounded = false;
        SurfaceNormal = FVector::UpVector;
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
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootRotationInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        IKInterpSpeed = 15.0f;
        FootHeight = 10.0f;
        MaxFootAdjustment = 30.0f;
        bEnableFootRotation = true;
        FootRotationInterpSpeed = 10.0f;
        TraceChannel = ECC_WorldStatic;
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
    // Main IK functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData CalculateFootIK(FName FootBoneName, FVector FootSocketLocation);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    bool IsIKEnabled() const { return bIKEnabled; }

    // Foot data accessors
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetHipOffset() const { return CurrentHipOffset; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_IKSettings GetIKSettings() const { return IKSettings; }

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool PerformFootTrace(FVector StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator CalculateFootRotationFromNormal(FVector SurfaceNormal);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float CalculateHipOffset();

protected:
    // IK data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float CurrentHipOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float TargetHipOffset;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAnim_IKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FName RightFootSocketName;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIKEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsInitialized;

private:
    // Internal functions
    void InitializeIKSystem();
    FVector GetFootSocketLocation(FName SocketName);
    USkeletalMeshComponent* GetOwnerSkeletalMesh();
    void InterpolateIKData(FAnim_FootIKData& CurrentData, const FAnim_FootIKData& TargetData, float DeltaTime);
    bool IsCharacterMoving();
    float GetCharacterSpeed();
};