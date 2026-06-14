#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName IKBoneName;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector WorldLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector IKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator IKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootHeight;

    FAnim_FootData()
    {
        BoneName = NAME_None;
        IKBoneName = NAME_None;
        WorldLocation = FVector::ZeroVector;
        IKOffset = FVector::ZeroVector;
        IKRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        TraceDistance = 50.0f;
        FootHeight = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MinIKDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDrawDebugTraces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    FAnim_IKSettings()
    {
        InterpSpeed = 15.0f;
        MaxIKDistance = 30.0f;
        MinIKDistance = -20.0f;
        bEnableIK = true;
        bDrawDebugTraces = false;
        TraceChannel = ECC_WorldStatic;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Foot data for each foot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    TArray<FAnim_FootData> FootData;

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Pelvis adjustment
    UPROPERTY(BlueprintReadOnly, Category = "Pelvis IK")
    float PelvisOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis IK")
    FName PelvisBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis IK")
    float PelvisInterpSpeed;

    // Character references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // IK calculation functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdatePelvisIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector PerformFootTrace(const FAnim_FootData& FootInfo, bool& bHitGround);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator CalculateFootRotation(const FVector& ImpactPoint, const FVector& ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float CalculateIKAlpha(float Distance);

    // Setup functions
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void InitializeFootData();

    UFUNCTION(CallInEditor, Category = "Setup")
    void SetupDefaultFeet();

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInfo();

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootData GetFootData(int32 FootIndex) const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FVector GetLeftFootIKOffset() const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FVector GetRightFootIKOffset() const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FRotator GetLeftFootIKRotation() const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FRotator GetRightFootIKRotation() const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetLeftFootIKAlpha() const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetRightFootIKAlpha() const;

private:
    // Internal state
    float TargetPelvisOffset;
    TArray<FVector> PreviousFootLocations;
    
    // Helper functions
    FVector GetBoneWorldLocation(const FName& BoneName);
    void ClampIKValues(FAnim_FootData& FootInfo);
};

#include "Anim_IKFootPlacement.generated.h"