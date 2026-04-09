#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "NaniteArchitectureManager.generated.h"

/**
 * Nanite Architecture Manager V43
 * 
 * Enforces Nanite virtualized geometry standards across the entire project.
 * This manager ensures ALL supported geometry uses Nanite for optimal performance
 * and visual fidelity in our prehistoric survival game.
 * 
 * Key responsibilities:
 * - Automatic Nanite enablement on import
 * - Runtime validation of Nanite compliance
 * - Performance monitoring of Nanite systems
 * - Integration with World Partition for streaming
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNaniteArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UNaniteArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Validates that a static mesh component uses Nanite where appropriate
     */
    UFUNCTION(BlueprintCallable, Category = "Nanite Architecture")
    bool ValidateNaniteUsage(UStaticMeshComponent* MeshComponent) const;

    /**
     * Automatically enables Nanite on a static mesh if supported
     */
    UFUNCTION(BlueprintCallable, Category = "Nanite Architecture")
    bool EnableNaniteOnMesh(UStaticMesh* StaticMesh);

    /**
     * Scans all static mesh components in the world for Nanite compliance
     */
    UFUNCTION(BlueprintCallable, Category = "Nanite Architecture")
    void ValidateWorldNaniteCompliance();

    /**
     * Gets performance metrics for Nanite rendering
     */
    UFUNCTION(BlueprintCallable, Category = "Nanite Architecture")
    FString GetNanitePerformanceReport() const;

    /**
     * Checks if a mesh should use Nanite based on our architectural rules
     */
    UFUNCTION(BlueprintCallable, Category = "Nanite Architecture")
    bool ShouldMeshUseNanite(UStaticMesh* StaticMesh) const;

protected:
    /**
     * Minimum triangle count for Nanite enablement
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nanite Settings")
    int32 MinTriangleCountForNanite = 1000;

    /**
     * Maximum triangle count before Nanite becomes mandatory
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nanite Settings")
    int32 MaxTriangleCountWithoutNanite = 50000;

    /**
     * Whether to automatically enable Nanite on import
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nanite Settings")
    bool bAutoEnableNaniteOnImport = true;

    /**
     * Whether to enforce Nanite on all supported geometry
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nanite Settings")
    bool bEnforceNaniteCompliance = true;

private:
    /**
     * Internal validation helpers
     */
    bool IsNaniteSupported() const;
    void LogNaniteViolation(const FString& MeshName, const FString& Reason) const;
    void LogNaniteCompliance(const FString& MeshName) const;

    /**
     * Performance tracking
     */
    mutable int32 TotalMeshesValidated = 0;
    mutable int32 NaniteCompliantMeshes = 0;
    mutable int32 NaniteViolations = 0;
};

/**
 * Nanite validation result structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNaniteValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsCompliant = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString MeshName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bNaniteEnabled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString RecommendedAction;

    FNaniteValidationResult()
    {
        bIsCompliant = false;
        TriangleCount = 0;
        bNaniteEnabled = false;
    }
};