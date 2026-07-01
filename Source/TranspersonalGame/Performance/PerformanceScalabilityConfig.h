// PerformanceScalabilityConfig.h — Performance Optimizer Agent #4
// Scalability profiles for 60fps PC / 30fps Console targets
// Prehistoric survival game — dinosaur AI + large open world

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceScalabilityConfig.generated.h"

UENUM(BlueprintType)
enum class EPerf_ScalabilityTier : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_Medium_60fps UMETA(DisplayName = "PC Medium 60fps"),
    PC_High_60fps   UMETA(DisplayName = "PC High 60fps"),
    PC_Epic_60fps   UMETA(DisplayName = "PC Epic 60fps"),
};

USTRUCT(BlueprintType)
struct FPerf_CVarEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FString CVarName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float Value = 1.0f;
};

USTRUCT(BlueprintType)
struct FPerf_ScalabilityProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_ScalabilityTier Tier = EPerf_ScalabilityTier::PC_High_60fps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TargetFPS = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ShadowMaxCascades = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ShadowMaxResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LumenMaxTraceDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 StreamingPoolSizeMB = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FoliageLODDistanceScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<FPerf_CVarEntry> AdditionalCVars;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_ScalabilityConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_ScalabilityConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Scalability")
    TArray<FPerf_ScalabilityProfile> Profiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Scalability")
    EPerf_ScalabilityTier ActiveTier = EPerf_ScalabilityTier::PC_High_60fps;

    UFUNCTION(BlueprintCallable, Category = "Performance|Scalability")
    void ApplyProfile(EPerf_ScalabilityTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Performance|Scalability")
    void ApplyConsoleProfile();

    UFUNCTION(BlueprintCallable, Category = "Performance|Scalability")
    void ApplyPCHighProfile();

    UFUNCTION(BlueprintCallable, Category = "Performance|Scalability")
    void ApplyPCEpicProfile();

    UFUNCTION(BlueprintPure, Category = "Performance|Scalability")
    EPerf_ScalabilityTier GetActiveTier() const { return ActiveTier; }

    UFUNCTION(BlueprintPure, Category = "Performance|Scalability")
    int32 GetTargetFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Scalability")
    void AutoDetectAndApply();

private:
    void BuildDefaultProfiles();
    void ApplyCVar(const FString& CVarName, float Value);
    FPerf_ScalabilityProfile* FindProfile(EPerf_ScalabilityTier Tier);
};
