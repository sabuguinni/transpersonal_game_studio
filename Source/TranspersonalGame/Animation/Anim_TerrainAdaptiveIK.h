#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimationAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Anim_TerrainAdaptiveIK.generated.h"

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

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TerrainIKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "200.0"))
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugDraw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TArray<FName> FootBoneNames;

    FAnim_TerrainIKSettings()
    {
        TraceDistance = 100.0f;
        FootOffset = 5.0f;
        InterpSpeed = 2.0f;
        bEnableDebugDraw = false;
        FootBoneNames.Add(TEXT("foot_l"));
        FootBoneNames.Add(TEXT("foot_r"));
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TerrainAdaptiveIK : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TerrainAdaptiveIK();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configurações principais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    FAnim_TerrainIKSettings IKSettings;

    // Dados dos pés
    UPROPERTY(BlueprintReadOnly, Category = "Terrain IK")
    TMap<FName, FAnim_FootIKData> FootIKData;

    // Componente de mesh esquelética
    UPROPERTY(BlueprintReadOnly, Category = "Terrain IK")
    USkeletalMeshComponent* OwnerMeshComponent;

    // Métodos principais
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    FAnim_FootIKData CalculateFootIK(const FName& BoneName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    bool PerformGroundTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    bool IsIKEnabled() const { return bIKEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void AddFootBone(const FName& BoneName);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void RemoveFootBone(const FName& BoneName);

    // Getters para Blueprint
    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    FAnim_FootIKData GetFootIKData(const FName& BoneName) const;

    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    float GetFootIKAlpha(const FName& BoneName) const;

    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    FVector GetFootIKLocation(const FName& BoneName) const;

    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    FRotator GetFootIKRotation(const FName& BoneName) const;

protected:
    // Estado interno
    UPROPERTY()
    bool bIKEnabled;

    UPROPERTY()
    float LastUpdateTime;

    // Cache para performance
    UPROPERTY()
    TMap<FName, FVector> PreviousFootLocations;

    UPROPERTY()
    TMap<FName, FRotator> PreviousFootRotations;

    // Métodos auxiliares
    void InitializeComponent();
    void CacheOwnerMeshComponent();
    void InitializeFootData();
    void DrawDebugInfo(const FName& BoneName, const FAnim_FootIKData& FootData, const FVector& TraceStart, const FVector& TraceEnd);
};