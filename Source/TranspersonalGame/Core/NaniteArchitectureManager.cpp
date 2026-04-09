#include "NaniteArchitectureManager.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogNaniteArchitecture, Log, All);

UNaniteArchitectureManager::UNaniteArchitectureManager()
{
    // Initialize performance tracking
    TotalMeshesValidated = 0;
    NaniteCompliantMeshes = 0;
    NaniteViolations = 0;
}

void UNaniteArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("=== NANITE ARCHITECTURE MANAGER V43 INITIALIZED ==="));
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("Enforcing Nanite virtualized geometry standards..."));
    
    // Check if Nanite is supported on current platform
    if (!IsNaniteSupported())
    {
        UE_LOG(LogNaniteArchitecture, Error, TEXT("❌ Nanite not supported on current platform"));
        return;
    }
    
    UE_LOG(LogNaniteArchitecture, Log, TEXT("✅ Nanite support confirmed"));
    
    // Perform initial world validation
    ValidateWorldNaniteCompliance();
}

void UNaniteArchitectureManager::Deinitialize()
{
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("Nanite Architecture Manager V43 shutting down"));
    
    // Log final performance report
    FString PerformanceReport = GetNanitePerformanceReport();
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("Final Performance Report:\n%s"), *PerformanceReport);
    
    Super::Deinitialize();
}

bool UNaniteArchitectureManager::ValidateNaniteUsage(UStaticMeshComponent* MeshComponent) const
{
    if (!MeshComponent)
    {
        LogNaniteViolation(TEXT("Unknown"), TEXT("MeshComponent is null"));
        return false;
    }

    UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
    if (!StaticMesh)
    {
        LogNaniteViolation(TEXT("Unknown"), TEXT("StaticMesh is null"));
        return false;
    }

    FString MeshName = StaticMesh->GetName();
    
    // Check if this mesh should use Nanite
    if (!ShouldMeshUseNanite(StaticMesh))
    {
        LogNaniteCompliance(MeshName + TEXT(" (Not required for Nanite)"));
        return true;
    }

    // Check if Nanite is enabled
    bool bNaniteEnabled = StaticMesh->IsNaniteEnabled();
    
    if (!bNaniteEnabled && bEnforceNaniteCompliance)
    {
        LogNaniteViolation(MeshName, TEXT("Nanite not enabled on high-poly mesh"));
        return false;
    }

    LogNaniteCompliance(MeshName);
    return true;
}

bool UNaniteArchitectureManager::EnableNaniteOnMesh(UStaticMesh* StaticMesh)
{
    if (!StaticMesh)
    {
        UE_LOG(LogNaniteArchitecture, Error, TEXT("Cannot enable Nanite: StaticMesh is null"));
        return false;
    }

    if (!IsNaniteSupported())
    {
        UE_LOG(LogNaniteArchitecture, Error, TEXT("Cannot enable Nanite: Not supported on current platform"));
        return false;
    }

    FString MeshName = StaticMesh->GetName();

    // Check if mesh should use Nanite
    if (!ShouldMeshUseNanite(StaticMesh))
    {
        UE_LOG(LogNaniteArchitecture, Warning, TEXT("Mesh '%s' doesn't meet criteria for Nanite"), *MeshName);
        return false;
    }

    // Check if already enabled
    if (StaticMesh->IsNaniteEnabled())
    {
        UE_LOG(LogNaniteArchitecture, Log, TEXT("Nanite already enabled on mesh '%s'"), *MeshName);
        return true;
    }

    // Enable Nanite (this would require editor-only functionality in practice)
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("Nanite enablement on mesh '%s' requires editor rebuild"), *MeshName);
    
    return false; // Cannot enable at runtime, requires asset rebuild
}

