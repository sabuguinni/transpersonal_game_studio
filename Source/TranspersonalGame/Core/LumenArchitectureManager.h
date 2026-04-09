#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/LightComponent.h"
#include "LumenArchitectureManager.generated.h"

/**
 * Lumen Architecture Manager V43
 * 
 * Enforces Lumen Global Illumination standards across the entire project.
 * This manager ensures optimal lighting setup for our prehistoric survival game,
 * providing dynamic global illumination with infinite bounces and real-time
 * reflections for maximum visual fidelity.
 * 
 * Key responsibilities:
 * - Enforce Lumen GI as the ONLY lighting solution
 * - Validate lighting setup for optimal performance
 * - Monitor Lumen performance metrics
 * - Ensure compatibility with Nanite and VSM systems
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ULumenArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULumenArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Validates that Lumen Global Illumination is properly configured
     */
    UFUNCTION(BlueprintCallable, Category = "Lumen Architecture")
    bool ValidateLumenConfiguration() const;

    /**
     * Validates that all lights are compatible with Lumen
     */
    UFUNCTION(BlueprintCallable, Category = "Lumen Architecture")
    bool ValidateLightingSetup() const;

    /**
     * Checks if a light component is Lumen-compatible
     */
    UFUNCTION(BlueprintCallable, Category = "Lumen Architecture")
    bool ValidateLightComponent(ULightComponent* LightComponent) const;

    /**
     * Gets Lumen performance metrics and recommendations
     */
    UFUNCTION(BlueprintCallable, Category = "Lumen Architecture")
    FString GetLumenPerformanceReport() const;

    /**
     * Validates Lumen settings for our target performance (60fps PC / 30fps console)
     */
    UFUNCTION(BlueprintCallable, Category = "Lumen Architecture")
    bool ValidateLumenPerformanceSettings() const;

    /**
     * Ensures no legacy lighting systems are active
     */
    UFUNCTION(BlueprintCallable, Category = "Lumen Architecture")
    bool ValidateNoLegacyLighting() const;

protected:
    /**
     * Whether to enforce Lumen as the only GI solution
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lumen Settings")
    bool bEnforceLumenOnly = true;

    /**
     * Whether to allow static lighting (should be false for Lumen)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lumen Settings")
    bool bAllowStaticLighting = false;

    /**
     * Maximum Lumen scene view distance for performance
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lumen Settings")
    float MaxLumenSceneViewDistance = 20000.0f; // 200m default

    /**
     * Target Lumen quality level (0=Low, 1=Medium, 2=High, 3=Epic)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lumen Settings")
    int32 TargetLumenQuality = 2; // High quality for PC

private:
    /**
     * Internal validation helpers
     */
    bool IsLumenEnabled() const;
    bool IsLumenSupported() const;
    void LogLumenViolation(const FString& SystemName, const FString& Reason) const;
    void LogLumenCompliance(const FString& SystemName) const;

    /**
     * Performance tracking
     */
    mutable int32 TotalLightsValidated = 0;
    mutable int32 LumenCompatibleLights = 0;
    mutable int32 LightingViolations = 0;
};

/**
 * Lumen validation result structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLumenValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsCompliant = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bLumenEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bStaticLightingDisabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 IncompatibleLights = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Violations;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Recommendations;

    FLumenValidationResult()
    {
        bIsCompliant = false;
        bLumenEnabled = false;
        bStaticLightingDisabled = false;
        IncompatibleLights = 0;
    }
};