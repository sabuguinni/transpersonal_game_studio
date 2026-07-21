#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Perf_TextureStreaming.generated.h"

UENUM(BlueprintType)
enum class EPerf_TextureQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TextureSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
    EPerf_TextureQuality QualityLevel = EPerf_TextureQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
    float StreamingDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
    int32 MaxTextureSize = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
    bool bEnableTextureStreaming = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
    float MemoryBudgetMB = 512.0f;

    FPerf_TextureSettings()
    {
        QualityLevel = EPerf_TextureQuality::Medium;
        StreamingDistanceMultiplier = 1.0f;
        MaxTextureSize = 2048;
        bEnableTextureStreaming = true;
        MemoryBudgetMB = 512.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TextureStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 LoadedTextures = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 StreamedTextures = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float TextureMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float StreamingPoolUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 PendingStreamingRequests = 0;

    FPerf_TextureStats()
    {
        LoadedTextures = 0;
        StreamedTextures = 0;
        TextureMemoryUsageMB = 0.0f;
        StreamingPoolUsageMB = 0.0f;
        PendingStreamingRequests = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_TextureStreaming : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_TextureStreaming();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World Subsystem interface
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTextureStreaming();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTextureQuality(EPerf_TextureQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateTextureSettings(const FPerf_TextureSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_TextureStats GetTextureStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceTextureStreaming(bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ClearUnusedTextures();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMemoryBudget(float BudgetMB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsTextureStreamingEnabled() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_TextureSettings TextureSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    FPerf_TextureStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoOptimizeTextures = true;

private:
    FTimerHandle StreamingUpdateTimer;

    void UpdateTextureStats();
    void ProcessTextureStreaming();
    void OptimizeMaterialTextures();
    void ApplyQualitySettings();
    int32 GetTextureSizeForQuality(EPerf_TextureQuality Quality, int32 OriginalSize) const;
    float GetStreamingDistanceForQuality(EPerf_TextureQuality Quality) const;
};