void UNaniteArchitectureManager::ValidateWorldNaniteCompliance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogNaniteArchitecture, Error, TEXT("Cannot validate world: World is null"));
        return;
    }

    UE_LOG(LogNaniteArchitecture, Warning, TEXT("=== VALIDATING WORLD NANITE COMPLIANCE ==="));

    // Reset counters
    TotalMeshesValidated = 0;
    NaniteCompliantMeshes = 0;
    NaniteViolations = 0;

    // Iterate through all static mesh actors in the world
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* StaticMeshActor = *ActorItr;
        if (!StaticMeshActor)
        {
            continue;
        }

        UStaticMeshComponent* MeshComponent = StaticMeshActor->GetStaticMeshComponent();
        if (!MeshComponent)
        {
            continue;
        }

        TotalMeshesValidated++;

        if (ValidateNaniteUsage(MeshComponent))
        {
            NaniteCompliantMeshes++;
        }
        else
        {
            NaniteViolations++;
        }
    }

    // Log validation results
    float CompliancePercentage = TotalMeshesValidated > 0 ? 
        (float)NaniteCompliantMeshes / (float)TotalMeshesValidated * 100.0f : 100.0f;

    UE_LOG(LogNaniteArchitecture, Warning, TEXT("=== NANITE VALIDATION COMPLETE ==="));
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("Total meshes validated: %d"), TotalMeshesValidated);
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("Compliant meshes: %d"), NaniteCompliantMeshes);
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("Violations: %d"), NaniteViolations);
    UE_LOG(LogNaniteArchitecture, Warning, TEXT("Compliance rate: %.1f%%"), CompliancePercentage);

    if (NaniteViolations > 0)
    {
        UE_LOG(LogNaniteArchitecture, Error, TEXT("❌ NANITE VIOLATIONS DETECTED - Fix immediately"));
    }
    else
    {
        UE_LOG(LogNaniteArchitecture, Warning, TEXT("✅ FULL NANITE COMPLIANCE ACHIEVED"));
    }
}

FString UNaniteArchitectureManager::GetNanitePerformanceReport() const
{
    FString Report;
    
    Report += TEXT("=== NANITE PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Total Meshes Validated: %d\n"), TotalMeshesValidated);
    Report += FString::Printf(TEXT("Nanite Compliant: %d\n"), NaniteCompliantMeshes);
    Report += FString::Printf(TEXT("Violations: %d\n"), NaniteViolations);
    
    if (TotalMeshesValidated > 0)
    {
        float ComplianceRate = (float)NaniteCompliantMeshes / (float)TotalMeshesValidated * 100.0f;
        Report += FString::Printf(TEXT("Compliance Rate: %.1f%%\n"), ComplianceRate);
    }
    
    // Add platform-specific information
    Report += TEXT("\nPlatform Support:\n");
    Report += FString::Printf(TEXT("Nanite Supported: %s\n"), IsNaniteSupported() ? TEXT("Yes") : TEXT("No"));
    
    // Add performance recommendations
    Report += TEXT("\nRecommendations:\n");
    if (NaniteViolations > 0)
    {
        Report += TEXT("- Enable Nanite on all high-poly meshes (>1000 triangles)\n");
        Report += TEXT("- Rebuild assets with Nanite support\n");
        Report += TEXT("- Consider mesh optimization for low-poly assets\n");
    }
    else
    {
        Report += TEXT("- Nanite compliance achieved\n");
        Report += TEXT("- Continue monitoring new asset imports\n");
    }
    
    return Report;
}

bool UNaniteArchitectureManager::ShouldMeshUseNanite(UStaticMesh* StaticMesh) const
{
    if (!StaticMesh)
    {
        return false;
    }

    // Get triangle count from the mesh
    int32 TriangleCount = 0;
    if (StaticMesh->GetRenderData() && StaticMesh->GetRenderData()->LODResources.Num() > 0)
    {
        const FStaticMeshLODResources& LODResource = StaticMesh->GetRenderData()->LODResources[0];
        TriangleCount = LODResource.GetNumTriangles();
    }

    // Apply our architectural rules
    if (TriangleCount >= MaxTriangleCountWithoutNanite)
    {
        // High-poly meshes MUST use Nanite
        return true;
    }
    
    if (TriangleCount >= MinTriangleCountForNanite)
    {
        // Medium-poly meshes SHOULD use Nanite
        return true;
    }

    // Low-poly meshes don't need Nanite
    return false;
}

bool UNaniteArchitectureManager::IsNaniteSupported() const
{
    // Check if Nanite is supported on current platform
    static IConsoleVariable* NaniteEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite"));
    return NaniteEnabledCVar && NaniteEnabledCVar->GetInt() != 0;
}

void UNaniteArchitectureManager::LogNaniteViolation(const FString& MeshName, const FString& Reason) const
{
    UE_LOG(LogNaniteArchitecture, Error, TEXT("🚨 NANITE VIOLATION [%s]: %s"), *MeshName, *Reason);
}

void UNaniteArchitectureManager::LogNaniteCompliance(const FString& MeshName) const
{
    UE_LOG(LogNaniteArchitecture, Verbose, TEXT("✅ Nanite Compliant: %s"), *MeshName);
}