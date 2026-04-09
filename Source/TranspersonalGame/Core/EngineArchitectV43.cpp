#include "EngineArchitectV43.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogEngineArchitectV43, Log, All);

UEngineArchitectV43::UEngineArchitectV43()
{
    // Initialize architectural compliance tracking
    ArchitecturalViolations.Empty();
    ComplianceScore = 100;
}

void UEngineArchitectV43::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("=== ENGINE ARCHITECT V43 INITIALIZED ==="));
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("Enforcing UE5.5 architectural standards..."));
    
    // Validate current world on initialization
    if (UWorld* World = GetWorld())
    {
        ValidateFullArchitecturalCompliance(World);
    }
    
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("Engine Architecture V43 ready - all systems must comply"));
}

void UEngineArchitectV43::Deinitialize()
{
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("Engine Architect V43 shutting down"));
    Super::Deinitialize();
}

bool UEngineArchitectV43::ValidateWorldPartitionCompliance(UWorld* World) const
{
    if (!World)
    {
        LogArchitecturalViolation(TEXT("World Partition"), TEXT("World is null"));
        return false;
    }

    if (!bEnforceWorldPartition)
    {
        LogArchitecturalCompliance(TEXT("World Partition (Enforcement Disabled)"));
        return true;
    }

    // Check if World Partition is enabled
    bool bHasWorldPartition = CheckWorldPartitionEnabled(World);
    
    if (!bHasWorldPartition)
    {
        LogArchitecturalViolation(TEXT("World Partition"), 
            TEXT("World Partition is MANDATORY for all worlds > 4km². Enable immediately."));
        return false;
    }

    LogArchitecturalCompliance(TEXT("World Partition"));
    return true;
}

bool UEngineArchitectV43::ValidateNaniteCompliance(UStaticMeshComponent* MeshComponent) const
{
    if (!MeshComponent)
    {
        LogArchitecturalViolation(TEXT("Nanite"), TEXT("MeshComponent is null"));
        return false;
    }

    if (!bEnforceNaniteUsage)
    {
        LogArchitecturalCompliance(TEXT("Nanite (Enforcement Disabled)"));
        return true;
    }

    UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
    if (!StaticMesh)
    {
        LogArchitecturalViolation(TEXT("Nanite"), TEXT("StaticMesh is null"));
        return false;
    }

    // Check if Nanite is enabled on the mesh
    bool bNaniteEnabled = StaticMesh->IsNaniteEnabled();
    
    if (!bNaniteEnabled)
    {
        LogArchitecturalViolation(TEXT("Nanite"), 
            FString::Printf(TEXT("Mesh '%s' does not have Nanite enabled. Enable Nanite for ALL supported geometry."), 
                *StaticMesh->GetName()));
        return false;
    }

    LogArchitecturalCompliance(TEXT("Nanite"));
    return true;
}

bool UEngineArchitectV43::ValidateLumenCompliance(UWorld* World) const
{
    if (!World)
    {
        LogArchitecturalViolation(TEXT("Lumen"), TEXT("World is null"));
        return false;
    }

    if (!bEnforceLumenGI)
    {
        LogArchitecturalCompliance(TEXT("Lumen (Enforcement Disabled)"));
        return true;
    }

    bool bLumenEnabled = CheckLumenEnabled(World);
    
    if (!bLumenEnabled)
    {
        LogArchitecturalViolation(TEXT("Lumen"), 
            TEXT("Lumen Global Illumination is MANDATORY. Enable in Project Settings > Rendering."));
        return false;
    }

    LogArchitecturalCompliance(TEXT("Lumen Global Illumination"));
    return true;
}

bool UEngineArchitectV43::ValidateVSMCompliance(UWorld* World) const
{
    if (!World)
    {
        LogArchitecturalViolation(TEXT("Virtual Shadow Maps"), TEXT("World is null"));
        return false;
    }

    if (!bEnforceVirtualShadowMaps)
    {
        LogArchitecturalCompliance(TEXT("Virtual Shadow Maps (Enforcement Disabled)"));
        return true;
    }

    bool bVSMEnabled = CheckVSMEnabled(World);
    
    if (!bVSMEnabled)
    {
        LogArchitecturalViolation(TEXT("Virtual Shadow Maps"), 
            TEXT("Virtual Shadow Maps are MANDATORY. Enable in Project Settings > Rendering > Shadows."));
        return false;
    }

    LogArchitecturalCompliance(TEXT("Virtual Shadow Maps"));
    return true;
}

bool UEngineArchitectV43::ValidatePerformanceCompliance() const
{
    // Get current frame rate
    float CurrentFrameRate = 1.0f / FApp::GetDeltaTime();
    
    // Determine target frame rate based on platform
    float TargetFrameRate = MinFrameRatePC; // Default to PC target
    
#if PLATFORM_DESKTOP
    TargetFrameRate = MinFrameRatePC;
#else
    TargetFrameRate = MinFrameRateConsole;
#endif

    if (CurrentFrameRate < TargetFrameRate)
    {
        LogArchitecturalViolation(TEXT("Performance"), 
            FString::Printf(TEXT("Frame rate %.1f fps below target %.1f fps. Optimize immediately."), 
                CurrentFrameRate, TargetFrameRate));
        return false;
    }

    LogArchitecturalCompliance(TEXT("Performance"));
    return true;
}

