// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "AdvancedFootIKSystemV44.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName FootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName IKBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName KneeBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float KneeTargetAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector KneeTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bIsPlanted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float PlantingThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float LiftingThreshold;

    FFootIKData()
    {
        FootOffset = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        KneeTargetAlpha = 0.0f;
        KneeTargetLocation = FVector::ZeroVector;
        bIsPlanted = false;
        PlantingThreshold = 50.0f;
        LiftingThreshold = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTerrainSurfaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    FVector SurfaceNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    float SurfaceAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    FVector SurfaceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    UPhysicalMaterial* SurfaceMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    float SurfaceFriction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
    bool bIsValidSurface;

    FTerrainSurfaceData()
    {
        SurfaceNormal = FVector::UpVector;
        SurfaceAngle = 0.0f;
        SurfaceLocation = FVector::ZeroVector;
        SurfaceMaterial = nullptr;
        SurfaceFriction = 1.0f;
        bIsValidSurface = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFootPlantedDelegate, FName, FootBoneName, bool, bIsPlanted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSurfaceChangedDelegate, FName, FootBoneName, UPhysicalMaterial*, OldMaterial, UPhysicalMaterial*, NewMaterial);

/**
 * Advanced Foot IK System for realistic character movement on uneven terrain
 * Features include foot planting, surface adaptation, and procedural knee adjustment
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAdvancedFootIKSystemV44 : public UActorComponent
{
    GENERATED_BODY()

public:
    UAdvancedFootIKSystemV44();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TArray<FFootIKData> FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float RotationInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootPlanting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableKneeTargeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableSurfaceAdaptation;

    // Hip adjustment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip Adjustment")
    FName HipBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip Adjustment")
    float HipAdjustmentRatio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip Adjustment")
    float MaxHipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip Adjustment")
    float HipInterpSpeed;

    // Terrain adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxSlopeAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SurfaceAdaptationStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseAsyncTracing;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDebugVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowTraceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowFootTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowSurfaceNormals;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnFootPlantedDelegate OnFootPlanted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSurfaceChangedDelegate OnSurfaceChanged;

    // Blueprint Interface
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector GetFootIKOffset(FName FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator GetFootIKRotation(FName FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetFootIKAlpha(FName FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector GetHipOffset() const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool IsFootPlanted(FName FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FTerrainSurfaceData GetSurfaceDataForFoot(FName FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootPlantingEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void AddFootIKTarget(FName FootBoneName, FName IKBoneName, FName KneeBoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void RemoveFootIKTarget(FName FootBoneName);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootPlantingThresholds(FName FootBoneName, float PlantingThreshold, float LiftingThreshold);

    // Advanced features
    UFUNCTION(BlueprintCallable, Category = "Advanced IK")
    void EnableSpeedBasedPlanting(bool bEnabled, float MinSpeedForPlanting = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Advanced IK")
    void SetSurfaceAdaptationStrength(float Strength);

    UFUNCTION(BlueprintCallable, Category = "Advanced IK")
    void UpdateKneeTargeting(float DeltaTime);

protected:
    // Internal functions
    void InitializeFootIKData();
    void UpdateSingleFootIK(FFootIKData& FootData, float DeltaTime);
    void UpdateHipAdjustment(float DeltaTime);
    void UpdateFootPlanting(FFootIKData& FootData, const FTerrainSurfaceData& SurfaceData, float DeltaTime);
    
    FTerrainSurfaceData PerformFootTrace(const FFootIKData& FootData);
    FVector CalculateKneeTarget(const FFootIKData& FootData, const FVector& FootTargetLocation);
    FRotator CalculateFootRotationFromSurface(const FVector& SurfaceNormal, const FVector& ForwardVector);
    
    bool ShouldPlantFoot(const FFootIKData& FootData, const FTerrainSurfaceData& SurfaceData) const;
    bool ShouldLiftFoot(const FFootIKData& FootData, const FTerrainSurfaceData& SurfaceData) const;
    
    void DrawDebugVisualization() const;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Runtime data
    UPROPERTY(Transient)
    FVector HipOffset;

    UPROPERTY(Transient)
    TMap<FName, FTerrainSurfaceData> CachedSurfaceData;

    UPROPERTY(Transient)
    float LastUpdateTime;

    UPROPERTY(Transient)
    bool bIKEnabled;

    UPROPERTY(Transient)
    bool bSpeedBasedPlantingEnabled;

    UPROPERTY(Transient)
    float MinSpeedForPlanting;

    // Performance optimization
    UPROPERTY(Transient)
    int32 CurrentFootIndex;

    UPROPERTY(Transient)
    float TimeSinceLastUpdate;
};