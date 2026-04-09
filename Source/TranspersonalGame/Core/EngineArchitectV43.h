#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "EngineArchitectV43.generated.h"

/**
 * Engine Architect V43 - Core Architecture Foundation
 * 
 * This class defines the fundamental architecture rules and systems
 * that ALL other systems must follow. It enforces:
 * - UE5.5 compatibility standards
 * - Nanite virtualized geometry requirements
 * - Lumen global illumination integration
 * - Virtual Shadow Maps compliance
 * - World Partition mandatory usage
 * - Performance targets (60fps PC / 30fps console)
 * 
 * NO SYSTEM can violate these architectural laws.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectV43 : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectV43();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * ARCHITECTURAL LAW #1: World Partition is MANDATORY for worlds > 4km²
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateWorldPartitionCompliance(UWorld* World) const;

    /**
     * ARCHITECTURAL LAW #2: Nanite MUST be enabled on all supported geometry
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateNaniteCompliance(UStaticMeshComponent* MeshComponent) const;

    /**
     * ARCHITECTURAL LAW #3: Lumen Global Illumination is the ONLY lighting solution
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateLumenCompliance(UWorld* World) const;

    /**
     * ARCHITECTURAL LAW #4: Virtual Shadow Maps replace ALL legacy shadow methods
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateVSMCompliance(UWorld* World) const;

    /**
     * ARCHITECTURAL LAW #5: Performance targets are NON-NEGOTIABLE
     * PC: 60fps minimum | Console: 30fps minimum
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceCompliance() const;

    /**
     * ARCHITECTURAL LAW #6: All assets MUST use One File Per Actor in World Partition
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateOFPACompliance(UWorld* World) const;

    /**
     * Master validation - checks ALL architectural laws
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateFullArchitecturalCompliance(UWorld* World) const;

    /**
     * Enforcement system - automatically fixes violations where possible
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforceArchitecturalCompliance(UWorld* World);

protected:
    /**
     * Core architectural standards that define the engine foundation
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Architecture Standards")
    bool bEnforceWorldPartition = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Architecture Standards")
    bool bEnforceNaniteUsage = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Architecture Standards")
    bool bEnforceLumenGI = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Architecture Standards")
    bool bEnforceVirtualShadowMaps = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Architecture Standards")
    float MinFrameRatePC = 60.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Architecture Standards")
    float MinFrameRateConsole = 30.0f;

    /**
     * Maximum world size before World Partition becomes mandatory (in cm²)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Architecture Standards")
    float MaxWorldSizeWithoutPartition = 40000000.0f; // 4km² in cm²

private:
    /**
     * Internal validation helpers
     */
    bool CheckWorldPartitionEnabled(UWorld* World) const;
    bool CheckNaniteSupport() const;
    bool CheckLumenEnabled(UWorld* World) const;
    bool CheckVSMEnabled(UWorld* World) const;
    void LogArchitecturalViolation(const FString& ViolationType, const FString& Details) const;
    void LogArchitecturalCompliance(const FString& SystemName) const;

    /**
     * Violation tracking for reporting
     */
    mutable TArray<FString> ArchitecturalViolations;
    mutable int32 ComplianceScore = 100;
};

/**
 * Engine Architecture Validation Result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEngineArchitectureValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsCompliant = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ComplianceScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Violations;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Recommendations;

    FEngineArchitectureValidation()
    {
        bIsCompliant = false;
        ComplianceScore = 0;
    }
};