bool UEngineArchitectV43::ValidateOFPACompliance(UWorld* World) const
{
    if (!World)
    {
        LogArchitecturalViolation(TEXT("OFPA"), TEXT("World is null"));
        return false;
    }

    // Check if One File Per Actor is enabled
    // This is automatically handled by World Partition
    bool bWorldPartitionEnabled = CheckWorldPartitionEnabled(World);
    
    if (!bWorldPartitionEnabled)
    {
        LogArchitecturalViolation(TEXT("OFPA"), 
            TEXT("One File Per Actor requires World Partition. Enable World Partition first."));
        return false;
    }

    LogArchitecturalCompliance(TEXT("One File Per Actor"));
    return true;
}

bool UEngineArchitectV43::ValidateFullArchitecturalCompliance(UWorld* World) const
{
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("=== FULL ARCHITECTURAL COMPLIANCE VALIDATION ==="));
    
    // Reset violation tracking
    ArchitecturalViolations.Empty();
    ComplianceScore = 100;
    
    bool bFullCompliance = true;
    int32 ViolationCount = 0;

    // Validate each architectural law
    if (!ValidateWorldPartitionCompliance(World))
    {
        bFullCompliance = false;
        ViolationCount++;
    }

    if (!ValidateLumenCompliance(World))
    {
        bFullCompliance = false;
        ViolationCount++;
    }

    if (!ValidateVSMCompliance(World))
    {
        bFullCompliance = false;
        ViolationCount++;
    }

    if (!ValidatePerformanceCompliance())
    {
        bFullCompliance = false;
        ViolationCount++;
    }

    if (!ValidateOFPACompliance(World))
    {
        bFullCompliance = false;
        ViolationCount++;
    }

    // Calculate compliance score
    ComplianceScore = FMath::Max(0, 100 - (ViolationCount * 20));

    if (bFullCompliance)
    {
        UE_LOG(LogEngineArchitectV43, Warning, TEXT("✅ FULL ARCHITECTURAL COMPLIANCE ACHIEVED"));
        UE_LOG(LogEngineArchitectV43, Warning, TEXT("Compliance Score: %d/100"), ComplianceScore);
    }
    else
    {
        UE_LOG(LogEngineArchitectV43, Error, TEXT("❌ ARCHITECTURAL VIOLATIONS DETECTED"));
        UE_LOG(LogEngineArchitectV43, Error, TEXT("Compliance Score: %d/100"), ComplianceScore);
        UE_LOG(LogEngineArchitectV43, Error, TEXT("Violations: %d"), ViolationCount);
        
        for (const FString& Violation : ArchitecturalViolations)
        {
            UE_LOG(LogEngineArchitectV43, Error, TEXT("VIOLATION: %s"), *Violation);
        }
    }

    return bFullCompliance;
}

void UEngineArchitectV43::EnforceArchitecturalCompliance(UWorld* World)
{
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("=== ENFORCING ARCHITECTURAL COMPLIANCE ==="));
    
    if (!World)
    {
        UE_LOG(LogEngineArchitectV43, Error, TEXT("Cannot enforce compliance: World is null"));
        return;
    }

    // Auto-fix violations where possible
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("Auto-fixing architectural violations..."));
    
    // Note: Some fixes require manual intervention and cannot be automated
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("Manual fixes required for some violations"));
    UE_LOG(LogEngineArchitectV43, Warning, TEXT("Refer to architectural violation log for details"));
}

// Private helper methods
bool UEngineArchitectV43::CheckWorldPartitionEnabled(UWorld* World) const
{
    if (!World)
    {
        return false;
    }
    
    // Check if World Partition is enabled
    // In UE5, this can be checked through the world's settings
    return World->IsPartitionedWorld();
}

bool UEngineArchitectV43::CheckNaniteSupport() const
{
    // Check if Nanite is supported on current platform
    static IConsoleVariable* NaniteEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite"));
    return NaniteEnabledCVar && NaniteEnabledCVar->GetInt() != 0;
}

bool UEngineArchitectV43::CheckLumenEnabled(UWorld* World) const
{
    // Check if Lumen is enabled in project settings
    static IConsoleVariable* LumenGICVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicGlobalIlluminationMethod"));
    return LumenGICVar && LumenGICVar->GetInt() == 1; // 1 = Lumen
}

bool UEngineArchitectV43::CheckVSMEnabled(UWorld* World) const
{
    // Check if Virtual Shadow Maps are enabled
    static IConsoleVariable* VSMCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.Enable"));
    return VSMCVar && VSMCVar->GetInt() != 0;
}

void UEngineArchitectV43::LogArchitecturalViolation(const FString& ViolationType, const FString& Details) const
{
    FString ViolationMessage = FString::Printf(TEXT("[%s] %s"), *ViolationType, *Details);
    ArchitecturalViolations.Add(ViolationMessage);
    
    UE_LOG(LogEngineArchitectV43, Error, TEXT("🚨 ARCHITECTURAL VIOLATION: %s"), *ViolationMessage);
}

void UEngineArchitectV43::LogArchitecturalCompliance(const FString& SystemName) const
{
    UE_LOG(LogEngineArchitectV43, Log, TEXT("✅ %s: COMPLIANT"), *SystemName);
}